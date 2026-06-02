#ifndef DQM_ECALSPELECTRONKINEMATICS_H
#define DQM_ECALSPELECTRONKINEMATICS_H

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * @class EcalSPElectronKinematics
 * @brief Extracts and histograms the kinematics of the primary electron
 *        at the ECal front-face scoring plane.
 *
 * Searches EcalScoringPlaneHits for the primary beam electron
 * (track ID 1, PDG ID 11) crossing plane 31 (ECal front face) in the
 * forward direction.  The hit energy, three-momentum, and transverse
 * position are stored in the event tree and histogrammed.
 *
 * ## Products
 * EcalSPEnergy        - total energy at ECal front face [MeV]
 * EcalSP{Px,Py,Pz}   - momentum components [MeV]
 * EcalSP{X,Y}        - transverse position at ECal front face [mm]
 */
class EcalSPElectronKinematics : public framework::Producer {
 public:
  EcalSPElectronKinematics(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  ~EcalSPElectronKinematics() = default;

  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;

 private:
  std::string ecal_sp_coll_name_;
  std::string ecal_sp_pass_name_;
};

}  // namespace dqm

#endif  // DQM_ECALSPELECTRONKINEMATICS_H
