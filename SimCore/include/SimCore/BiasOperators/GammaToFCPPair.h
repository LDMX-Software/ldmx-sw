#ifndef SIMCORE_BIASOPERATORS_GAMMATOFCPPAIR_H_
#define SIMCORE_BIASOPERATORS_GAMMATOFCPPAIR_H_

#include "SimCore/BiasOperators/XsecBiasingOperator.h"

namespace simcore {
namespace biasoperators {

/**
 * Bias the Gamma to FCP Pair process
 */
class GammaToFCPPair : public XsecBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Calls parent constructor and allows
   * access to configuration parameters.
   */
  GammaToFCPPair(std::string name, const framework::config::Parameters& p);

  /** Destructor */
  virtual ~GammaToFCPPair() = default;

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the conversion of gammas to fcp pairs.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) override;

  /// Return the process to bias
  std::string getProcessToBias() const override { return "GammaToFCPPair"; }

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
    header.setStringParameter("BiasOperator::GammaToFCPPair::Volume", volume_);
    header.setFloatParameter("BiasOperator::GammaToFCPPair::Factor", factor_);
    header.setFloatParameter("BiasOperator::GammaToFCPPair::Threshold",
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

#endif  // SIMCORE_BIASOPERATORS_GAMMATOFCPPAIR_H_
