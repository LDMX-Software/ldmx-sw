/**
 * @file APrimeToFCPPair.h
 * @brief Biasing operator for the A' -> fcp+ fcp- process.
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_BIASOPERATORS_APRIMETOFCPPAIR_H_
#define SIMCORE_BIASOPERATORS_APRIMETOFCPPAIR_H_

#include "SimCore/APrimeConversionToFCPs.h"
#include "SimCore/BiasOperators/XsecBiasingOperator.h"

namespace simcore {
namespace biasoperators {

/**
 * Bias the A' -> fcp+ fcp- process.
 *
 * Modeled after the GammaToMuPair biasing operator.
 */
class APrimeToFCPPair : public XsecBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Calls parent constructor and allows
   * access to configuration parameters.
   */
  APrimeToFCPPair(std::string name, const framework::config::Parameters& p);

  /** Destructor */
  virtual ~APrimeToFCPPair() = default;

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the conversion of A' to fcp pairs.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) override;

  /// Return the process to bias
  std::string getProcessToBias() const override {
    return APrimeConversionToFCPs::PROCESS_NAME;
  }

  /// Return the particle to bias (A' short name in Geant4)
  std::string getParticleToBias() const override { return "A^1"; }

  /// Return the volume to bias in
  std::string getVolumeToBias() const override { return volume_; }

  /**
   * Record the configuration to the run header
   *
   * @param[in,out] header RunHeader to record to
   */
  void RecordConfig(ldmx::RunHeader& header) const override {
    header.setStringParameter("BiasOperator::APrimeToFCPPair::Volume", volume_);
    header.setFloatParameter("BiasOperator::APrimeToFCPPair::Factor", factor_);
    header.setFloatParameter("BiasOperator::APrimeToFCPPair::Threshold",
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

#endif  // SIMCORE_BIASOPERATORS_APRIMETOFCPPAIR_H_
