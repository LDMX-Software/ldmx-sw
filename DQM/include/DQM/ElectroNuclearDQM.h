#ifndef DQM_ELECTRONUCLEARDQM_H
#define DQM_ELECTRONUCLEARDQM_H

#include "DQM/NuclearDQM.h"
#include "Tools/AnalysisUtils.h"

namespace dqm {

/**
 * DQM analyzer for Geant4 electro-nuclear (EN) interactions.
 *
 * Finds the primary (recoil) electron in the SimParticle map and collects
 * its daughters that were produced by the electronNuclear process. These
 * products are then classified and their kinematics histogrammed using the
 * shared machinery in NuclearDQM.
 */
class ElectroNuclearDQM : public NuclearDQM {
 public:
  ElectroNuclearDQM(const std::string& name, framework::Process& process);
  virtual ~ElectroNuclearDQM() = default;

  void configure(framework::config::Parameters& parameters) override;
  void analyze(const framework::Event& event) override;

 private:
  /**
   * Fill histograms describing the EN electron and the interaction vertex.
   *
   * Uses the EN electron's initial energy and vertex position, and takes
   * the vertex position from the first EN daughter (where the reaction
   * occurred).
   */
  void findENElectronProperties(
      const ldmx::SimParticle* en_electron,
      const std::vector<const ldmx::SimParticle*>& en_daughters);

  /**
   * Fill "reconstructable" kinematic histograms applying the semi-inclusive
   * acceptance cuts from the analysis definition:
   *   - theta < 80 deg from beam axis
   *   - |p| > 100 MeV/c  for charged pions
   *   - |p| > 800 MeV/c  for protons and kaons
   *   - KE  > 1000 MeV   for neutrons
   *   - KE  > 2000 MeV for pi0
   *
   * Fills recon_* and en_recon_* histograms mirroring the extended kinematics
   * set, plus event_type_recon for the reconstructable subset classification.
   */
  void findReconstructableKinematics(
      const std::vector<const ldmx::SimParticle*>& en_daughters);
};

}  // namespace dqm

#endif  // DQM_ELECTRONUCLEARDQM_H
