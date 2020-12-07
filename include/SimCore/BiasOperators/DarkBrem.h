#ifndef SIMCORE_BIASOPERATORS_DARKBREM_H
#define SIMCORE_BIASOPERATORS_DARKBREM_H

//----------//
//   LDMX   //
//----------//
#include "SimCore/DarkBrem/G4eDarkBremsstrahlung.h"
#include "SimCore/XsecBiasingOperator.h"

namespace simcore {
namespace biasoperators {

/**
 * Bias operator for the dark brem process
 */
class DarkBrem : public XsecBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Calls base class constructor and allows
   * access to configuration parameters.
   */
  DarkBrem(std::string name, const ldmx::Parameters& p);

  /**
   * Destructor
   *
   * Blank right now
   */
  ~DarkBrem() {}

  /**
   * Calculate the biased cross section given the
   * input process and track. This allows for us
   * to have access to the current information about the 
   * track while calculating the biased cross section.
   *
   * @see XsecBiasingOperator::BiasedXsec
   *
   * @param[in] track const pointer to track to Bias
   * @param[in] callingProcess process that might be biased by this operator
   * @return Method that returns the biasing operation that will be used
   *         to bias the occurence of events.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track, const G4BiasingProcessInterface* callingProcess) final override;

  /// Return the name of the process this operator biases
  virtual std::string getProcessToBias() const {
    return ldmx::darkbrem::G4eDarkBremsstrahlung::PROCESS_NAME;
  }

  /// Return the name of the particle this operator biases
  virtual std::string getParticleToBias() const { return "e-"; }

  /// Return the volume this operator biases
  virtual std::string getVolumeToBias() const { return volume_; }

  /**
   * Record the configuration of this biasing operator into the run header
   *
   * @param[in,out] header RunHeader to record configuration to
   */
  virtual void RecordConfig(ldmx::RunHeader& header) const;

 protected:
  /**
   * DEBUG FUNCTION
   * This function is called by the biasing interface class during PostStepDoIt.
   * You can observe the particle change that was produced by the process
   * and the weight that will be multiplied into this particle change.
   *
   * This is called inside G4VBiasingOperator::ReportOperationApplied
   * which is called inside G4BiasingProcessInterface::PostStepDoIt
  void OperationApplied(const G4BiasingProcessInterface* callingProcess,
          G4BiasingAppliedCase biasingCase,
          G4VBiasingOperation* operationApplied,
          G4double weight,
          G4VBiasingOperation* finalStateOpApplied,
          const G4VParticleChange* particleChangeProduced
          ) {
      std::string currentProcess =
  callingProcess->GetWrappedProcess()->GetProcessName(); if
  (currentProcess.compare(this->getProcessToBias()) == 0) { std::cout << "DB
  Final State Biasing Operator Applied: "
              << callingProcess->GetProcessName()
              << " -> " << weight*particleChangeProduced->GetWeight()
              << std::endl;
      }
  }
   */

 private:
  /// volume we want to bias in
  std::string volume_;

  /// factor we want to bias by
  double factor_;

  /// should we bias all electrons? (or only the primary)
  bool bias_all_;

};  // DarkBrem
}  // namespace biasoperators
}  // namespace simcore

#endif  // SIMCORE_DARKBREM_DARKBREMXSECBIASINGOPERATOR_H
