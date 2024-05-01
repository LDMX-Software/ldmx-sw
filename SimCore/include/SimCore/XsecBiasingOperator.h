#ifndef SIMCORE_XSECBIASINGOPERATOR_H_
#define SIMCORE_XSECBIASINGOPERATOR_H_

#include "Framework/Configure/Parameters.h"
#include "Framework/RunHeader.h"
#include "SimCore/Factory.h"

//------------//
//   Geant4   //
//------------//
#include "G4BOptnChangeCrossSection.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4BiasingProcessSharedData.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4KaonZeroLong.hh"
#include "G4Neutron.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4VBiasingOperator.hh"

namespace simcore {

/**
 * Our specialization of the biasing operator used with Geant4.
 *
 * This specialization accomplishes three main tasks.
 * 1. Allows any derived class to be dynamically loaded after
 *    using the declaration macro given below.
 * 2. Interfaces with the derived class using our parameters
 *    class.
 * 3. Pre-defines the necessary biasing operation so the derived
 *    class only needs to worry about calculating the biased
 *    xsec.
 */
class XsecBiasingOperator : public G4VBiasingOperator {
 public:
  /**
   * Constructor
   *
   * Here, we define a unique name for this biasing operator
   * and are given the configuration parameters loaded from the
   * python script.
   *
   * @param[in] name unique instance name for this biasing operator
   * @param[in] parameters python configuration parameters
   */
  XsecBiasingOperator(std::string name,
                      const framework::config::Parameters& parameters);

  /**
   * The BiasingOperator factory
   */
  using Factory =
      ::simcore::Factory<XsecBiasingOperator,
                         std::shared_ptr<XsecBiasingOperator>, std::string,
                         const framework::config::Parameters&>;

  /** Destructor */
  virtual ~XsecBiasingOperator() = default;

  /**
   * Propose a biasing operation for the current track and calling process.
   *
   * @note Returning `0` from this function will mean that the current track
   * and process will not be biased.
   *
   * @see BiasedXsec for a method that allows the derived class to not
   * interact with the biasing operation itself.
   *
   * @param[in] track handle to current track that could be biased
   * @param[in] callingProcess handle to process asking if it should be biased
   * @return the biasing operation with the biased xsec
   */
  virtual G4VBiasingOperation* ProposeOccurenceBiasingOperation(
      const G4Track* track,
      const G4BiasingProcessInterface* callingProcess) = 0;

  /**
   * Method called at the beginning of a run.
   *
   * This makes sure that the process we want to bias can
   * be biased and constructs a corresponding biasing operation.
   *
   * It can be over-written, but then the derived class should
   * call `XsecBiasingOperator::StartRun()` at the beginning of
   * their own StartRun.
   */
  void StartRun();

  /**
   * Return the process whose cross-section will be biased.
   *
   * We need this to be able to check that the process
   * was biased before creating the biasing operator.
   */
  virtual std::string getProcessToBias() const = 0;

  /**
   * Return the particle which should be biased.
   *
   * We need this to be able to tell the physics
   * list which particle to bias.
   * @see RunManager::setupPhysics
   */
  virtual std::string getParticleToBias() const = 0;

  /**
   * Return the volume which should be biased.
   *
   * We need this to be able to tell the detector
   * construction which volumes to attach this
   * operator to.
   */
  virtual std::string getVolumeToBias() const = 0;

  /**
   * Record the configuration of this
   * biasing operator into the run header.
   *
   * @param[in,out] header RunHeader to write configuration to
   */
  virtual void RecordConfig(ldmx::RunHeader& header) const = 0;

 protected:
  /**
   * Helper method for passing a biased interaction length
   * to the Geant4 biasing framework.
   *
   * Use like:
   *
   *    return BiasedXsec(biased_xsec);
   *
   * inside of ProposeOccurenceBiasingOperation when
   * you want to update the biased cross section.
   *
   * @param[in] biased_xsec the biased cross section
   * @return the biasing operation with the input biased cross section
   */
  G4VBiasingOperation* BiasedXsec(double biased_xsec) {
    xsecOperation_->SetBiasedCrossSection(biased_xsec);
    xsecOperation_->Sample();
    return xsecOperation_;
  }

  /**
   * Check if the given processed is being biased.
   *
   * @param process Process of interest
   * @return true if the process is being biased, false otherwise
   */
  bool processIsBiased(std::string process);

  /** Cross-section biasing operation. */
  G4BOptnChangeCrossSection* xsecOperation_{nullptr};

  /** Process manager associated with the particle of interest. */
  G4ProcessManager* processManager_{nullptr};

  /**
   * Do *not* propose any biasing on final states.
   */
  G4VBiasingOperation* ProposeFinalStateBiasingOperation(
      const G4Track*, const G4BiasingProcessInterface*) {
    return nullptr;
  }

  /**
   * Do *not* propose any non-physics biasing.
   */
  G4VBiasingOperation* ProposeNonPhysicsBiasingOperation(
      const G4Track*, const G4BiasingProcessInterface*) {
    return nullptr;
  }

};  // XsecBiasingOperator
}  // namespace simcore

/**
 * @macro DECLARE_XSECBIASINGOPERATOR
 *
 * Defines a builder for the declared class
 * and then registers the class as a biasing operator.
 */
#define DECLARE_XSECBIASINGOPERATOR(CLASS)                                  \
  namespace {                                                               \
  auto v = ::simcore::XsecBiasingOperator::Factory::get().declare<CLASS>(); \
  }

#endif  // SIMCORE_XSECBIASINGOPERATOR_H_
