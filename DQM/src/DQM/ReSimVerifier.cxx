#include "DQM/ReSimVerifier.h"
namespace dqm {

void ReSimVerifier::configure(framework::config::Parameters &parameters) {
  hcalSimHitsCollection_ =
      parameters.getParameter<std::string>("hcal_sim_coll_name");
  simPassName_ = parameters.getParameter<std::string>("sim_pass_name");
  reSimPassName_ = parameters.getParameter<std::string>("resim_pass_name");
  hcalReSimHitsCollection_ =
      parameters.getParameter<std::string>("hcal_resim_coll_name");
  stop_on_error = parameters.getParameter<bool>("stop_on_error");
  tolerance = parameters.getParameter<double>("tolerance");
}
void ReSimVerifier::analyze(const framework::Event &event) {
  const auto hcalSimHits = event.getCollection<ldmx::SimCalorimeterHit>(
      hcalSimHitsCollection_, simPassName_);

  const auto hcalReSimHits = event.getCollection<ldmx::SimCalorimeterHit>(
      hcalSimHitsCollection_, reSimPassName_);

  for (auto i{0}; i < hcalSimHits.size(); ++i) {
    auto hit{hcalSimHits[i]};
    auto rehit{hcalReSimHits[i]};
    if (hit.getEdep() != rehit.getEdep()) {
      throw 32;
    }
  }

}  // Analyze
}  // namespace dqm
DECLARE_ANALYZER_NS(dqm, ReSimVerifier);
