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

HcalVetoProcessor::HcalVetoProcessor(const std::string &name,
                                     framework::Process &process)
    : Producer(name, process) {}

HcalVetoProcessor::~HcalVetoProcessor() {}

void HcalVetoProcessor::configure(framework::config::Parameters &parameters) {
  totalPEThreshold_ = parameters.getParameter<double>("pe_threshold");
  maxTime_ = parameters.getParameter<double>("max_time");
  outputCollName_ = parameters.getParameter<std::string>("output_coll_name");
  inputHitCollName_ =
      parameters.getParameter<std::string>("input_hit_coll_name");
  inputHitPassName_ =
      parameters.getParameter<std::string>("input_hit_pass_name");
  // A fake-hit that gets added for the rare case where no hit actually reaches
  // the maxPE < pe check to avoid producing uninitialized memory
  //
  // Default constructed hits have nonsense-but predictable values and are
  // harder to mistake for real hits
  defaultMaxHit_.Clear();
  defaultMaxHit_.setPE(-9999);
  defaultMaxHit_.setMinPE(-9999);
  defaultMaxHit_.setSection(-9999);
  defaultMaxHit_.setLayer(-9999);
  defaultMaxHit_.setStrip(-9999);
  defaultMaxHit_.setEnd(-999);
  defaultMaxHit_.setTimeDiff(-9999);
  defaultMaxHit_.setToaPos(-9999);
  defaultMaxHit_.setToaNeg(-9999);
  defaultMaxHit_.setAmplitudePos(-9999);
  defaultMaxHit_.setAmplitudeNeg(-9999);

  double maxDepth_ = parameters.getParameter<double>("max_depth", 0.);
  if (maxDepth_ != 0.) {
    EXCEPTION_RAISE(
        "InvalidParam",
        "Earlier versions of the Hcal veto defined a max depth for "
        "positions which is no longer implemented. Remove the "
        "parameter (max_depth) from your configuration. See "
        "https://github.com/LDMX-Software/Hcal/issues/61 for details");
  }
  backMinPE_ = parameters.getParameter<double>("back_min_pe");
}

void HcalVetoProcessor::produce(framework::Event &event) {
  // Get the collection of sim particles from the event
  const std::vector<ldmx::HcalHit> hcalRecHits =
      event.getCollection<ldmx::HcalHit>(inputHitCollName_, inputHitPassName_);

  // Loop over all of the Hcal hits and calculate to total photoelectrons
  // in the event.
  float totalPe{0};
  float maxPE{-1000};

  const ldmx::HcalHit *maxPEHit{defaultMaxHit_};
  for (const ldmx::HcalHit &hcalHit : hcalRecHits) {
    // If the hit time is outside the readout window, don't consider it.
    if (hcalHit.getTime() >= maxTime_) {
      continue;
    }

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

  ldmx::HcalVetoResult result;
  result.setVetoResult(passesVeto);
  result.setMaxPEHit(*maxPEHit);

  if (passesVeto) {
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }

  event.add(outputCollName_, result);
}
}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalVetoProcessor);
