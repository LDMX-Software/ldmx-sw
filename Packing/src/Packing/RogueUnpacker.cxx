
#include "Packing/RogueUnpacker.h"

namespace packing {

struct StreamWriterChannelHeader {
  uint16_t flags;
  uint8_t error;
  uint8_t channel;
  uint32_t size;

  Reader& read(Reader& r) {
    return (r >> size >> flags >> error >> channel);
  }
};

struct LDMXRoRHeader {
  uint8_t vers;
  uint8_t subsys;
  uint8_t contrib;
  uint8_t sentinel;
  uint32_t zero;
  uint64_t timestamp;
  
  Reader& read(Reader& r) {
    return (r >> vers >> subsys >> contrib >> sentinel >> zero >> timestamp);
  }
};

struct SubsysRawData {
  uint8_t subsys;
  std::vector<uint8_t> data;
};

void RogueUnpacker::beforeNewRun(ldmx::RunHeader& rh) {
  rh.setDetectorName(detector_name_);
}

void RogueUnpacker::configure(framework::config::Parameters& ps) {
  reader_.open(ps.get<std::string>("raw_file"));
  detector_name_ = ps.get<std::string>("detector_name");
  subsystems_ = ps.get<std::vector<int>>("subsystems");
  contribs_ = ps.get<std::vector<int>>("contribs");
  subsystem_name_ = ps.get<std::vector<int>>("subsystem_name");
}

void RogueUnpacker::produce(framework::Event& event) {
  if (!reader_ or reader_.eof()) abortEvent();

  std::set subsys_found;
  have_one_of_each = false;
  StreamWriterChannelHeader stream_writer_header;
  LDMXRoRHeader ldmx_header;
  while(reader_ and not have_one_of_each) {
    if (!reader_.read(stream_writer_header)) {
      EXCEPTION_RAISE("MalForm", "Unable to read Rogue StreamWriter Header from input data file.");
    }
    if (stream_writer_header.channel != 0) {
      // skip this packet
      reader_.seek(stream_writer_header.size);
      continue;
    }
    if (!reader_.read(ldmx_header)) {
      EXCEPTION_RAISE("MalForm", "Unable to read LDMX RoR Header from input data file.");
    }
    auto subsys_index{subsystems_.find(ldmx_header.subsys) - subsystems_.begin()};
    if (ldmx_header.subsys not in subsys_found and subsys_index < subsystems_.size()) {
      // check if contrib is correct
      if (ldmx_header.contrib != contribs_.at(subsys_index)) {
        // skip packet
        reader_.seek(stream_writer_header.size - 16);
        continue;
      }

      // read into buffer and put into event
      std::vector<uint8_t> buff;
      if (!reader_.read(buff, stream_writer_header.size - 16)) {
        EXCEPTION_RAISE("MalForm", "Raw file provided was unable to read raw data from subsystem");
      }
      event.add(subsystem_name_.at(subsys_index), buff);
      subsys_found.insert(ldmx_header.subsys);
    }

    have_one_of_each = 
  }
}

}  // namespace packing

DECLARE_PRODUCER(packing::RogueUnpacker)
