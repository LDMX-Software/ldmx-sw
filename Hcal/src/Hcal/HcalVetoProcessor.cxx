/**
 * @file HcalVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tamas Almos Vami, UCSB
 */

#include "DetDescr/HcalID.h"
#include "Hcal/HcalVetoProcessor.h"

namespace hcal {

HcalVetoProcessor::HcalVetoProcessor(const std::string &name,
                                     framework::Process &process)
    : Producer(name, process) {}

void HcalVetoProcessor::configure(framework::config::Parameters &parameters) {
  total_PE_threshold_ = parameters.getParameter<double>("pe_threshold");
  max_time_ = parameters.getParameter<double>("max_time");
  output_coll_name_ = parameters.getParameter<std::string>("output_coll_name");
  input_hit_coll_name_ =
      parameters.getParameter<std::string>("input_hit_coll_name");
  input_hit_pass_name_ =
      parameters.getParameter<std::string>("input_hit_pass_name");
  // A fake-hit that gets added for the rare case where no hit actually reaches
  // the maxPE < pe check to avoid producing uninitialized memory
  //
  // Default constructed hits have nonsense-but predictable values and are
  // harder to mistake for real hits
  default_max_hit_.Clear();
  default_max_hit_.setPE(-9999);
  default_max_hit_.setMinPE(-9999);
  default_max_hit_.setSection(-9999);
  default_max_hit_.setLayer(-9999);
  default_max_hit_.setStrip(-9999);
  default_max_hit_.setEnd(-999);
  default_max_hit_.setTimeDiff(-9999);
  default_max_hit_.setToaPos(-9999);
  default_max_hit_.setToaNeg(-9999);
  default_max_hit_.setAmplitudePos(-9999);
  default_max_hit_.setAmplitudeNeg(-9999);

  double max_depth_ = parameters.getParameter<double>("max_depth", 0.);
  if (max_depth_ != 0.) {
    EXCEPTION_RAISE(
        "InvalidParam",
        "Earlier versions of the Hcal veto defined a max depth for "
        "positions which is no longer implemented. Remove the "
        "parameter (max_depth) from your configuration. See "
        "https://github.com/LDMX-Software/Hcal/issues/61 for details");
  }
  back_min_PE_ = parameters.getParameter<double>("back_min_pe");
}

void HcalVetoProcessor::produce(framework::Event &event) {
  // Get the collection of sim particles from the event
  const std::vector<ldmx::HcalHit> hcal_rec_hits =
      event.getCollection<ldmx::HcalHit>(input_hit_coll_name_, input_hit_pass_name_);

  // Loop over all of the Hcal hits and calculate to total photoelectrons
  // in the event.
  float total_PE{0.0};
  float max_PE{-1000};
  int num_total_hits{0};
  int num_valid_hits{0};

  const ldmx::HcalHit *max_PE_hit{&default_max_hit_};
  for (const ldmx::HcalHit &hcal_hit : hcal_rec_hits) {
    num_total_hits++ ;
    // If the hit time is outside the readout window, don't consider it.
    if (hcal_hit.getTime() >= max_time_) {
      continue;
    }

    // Get the total PE in the bar
    float pe = hcal_hit.getPE();

    // Get the position of this hit
    auto postitionX = hcal_hit.getXPos();
    auto postitionY = hcal_hit.getYPos();
    auto postitionZ = hcal_hit.getZPos();
    ldmx_log(info) << " postition[0] " << postitionX  << " postition[1] " << postitionY  << " postition[2] " << postitionZ;

    // Keep track of the total PE
    total_PE += pe;

    // Check that both sides of the bar have a PE value above threshold.
    // If not, don't consider the hit.  Double sided readout is only
    // being used for the back HCal bars.  For the side HCal, just
    // use the maximum PE as before.
    ldmx::HcalID id(hcal_hit.getID());
    if ((id.section() == ldmx::HcalID::BACK) &&
        (hcal_hit.getMinPE() < back_min_PE_))
      continue;

    num_valid_hits++ ;

    // Find the maximum PE in the list
    if (max_PE < pe) {
      max_PE = pe;
      max_PE_hit = &hcal_hit;
    }
  }

  ldmx_log(info) << "There are " <<  num_valid_hits << " / "  << num_total_hits << " HCal hits read out, total PE of " << total_PE;
  // If the maximum PE found is below threshold, it passes the veto.
  bool passes_veto = (max_PE < total_PE_threshold_);
  ldmx_log(info) << "HCAL veto passed? " << passes_veto;

  ldmx::HcalVetoResult result;
  result.setVetoResult(passes_veto);
  result.setMaxPEHit(*max_PE_hit);
  result.setTotalPE(total_PE);
  

  if (passes_veto) {
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }

  event.add(output_coll_name_, result);
}
}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalVetoProcessor);
