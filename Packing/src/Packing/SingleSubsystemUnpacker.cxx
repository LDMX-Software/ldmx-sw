
#include "Packing/SingleSubsystemUnpacker.h"

#include "Packing/LDMXRoRHeader.h"
#include "Packing/RogueFrameHeader.h"

namespace packing {

void SingleSubsystemUnpacker::configure(framework::config::Parameters& ps) {
  auto dat_file{ps.get<std::string>("dat_file")};
  reader_.open(dat_file);
  if (!reader_) {
    EXCEPTION_RAISE("FileNotFound",
                    "SingleSubsystemUnpacker could not open '" + dat_file +
                        "'. Check the path and that it is mounted inside the "
                        "container (denv_mounts in .denv/config).");
  }
  auto subsystem_name{ps.get<std::string>("subsystem_name")};
  if (subsystem_name.empty()) {
    subsystem_ = ps.get<int>("subsystem");
    contributor_ = ps.get<int>("contributor");
  } else {
    auto [subsys, contrib] = packing::LDMXRoRHeader::subsystem(subsystem_name);
    if (subsys == -1) {
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

  while (reader_ and not reader_.eof()) {
    reader_ >> frame_header;

    // store location of end-of-frame for skipping this frame
    // if we fail any of the filter checks
    const auto frame_end = reader_.tell() + frame_header.size();

    if (frame_header.probablyYaml()) {
      // configuration/YAML frame written by StreamWriter, skip
      reader_.seek(frame_end);
      continue;
    }

    // data channel, read RoR header
    reader_ >> ror_header;
    if (!ror_header.valid() or ror_header.subsystem() != subsystem_) {
      // not a valid LDMX data frame or wrong subsystem ID number
      reader_.seek(frame_end);
      continue;
    }

    if (contributor_ >= 0 and contributor_ != ror_header.contributor()) {
      // wrong contributor ID number
      reader_.seek(frame_end);
      continue;
    }

    // correct subsystem and contributor channel
    frame_count_++;
    if (frame_offset_ >= frame_count_) {
      // skip the first frame_offset_ frames that correspond to the selected
      // subsystem
      reader_.seek(frame_end);
      continue;
    }

    // load data into memory, add to event, and leave
    std::vector<uint8_t> buff;
    if (not reader_.read(buff,
                         frame_header.size() - packing::LDMXRoRHeader::SIZE)) {
      EXCEPTION_RAISE(
          "MalForm", "Raw file provided was unable to read entire data frame.");
    }

    // buff has subsystem data without RoR header
    event.add(output_name_, buff);
    // Store the full 64-bit RoR timestamp as two 32-bit halves since
    // EventHeader::setIntParameter only accepts int.
    uint64_t ts = ror_header.timestamp();
    event.getEventHeader().setIntParameter("RoR Timestamp LSB",
                                           static_cast<int>(ts & 0xFFFFFFFFU));
    event.getEventHeader().setIntParameter("RoR Timestamp MSB",
                                           static_cast<int>(ts >> 32));
    // successfully unpacked an event, return from produce
    return;
  }

  /// abort event if we've reached the end of the file (left while loop)
  abortEvent();
}

}  // namespace packing

DECLARE_PRODUCER(packing::SingleSubsystemUnpacker)
