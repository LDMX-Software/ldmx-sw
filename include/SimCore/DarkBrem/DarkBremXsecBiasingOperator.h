/**
 * @file DarkBremXsecBiasingOperator.h
 * @brief Geant4 Biasing Operator used to bias the occurence of dark brem
 *        events by modifying the cross-section.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_DARKBREM_DARKBREMXSECBIASINGOPERATOR_H
#define SIMCORE_DARKBREM_DARKBREMXSECBIASINGOPERATOR_H

//----------//
//   LDMX   //
//----------//
#include "SimCore/DarkBrem/G4eDarkBremsstrahlung.h"
#include "SimCore/XsecBiasingOperator.h"

namespace ldmx {
namespace darkbrem {

class DarkBremXsecBiasingOperator : public XsecBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Calls base class constructor.
   */
  DarkBremXsecBiasingOperator(std::string name) : XsecBiasingOperator(name) {}

  /**
   * Destructor
   *
   * Blank right now
   */
  ~DarkBremXsecBiasingOperator() {}

  /**
   * This the following protected member variables from XsecBiasingOperator:
   *  - biasAll_ : If true, bias all particles connected to the dark brem
   * process; otherwise, only bias the primary particle (ParentID == 0)
   *  - xsecFactor_ : Factor to multiply cross section by
   *  - xsecOperator : Geant4 biasing operator to use
   *
   * @param[in] track const pointer to track to Bias
   * @param[in] callingProcess process that might be biased by this operator
   * @return Method that returns the biasing operation that will be used
   *         to bias the occurence of events.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track, const G4BiasingProcessInterface* callingProcess);

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

  /// Return the name of the process this operator biases
  virtual std::string getProcessToBias() {
    return G4eDarkBremsstrahlung::PROCESS_NAME;
  }

};  // DarkBremXsecBiasingOperator
}  // namespace darkbrem
}  // namespace ldmx

#endif  // SIMCORE_DARKBREM_DARKBREMXSECBIASINGOPERATOR_H
