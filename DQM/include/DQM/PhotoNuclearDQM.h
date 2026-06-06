#ifndef DQM_PHOTONUCLEARDQM_H
#define DQM_PHOTONUCLEARDQM_H

#include "DQM/NuclearDQM.h"
#include "SimCore/Event/PhotonuclearInteraction.h"
#include "Tools/AnalysisUtils.h"

namespace dqm {

class PhotoNuclearDQM : public NuclearDQM {
 public:
  PhotoNuclearDQM(const std::string& name, framework::Process& process);
  virtual ~PhotoNuclearDQM() = default;

  void configure(framework::config::Parameters& parameters) override;
  void analyze(const framework::Event& event) override;

 private:
  /** Fill recoil-electron vertex histograms. */
  void findRecoilProperties(const ldmx::SimParticle* recoil);

  /**
   * Analyze the PhotonuclearInteraction collection when present.
   * Fills target Z/A, cascade multiplicity, and descendant histograms.
   */
  void analyzeInteractionDetails(const framework::Event& event);

  std::string pn_collection_name_;
  std::string pn_pass_name_;
};

}  // namespace dqm

#endif  // DQM_PHOTONUCLEARDQM_H
