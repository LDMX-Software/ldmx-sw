
#include "Packing/SingleSubsystemUnpacker.h"

#include "Packing/RogueFrameHeader.h"
#include "Packing/LDMXRoRHeader.h"

namespace packing {

void SingleSubsystemUnpacker::configure(framework::config::Parameters& ps) {
  reader_.open(ps.get<std::string>("dat_file"));
  subsystem_ = ps.get<int>("subsystem");
  contributor_ = ps.get<int>("contributor");
  frame_offset_ = ps.get<int>("frame_offset");
  output_name_ = ps.get<std::string>("output_name");
  frame_count_ = 0;
}

void SingleSubsystemUnpacker::produce(framework::Event& event) {
  static packing::RogueFrameHeader frame_header;
  static packing::LDMXRoRHeader ror_header;

  while(reader_ and not reader_.eof()) {
    reader_ >> frame_header;
    if (frame_header.channel() != 0) {
      // non-data channel in StreamWriter, skip
      reader_.seek(reader_.tell()+frame_header.size());
      continue;
    }

    // data channel, read RoR header
    reader_ >> ror_header;
    if (ror_header.subsystem() != subsystem_) {
      // wrong subsystem ID number
      reader_.seek(reader_.tell()+frame_header.size()-ror_header.size);
      continue;
    }

    if (contributor_ >= 0 and contributor_ != ror_header.contributor()) {
      // wrong contributor ID number
      reader_.seek(reader_.tell()+frame_header.size()-ror_header.size);
      continue;
    }
    
    // correct subsystem and contributor channel
    frame_count_++;
    if (frame_offset_ >= frame_count_) {
      // skip the first frame_offset_ frames that correspond to the selected subsystem
      reader_.seek(reader_.tell()+frame_header.size()-ror_header.size);
      continue;
    }

    // load data into memory, add to event, and leave
    std::vector<uint8_t> buff;
    if (not reader_.read(buff, frame_header.size() - ror_header.size)) {
      EXCEPTION_RAISE("MalForm",
                      "Raw file provided was unable to read entire data frame.");
    }

    // buff has subsystem data without RoR header
    event.add(output_name_, buff);
    // ror_header has global RoR information
    event.getEventHeader().setIntParameter("RoR Timestamp", ror_header.timestamp());
    // successfully unpacked an event, return from produce
    return;
  }

  /// abort event if we've reached the end of the file (left while loop)
  abortEvent();
}

}  // namespace packing

DECLARE_PRODUCER(packing::SingleSubsystemUnpacker)
