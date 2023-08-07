#ifndef SIMCORE_BIASOPERATORS_K0LONGINELASTIC_H_
#define SIMCORE_BIASOPERATORS_K0LONGINELASTIC_H_

#include "SimCore/XsecBiasingOperator.h"

namespace simcore {
namespace biasoperators {

/**
 * Bias the k0 long inelastic collisions
 */
class K0LongInelastic : public XsecBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Calls parent constructor and allows
   * accesss to configuration parameters.
   */
  K0LongInelastic(std::string name, const framework::config::Parameters& p);

  /** Destructor */
  virtual ~K0LongInelastic() = default;

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the K0 Long inelactic hadronic interactions.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) final override;

  /// Return the process to bias
  std::string getProcessToBias() const override { return "kaon0LInelastic"; }

  /// Return the particle to bias
  std::string getParticleToBias() const override { return "kaon0L"; }

  /// Return the volume to bias in
  std::string getVolumeToBias() const override { return volume_; }

  /**
   * Record the configuration to the run header
   *
   * @param[in,out] header RunHeader to record to
   */
  void RecordConfig(ldmx::RunHeader& header) const override {
    header.setStringParameter("BiasOperator::K0LongInelastic::Volume", volume_);
    header.setFloatParameter("BiasOperator::K0LongInelastic::Factor", factor_);
    header.setFloatParameter("BiasOperator::K0LongInelastic::Threshold",
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

#endif  // SIMCORE_BIASOPERATORS_K0LONGINELASTIC_H_
