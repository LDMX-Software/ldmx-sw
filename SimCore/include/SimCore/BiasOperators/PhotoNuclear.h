
#ifndef SIMCORE_BIASOPERATORS_PHOTONUCLEAR_H_
#define SIMCORE_BIASOPERATORS_PHOTONUCLEAR_H_

#include "SimCore/XsecBiasingOperator.h"

namespace simcore {
namespace biasoperators {

/**
 * Bias the Photon-Nuclear process
 */
class PhotoNuclear : public XsecBiasingOperator {
 public:
  /** Constructor */
  PhotoNuclear(std::string name, const framework::config::Parameters& p);

  /** Method called at the beginning of a run. */
  void StartRun() override;

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the occurence of photonuclear events.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) final override;

  /// return the process we want to bias
  std::string getProcessToBias() const override { return "photonNuclear"; }

  /// return the particle that we want to bias
  std::string getParticleToBias() const override { return "gamma"; }

  /// return the volume we want to bias within
  std::string getVolumeToBias() const override { return volume_; }

  /// record the configuration into the run header
  void RecordConfig(ldmx::RunHeader& h) const override {
    h.setStringParameter("BiasOperators::PhotoNuclear::Volume", volume_);
    h.setFloatParameter("BiasOperators::PhotoNuclear::Threshold", threshold_);
    h.setFloatParameter("BiasOperators::PhotoNuclear::Factor", factor_);
    h.setIntParameter("BiasOperators::PhotoNuclear::Bias Conv Down",
                      down_bias_conv_);
    h.setIntParameter("BiasOperators::PhotoNuclear::Only Children Of Primary",
                      only_children_of_primary_);
  }

 private:
  /** Geant4 gamma conversion process name. */
  static const std::string CONVERSION_PROCESS;

  /** Cross-section biasing operation for conversion process */
  G4BOptnChangeCrossSection* emXsecOperation{nullptr};

  /** Unbiased photonuclear xsec. */
  double pnXsecUnbiased_{0};

  /** Biased photonuclear xsec. */
  double pnXsecBiased_{0};

  /** Volume we are going to bias within */
  std::string volume_;

  /** minimum kinetic energy [MeV] for a track to be biased */
  double threshold_;

  /** factor to bias PN by */
  double factor_;

  /// Should we down-bias the gamma conversion process?
  bool down_bias_conv_;

  /// Should we restrict biasing to only children of primary?
  bool only_children_of_primary_;

};  // PhotoNuclear
}  // namespace biasoperators
}  // namespace simcore

#endif  // SIMCORE_BIASOPERATORS_PHOTONUCLEAR_H_
