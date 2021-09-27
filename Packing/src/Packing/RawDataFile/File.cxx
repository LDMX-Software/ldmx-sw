
#include "Packing/RawDataFile/File.h"

#include "Packing/Utility/Mask.h"
#include "Packing/Utility/CRC.h"

#include "DetDescr/DetectorID.h"

namespace packing {
namespace rawdatafile {

File::File(const framework::config::Parameters &ps) {
  is_output_ = ps.getParameter<bool>("is_output");
  std::string fn = ps.getParameter<std::string>("filename");

  ecal_object_name_ = ps.getParameter<std::string>("ecal_object_name");
  hcal_object_name_ = ps.getParameter<std::string>("hcal_object_name");
  tracker_object_name_ = ps.getParameter<std::string>("tracker_object_name");
  triggerpad_object_name_ = ps.getParameter<std::string>("triggerpad_object_name");
  pass_name_ = ps.getParameter<std::string>("pass_name");

  if (is_output_) {
    writer_.open(fn);
    // leave entry count undefined
    // use passed run number
    uint32_t header = ((0 & utility::mask<4>) << 28) + run_ & utility::mask<28>;
    writer_ << header;
    crc_ << header;
    entries_ = 0;
    i_entry_ = 0;
  } else {
    reader_.open(fn);
    // get entry count from file
    // get run id number from file
    uint32_t word;
    reader_ >> word;

    uint8_t version = (word >> 28) & utility::mask<4>;
    if (version != 0) {
      EXCEPTION_RAISE("RawFileVers",
          "Unable to handle raw file version "
          + std::to_string(version));
    }

    run_ = word & utility::mask<8>;

    reader_.seek<uint32_t>(-2,std::ios::end);
    auto eof{reader_.tell<uint32_t>()}; // save EOF in number of 32-bit-width words
    uint32_t crc_read_in;
    reader_ >> entries_ >> crc_read_in;

    reader_.seek<uint32_t>(1,std::ios::beg);

    if (ps.getParameter<bool>("verify_checksum")) {
      utility::CRC crc;
      for (auto ifile{reader_.tell<uint32_t>()}; ifile < eof; ifile++) {
        reader_ >> word;
        crc << word;
      }

      if (crc.get() != crc_read_in) {
        EXCEPTION_RAISE("CRCNotOk",
            "Failure to verify CRC checksum of entire input file.");
      }

      reader_.seek<uint32_t>(1, std::ios::beg);
    } // verify checksum of input file
  } //input or output file
}

bool File::connect(framework::Event& event) {
  event_ = &event;
  return true;
}

bool File::nextEvent() {
  if(is_output_) {
    // dump buffers into event packet and write out
    static std::map<std::string, uint16_t> name_to_eid = {
      { tracker_object_name_ , ldmx::SubdetectorIDType::EID_TRACKER },
      { triggerpad_object_name_ , ldmx::SubdetectorIDType::EID_TRIGGER_SCINT },
      { ecal_object_name_ , ldmx::SubdetectorIDType::EID_ECAL },
      { triggerpad_object_name_ , ldmx::SubdetectorIDType::EID_HCAL },
    };
    
    std::map<uint16_t, std::vector<uint32_t>> the_subsys_data;
    for (auto const& [name, id] : name_to_eid) {
      the_subsys_data[id] = event_->getCollection<uint32_t>(name, pass_name_);
    }

    EventPacket write_event(event_->getEventNumber(), the_subsys_data);
    writer_ << write_event;
    crc_ << write_event;
    if (!writer_)
      return false;

    entries_++;
    i_entry_++;
  } else {
    // read buffers from event packet and add to event bus
    static EventPacket read_event;
    reader_ >> read_event;
    if (!reader_) {
      // ERROR or EOF
      return false;
    }
    
    //event->setEventNumber(read_event.id());
    for (auto &subsys : read_event.data()) {
      std::string name;
      switch(subsys.id()) {
        case ldmx::SubdetectorIDType::EID_TRACKER :
          name = tracker_object_name_;
          break;
        case ldmx::SubdetectorIDType::EID_TRIGGER_SCINT :
          name = triggerpad_object_name_;
          break;
        case ldmx::SubdetectorIDType::EID_ECAL :
          name = ecal_object_name_;
          break;
        case ldmx::SubdetectorIDType::EID_HCAL :
          name = hcal_object_name_;
          break;
        default :
          std::cerr << subsys.id() << " unrecognized electronics ID." << std::endl;
          name = "EID"+std::to_string(subsys.id());
      }  // switch for id

      event_->add(name, subsys.data());
    }  // loop over subsystems
  }    // input or output
  return true;
}

void File::writeRunHeader(ldmx::RunHeader &header) {
  //header.setRunNumber(run_);
}

void File::close() {
  event_ = nullptr;
  if (is_output_) {
    writer_ << (entries_ & utility::mask<32>);
    crc_ << (entries_ & utility::mask<32>);
    writer_ << crc_.get();
  }
}

}
}