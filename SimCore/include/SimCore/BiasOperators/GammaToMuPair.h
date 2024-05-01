#ifndef SIMCORE_BIASOPERATORS_GAMMATOMUPAIR_H_
#define SIMCORE_BIASOPERATORS_GAMMATOMUPAIR_H_

#include "SimCore/XsecBiasingOperator.h"

namespace simcore {
namespace biasoperators {

/**
 * Bias the Gamma to Mu Pair process
 */
class GammaToMuPair : public XsecBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Calls parent constructor and allows
   * accesss to configuration parameters.
   */
  GammaToMuPair(std::string name, const framework::config::Parameters& p);

  /** Destructor */
  virtual ~GammaToMuPair() = default;

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the conversion of gammas to muon pairs.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) final override;

  /// Return the process to bias
  std::string getProcessToBias() const override { return "GammaToMuPair"; }

  /// Return the particle to bias
  std::string getParticleToBias() const override { return "gamma"; }

  /// Return the volume to bias in
  std::string getVolumeToBias() const override { return volume_; }

  /**
   * Record the configuration to the run header
   *
   * @param[in,out] header RunHeader to record to
   */
  void RecordConfig(ldmx::RunHeader& header) const override {
    header.setStringParameter("BiasOperator::GammaToMuPair::Volume", volume_);
    header.setFloatParameter("BiasOperator::GammaToMuPair::Factor", factor_);
    header.setFloatParameter("BiasOperator::GammaToMuPair::Threshold",
                             threshold_);
  }

 private:
  /// The volume to bias in
  std::string volume_;

  /// The biasing factor
  double factor_;

  /// Minimum kinetic energy [MeV] to allow a track to be biased
  double threshold_;
};

}  // namespace biasoperators
}  // namespace simcore

#endif  // SIMCORE_BIASOPERATORS_GAMMATOMUPAIR_H_
