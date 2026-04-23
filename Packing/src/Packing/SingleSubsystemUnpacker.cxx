
#include "Packing/SingleSubsystemUnpacker.h"

#include "Packing/LDMXRoRHeader.h"
#include "Packing/RogueFrameHeader.h"
#include <iostream>

namespace packing {

void SingleSubsystemUnpacker::configure(framework::config::Parameters& ps) {
  reader_.open(ps.get<std::string>("dat_file"));
  auto subsystem_name{ps.get<std::string>("subsystem_name")};
  if (subsystem_name.empty()) {
    subsystem_ = ps.get<int>("subsystem");
    contributor_ = ps.get<int>("contributor");
  } else {
    auto [subsys, contrib] = packing::LDMXRoRHeader::subsystem(subsystem_name);
    if (subsystem_ == -1) {
      EXCEPTION_RAISE("BadName",
                      "Subsystem name '" + subsystem_name +
                          "' not 'ts', 'tdaq', 'tracker', 'ecal', 'hcal'.");
    }
    subsystem_ = subsys;
    contributor_ = contrib;
  }
  frame_offset_ = ps.get<int>("frame_offset");
  output_name_ = ps.get<std::string>("output_name");
  frame_count_ = 0;
}

void SingleSubsystemUnpacker::produce(framework::Event& event) {
  static packing::RogueFrameHeader frame_header;
  static packing::LDMXRoRHeader ror_header;

  static long n_total = 0;
  static long n_yaml = 0;
  static long n_invalid = 0;
  static long n_wrong_subsystem = 0;
  static long n_wrong_contributor = 0;
  static long n_accepted = 0;
  static bool summary_printed = false;

  while (reader_ and not reader_.eof()) { 
    reader_ >> frame_header;
    n_total++;

    // store location of end-of-frame for skipping this frame
    // if we fail any of the filter checks
    const auto frame_end =
        reader_.tell() + static_cast<std::streamoff>(frame_header.size());

    if (frame_header.probablyYaml()) {
      n_yaml++;
      if (n_yaml <= 10) {
        std::cout << "[yaml] frame " << n_total
                  << " size=" << frame_header.size()
                  << " channel=" << frame_header.channel()
                  << std::endl;
      }
      reader_.seek(frame_end);
      continue;
    }

    reader_ >> ror_header;

    if (!ror_header.valid()) {
      n_invalid++;
      if (n_invalid <= 10) {
        std::cout << "[invalid] frame " << n_total
                  << " size=" << frame_header.size()
                  << " channel=" << frame_header.channel()
                  << std::endl;
      }
      reader_.seek(frame_end);
      continue;
    }

    if (ror_header.subsystem() != subsystem_) {
      n_wrong_subsystem++;
      if (n_wrong_subsystem <= 10) {
        std::cout << "[wrong subsystem] frame " << n_total
                  << " got subsystem=" << ror_header.subsystem()
                  << " expected=" << subsystem_
                  << " contributor=" << ror_header.contributor()
                  << std::endl;
      }
      reader_.seek(frame_end);
      continue;
    }

    if (contributor_ >= 0 and contributor_ != ror_header.contributor()) {
      n_wrong_contributor++;
      if (n_wrong_contributor <= 10) {
        std::cout << "[wrong contributor] frame " << n_total
                  << " got contributor=" << ror_header.contributor()
                  << " expected=" << contributor_
                  << " subsystem=" << ror_header.subsystem()
                  << std::endl;
      }
      reader_.seek(frame_end);
      continue;
    }

    frame_count_++;
    if (frame_offset_ >= frame_count_) {
      reader_.seek(frame_end);
      continue;
    }

    std::vector<uint8_t> buff;
    if (not reader_.read(buff,
                         frame_header.size() - packing::LDMXRoRHeader::SIZE)) {
      EXCEPTION_RAISE(
          "MalForm", "Raw file provided was unable to read entire data frame.");
    }

    n_accepted++;

    event.add(output_name_, buff);
    event.getEventHeader().setIntParameter("RoR Timestamp",
                                           ror_header.timestamp());
    return;
  }

  if (!summary_printed) {
    summary_printed = true;
    std::cout << "\n=== SingleSubsystemUnpacker summary ===\n"
              << "total frames seen      : " << n_total << "\n"
              << "yaml frames skipped    : " << n_yaml << "\n"
              << "invalid RoR skipped    : " << n_invalid << "\n"
              << "wrong subsystem skipped: " << n_wrong_subsystem << "\n"
              << "wrong contrib skipped  : " << n_wrong_contributor << "\n"
              << "accepted frames        : " << n_accepted << "\n"
              << "=======================================\n"
              << std::endl;
  }

  abortEvent();
}

}  // namespace packing

DECLARE_PRODUCER(packing::SingleSubsystemUnpacker)
