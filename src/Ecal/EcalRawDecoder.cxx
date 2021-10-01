/**
 * @file EcalRawDecoder.cxx
 * @brief Class that performs basic ECal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/EcalRawDecoder.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Tools/HgcrocDecoder.h"
#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/EcalID.h"
#include "Ecal/EcalDetectorMap.h"

namespace ecal {

EcalRawDecoder::EcalRawDecoder(const std::string& name,
                                   framework::Process& process)
    : Producer(name, process) {
}

EcalRawDecoder::~EcalRawDecoder() {}

void EcalRawDecoder::configure(framework::config::Parameters& ps) {
  input_name_ = ps.getParameter<std::string>("input_name");
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  roc_version_ = ps.getParameter<int>("roc_version");
}

void EcalRawDecoder::produce(framework::Event& event) {
  /**
   * Decoding of raw data
   *
   * First we unpack the encoded data into a map
   * of electronic IDs to sets of timesamples.
   * This is done by another utility class
   */
  tools::HgcrocDecoder decoder{roc_version_};
  auto eid_to_samples{
    decoder.decode(event.getCollection<uint32_t>(input_name_, input_pass_))};

  /**
   * Translation
   *
   * Now the HgcrocDigiCollection::Sample class handles the 
   * unpacking of individual samples; however, we still need
   * to translate electronic IDs into detector IDs.
   */
  auto detmap{getCondition<EcalDetectorMap>(EcalDetectorMap::CONDITIONS_OBJECT_NAME)};
  ldmx::HgcrocDigiCollection digis;
  for (auto const& [eid_raw, digi] : eid_to_samples) {
    // TODO: This checking of existence should be temporary,
    //       the electronic ID mapping should be complete.
    uint32_t did_raw{eid_raw};
    ldmx::EcalElectronicsID eid{eid_raw};
    if (detmap.exists(eid)) {
      did_raw = detmap.get(eid).raw();
    }

    digis.addDigi(did_raw, digi);
  }

  event.add(output_name_, digis);

  return;
}  // produce

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalRawDecoder);
