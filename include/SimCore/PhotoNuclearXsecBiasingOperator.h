/**
 * @file XsecBiasingPlugin.h
 * @brief Geant4 Biasing Operator used to bias the occurence of photonuclear
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef BIASING_PHOTONUCLEARXSECBIASINGOPERATOR_H_
#define BIASING_PHOTONUCLEARXSECBIASINGOPERATOR_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//------------//
//   Geant4   //
//------------//
#include "G4BOptnChangeCrossSection.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4BiasingProcessSharedData.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4VBiasingOperator.hh"

//----------//
//   LDMX   //
//----------//
#include "XsecBiasingOperator.h"

namespace simcore {

class PhotoNuclearXsecBiasingOperator : public XsecBiasingOperator {
 public:
  /** Constructor */
  PhotoNuclearXsecBiasingOperator(std::string name);

  /** Destructor */
  ~PhotoNuclearXsecBiasingOperator();

  /** Method called at the beginning of a run. */
  void StartRun();

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the occurence of photonuclear events.
   */
  G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track, const G4BiasingProcessInterface* callingProcess);

 protected:
  virtual std::string getProcessToBias() { return PHOTONUCLEAR_PROCESS; }

 private:
  /** Geant4 photonuclear process name. */
  static const std::string PHOTONUCLEAR_PROCESS;

  /** Geant4 gamma conversion process name. */
  static const std::string CONVERSION_PROCESS;

  /** Cross-section biasing operation */
  G4BOptnChangeCrossSection* emXsecOperation{nullptr};

  /** Unbiased photonuclear xsec. */
  double pnXsecUnbiased_{0};

  /** Biased photonuclear xsec. */
  double pnXsecBiased_{0};

};  // PhotoNuclearXsecBiasingOperator
}  // namespace simcore

#endif  // SIMPLUGINS_PHOTONUCLEARXSECBIASINGOPERATOR_H_
