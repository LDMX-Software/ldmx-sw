

#include "Hcal/HcalRawDecoder.h"
// un comment for HcalRawDecoder-specific debug printouts to std::cout
//#define DEBUG

namespace hcal {

namespace utility {

/**
 * Read out 32-bit words from a 8-bit buffer.
 */
class Reader {
  const std::vector<uint8_t>& buffer_;
  std::size_t i_word_;
  uint32_t next() {
    uint32_t w = buffer_.at(i_word_) | (buffer_.at(i_word_ + 1) << 8) |
                 (buffer_.at(i_word_ + 2) << 16) |
                 (buffer_.at(i_word_ + 3) << 24);
    i_word_ += 4;
    return w;
  }

 public:
  Reader(const std::vector<uint8_t>& b) : buffer_{b}, i_word_{0} {}
  operator bool() { return (i_word_ < buffer_.size()); }
  Reader& operator>>(uint32_t& w) {
    if (*this) w = next();
    return *this;
  }
};  // Reader

}  // namespace utility

namespace debug {

struct hex {
  uint32_t word_;
  hex(uint32_t w) : word_{w} {}
};

}  // namespace debug

inline std::ostream& operator<<(std::ostream& os, const debug::hex& h) {
  os << "0x" << std::setfill('0') << std::setw(8) << std::hex << h.word_
     << std::dec;
  return os;
}

/**
 * Struct to help interface between raw decoder read function
 * and putting stuff onto event bus
 */
void PolarfireEventHeader::board(framework::Event& event,
                                 const std::string& prefix) {
  {
    event.add(prefix + "Version", version);
    event.add(prefix + "FPGA", fpga);
    event.add(prefix + "NSamples", nsamples);
    event.add(prefix + "Spill", spill);
    event.add(prefix + "Ticks", ticks);
    event.add(prefix + "Bunch", bunch);
    event.add(prefix + "Number", number);
    event.add(prefix + "Run", run);
    event.add(prefix + "DD", DD);
    event.add(prefix + "MM", MM);
    event.add(prefix + "hh", hh);
    event.add(prefix + "mm", mm);
    event.add(prefix + "GoodLinkHeader", good_bxheader);
    event.add(prefix + "GoodLinkTrailer", good_trailer);
  }
}

void HcalRawDecoder::configure(framework::config::Parameters& ps) {
  input_file_ = ps.getParameter<std::string>("input_file");
  input_names_ = ps.getParameter<std::vector<std::string>>("input_names", {});
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  roc_version_ = ps.getParameter<int>("roc_version");
  translate_eid_ = ps.getParameter<bool>("translate_eid");
  read_from_file_ = ps.getParameter<bool>("read_from_file");
  detector_name_ = ps.getParameter<std::string>("detector_name");
  if (read_from_file_) {
    file_reader_.open(input_file_);
  }
}

void HcalRawDecoder::beforeNewRun(ldmx::RunHeader& rh) {
  // if we are reading from a file, we need to provide the detector name
  if (read_from_file_) {
    rh.setDetectorName(detector_name_);
  }
}

void HcalRawDecoder::produce(framework::Event& event) {
  std::map<ldmx::HcalElectronicsID,
           std::vector<ldmx::HgcrocDigiCollection::Sample>>
      eid_to_samples;
  PolarfireEventHeader eh;
  if (read_from_file_) {
    if (!file_reader_ or file_reader_.eof()) return;
    eid_to_samples = this->read(file_reader_, eh);
  } else {
    for (const auto& name : input_names_) {
      hcal::utility::Reader bus_reader(
          event.getCollection<uint8_t>(name, input_pass_));
      auto single_pf_samples = this->read(bus_reader, eh);
      for (const auto& [id, samples] : single_pf_samples) {
        eid_to_samples[id] = samples;
      }
    }
  }

  eh.board(event, output_name_);

  ldmx::HgcrocDigiCollection digis;
  // assume all channels have same number of samples
  digis.setNumSamplesPerDigi(eid_to_samples.begin()->second.size());
  digis.setSampleOfInterestIndex(0);  // TODO configurable
  digis.setVersion(roc_version_);
  if (translate_eid_) {
    /**
     * Translation
     *
     * Now the HgcrocDigiCollection::Sample class handles the
     * unpacking of individual samples; however, we still need
     * to translate electronic IDs into detector IDs.
     */
#ifdef DEBUG
    std::cout << "Translating EIDs into DetIDs. Printing skipped EIDs..."
              << std::endl;
#endif
    auto detmap{
        getCondition<HcalDetectorMap>(HcalDetectorMap::CONDITIONS_OBJECT_NAME)};
    for (auto const& [eid, digi] : eid_to_samples) {
      // The electronics map returns an empty ID of the correct
      // type when the electronics ID is not found.
      //  need to check if the electronics ID exists
      //  TODO: do we want to end processing if this happens?
      if (detmap.exists(eid)) {
        uint32_t did_raw = detmap.get(eid).raw();
        digis.addDigi(did_raw, digi);
      } else {
        /** DO NOTHING
         *  skip hits where the EID aren't in the detector mapping
         *  no zero supp during test beam on the front-end,
         *  so channels that aren't connected to anything are still
         *  being readout.
         */
#ifdef DEBUG
        std::cout << "EID(" << eid.fiber() << "," << eid.elink() << ","
                  << eid.channel() << ") ";
        for (auto& s : digi) std::cout << debug::hex(s.raw()) << " ";
        std::cout << std::endl;
#endif
      }
    }
  } else {
    /**
     * no EID translation, just add the digis to the digi collection
     * with their raw electronic ID
     * TODO: remove this, we shouldn't be able to get past
     *       the decoding stage without translating the EID
     *       into a detector ID to avoid confusion in recon
     */
    for (auto const& [eid, digi] : eid_to_samples) {
      digis.addDigi(eid.raw(), digi);
    }
  }

#ifdef DEBUG
  std::cout << "adding " << digis.getNumDigis() << " digis each with "
            << digis.getNumSamplesPerDigi() << " samples to event bus"
            << std::endl;
#endif
  event.add(output_name_, digis);
  return;
}  // produce

}  // namespace hcal
DECLARE_PRODUCER_NS(hcal, HcalRawDecoder);
