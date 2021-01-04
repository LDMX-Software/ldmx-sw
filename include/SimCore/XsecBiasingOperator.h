/**
 * @file XsecBiasingPlugin.h
 * @brief Geant4 Biasing Operator used to bias the occurence of photonuclear
 *        events by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef BIASING_XSECBIASINGOPERATOR_H_
#define BIASING_XSECBIASINGOPERATOR_H_

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

namespace ldmx {

class XsecBiasingOperator : public G4VBiasingOperator {
 public:
  /** Constructor */
  XsecBiasingOperator(std::string name);

  /** Destructor */
  virtual ~XsecBiasingOperator();

  /** Method called at the beginning of a run. */
  void StartRun();

  /**
   * @return Method that returns the biasing operation that will be used
   *         to bias the occurence of photonuclear events.
   */
  virtual G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) = 0;

  /** Bias all particles of the given type. */
  void biasAll() { biasAll_ = true; };

  /** Bias only the incident particle. */
  void biasIncident() { biasIncident_ = true; };

  /**
   * Disable the biasing down of EM when either the
   * photonuclear or gamma->mu+mu- xsections are biased up. This was
   * added to remain backwards compatible with the old biasing
   * scheme.
   */
  void disableBiasDownEM() { biasDownEM_ = false; };

  /** Set the particle type to bias. */
  void setParticleType(std::string particleType) {
    particleType_ = particleType;
  };

  /** Set the minimum energy required to bias the particle. */
  void setThreshold(double threshold) { threshold_ = threshold; };

  /** Set the factor by which the xsec will be enhanced. */
  void setBiasFactor(double xsecFactor) { xsecFactor_ = xsecFactor; };

 protected:
  /**
   * Check if the given processed is being biased.
   *
   * @param process Process of interest
   * @return true if the process is being biased, false otherwise
   */
  bool processIsBiased(std::string process);

  /** Return the process whose cross-section will be biased. */
  virtual std::string getProcessToBias() = 0;

  /** Cross-section biasing operation. */
  G4BOptnChangeCrossSection* xsecOperation{nullptr};

  /** Process manager associated with the particle of interest. */
  G4ProcessManager* processManager_{nullptr};

  /**
   * Flag indicating whether EM should be biased down when either
   * photonuclear or gammatomumu is biased up.  This flag is only
   * valid when either PhotoNuclearXsecBiasingOperator or
   * GammaToMuMuXsecBiasingOperator is used.
   */
  bool biasDownEM_{true};

  /** Flag indicating whether all particles should be biased. */
  bool biasAll_{false};

  /** Flag indicating whether to bias only the incident particle. */
  bool biasIncident_{false};

  /** The particle type to bias. */
  std::string particleType_{""};

  /** The minimum energy required to apply the biasing operation. */
  double threshold_{0};

  /** Factor to multiply the xsec by. */
  double xsecFactor_{0};

  //--------//
  // Unused //
  //--------//
  G4VBiasingOperation* ProposeFinalStateBiasingOperation(
      const G4Track*, const G4BiasingProcessInterface*) {
    return nullptr;
  }

  G4VBiasingOperation* ProposeNonPhysicsBiasingOperation(
      const G4Track*, const G4BiasingProcessInterface*) {
    return nullptr;
  }

};  // XsecBiasingOperator
}  // namespace ldmx

#endif  // SIMPLUGINS_XSECBIASINGOPERATOR_H_
