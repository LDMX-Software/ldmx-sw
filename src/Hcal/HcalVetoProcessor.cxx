/**
 * @file HcalVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Hcal/HcalVetoProcessor.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "DetDescr/HcalID.h"

namespace hcal {

HcalVetoProcessor::HcalVetoProcessor(const std::string &name, framework::Process &process)
    : Producer(name, process) {}

HcalVetoProcessor::~HcalVetoProcessor() {}

void HcalVetoProcessor::configure(framework::config::Parameters &parameters) {
  totalPEThreshold_ = parameters.getParameter<double>("pe_threshold");
  maxTime_ = parameters.getParameter<double>("max_time");
  maxDepth_ = parameters.getParameter<double>("max_depth");
  minPE_ = parameters.getParameter<double>("back_min_pe");
}

void HcalVetoProcessor::produce(framework::Event &event) {
  // Get the collection of sim particles from the event
  const std::vector<hcal::event::HcalHit> hcalRecHits =
      event.getCollection<hcal::event::HcalHit>("HcalRecHits");

  // Loop over all of the Hcal hits and calculate to total photoelectrons
  // in the event.
  float totalPe{0};
  float maxPE{-1000};
  const hcal::event::HcalHit *maxPEHit;
  for (const hcal::event::HcalHit &hcalHit : hcalRecHits) {
    // If the hit time is outside the readout window, don't consider it.
    if (hcalHit.getTime() >= maxTime_) continue;

    // If the hit z position is beyond the maximum HCal depth, skip it.
    if (hcalHit.getZPos() > maxDepth_) continue;

    // Get the total PE in the bar
    float pe = hcalHit.getPE();

    // Keep track of the total PE
    totalPe += pe;

    // Check that both sides of the bar have a PE value above threshold.
    // If not, don't consider the hit.  Double sided readout is only
    // being used for the back HCal bars.  For the side HCal, just
    // use the maximum PE as before.
    ldmx::HcalID id(hcalHit.getID());
    if ((id.section() == ldmx::HcalID::BACK) && (hcalHit.getMinPE() < minPE_))
      continue;

    // Find the maximum PE in the list
    if (maxPE < pe) {
      maxPE = pe;
      maxPEHit = &hcalHit;
    }
  }

  // If the maximum PE found is below threshold, it passes the veto.
  bool passesVeto = (maxPE < totalPEThreshold_);

  hcal::event::HcalVetoResult result;
  result.setVetoResult(passesVeto);
  result.setMaxPEHit(*maxPEHit);

  if (passesVeto) {
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }

  event.add("HcalVeto", result);
}
}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalVetoProcessor);
