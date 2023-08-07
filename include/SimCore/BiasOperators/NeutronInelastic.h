#ifndef SIMCORE_BIASOPERATORS_NEUTRONINELASTIC_H_
#define SIMCORE_BIASOPERATORS_NEUTRONINELASTIC_H_

#include "SimCore/XsecBiasingOperator.h"

namespace simcore {
namespace biasoperators {

/**
 * Bias the neutron inelastic collsions
 */
class NeutronInelastic : public XsecBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Calls parent constructor and allows
   * accesss to configuration parameters.
   */
  NeutronInelastic(std::string name, const framework::config::Parameters& p);

  /** Destructor */
  virtual ~NeutronInelastic() = default;

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the neutron inelactic hadronic interactions.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) final override;

  /// Return the process to bias
  std::string getProcessToBias() const override { return "neutronInelastic"; }

  /// Return the particle to bias
  std::string getParticleToBias() const override { return "neutron"; }

  /// Return the volume to bias in
  std::string getVolumeToBias() const override { return volume_; }

  /**
   * Record the configuration to the run header
   *
   * @param[in,out] header RunHeader to record to
   */
  void RecordConfig(ldmx::RunHeader& header) const override {
    header.setStringParameter("BiasOperator::NeutronInelastic::Volume",
                              volume_);
    header.setFloatParameter("BiasOperator::NeutronInelastic::Factor", factor_);
    header.setFloatParameter("BiasOperator::NeutronInelastic::Threshold",
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

#endif  // SIMCORE_BIASOPERATORS_NEUTRONINELASTIC_H_
