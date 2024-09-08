/**
 * @file PrimaryGeneratorAction.h
 * @brief Class implementing the Geant4 primary generator action
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_G4USER_PRIMARYGENERATORACTION_H
#define SIMCORE_G4USER_PRIMARYGENERATORACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>
#include <memory>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4VUserPrimaryGeneratorAction.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

// Forward declarations
class G4Event;

namespace simcore::g4user {

/**
 * @class PrimaryGeneratorAction
 * @brief Implementation of Geant4 primary generator action
 */
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
 public:
  /*
   * Constructor
   *
   * @param parameters The parameters used to configure the primary
   *                   generator action.
   */
  PrimaryGeneratorAction(const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~PrimaryGeneratorAction() = default;

  /**
   * Generate primaries for the event.
   *
   * This is called by the RunManager before being
   * given to the EventManager to process. This means
   * we must create the UserEventInformation here so
   * that it is accessible for including the weights imported
   * by the primary generators.
   *
   * @see G4RunManager::GenerateEvent for where this method is called
   * @see G4RunManager::ProcessOneEvent for where G4RunManager::GenerateEvent is
   * called
   *
   * Set UserInformation for primary vertices if they haven't been set before.
   *
   * Some features downstream of the primaries require certain user info to
   * function properly. This ensures that it happens.
   *
   * Makes sure that each particle on each primary vertex has
   *  1. A defined UserPrimaryParticleInformation member
   *  2. The HepEvtStatus for this primary info is non-zero
   *
   * If we are passed configuration to smear the beam spot,
   * we smear the beam spot around the spot generated by the primary generator.
   *
   * If we are configured to time-shift the primaries,
   * we shift them so that t=0 coincides with primaries
   * arriving at (or coming from) the target.
   *
   * @param event The Geant4 event.
   */
  void GeneratePrimaries(G4Event* event) override;

 private:
  /**
   * Flag denoting whether the vertex position of a particle
   * should be smeared.
   */
  bool useBeamspot_{false};

  /** Extent of the beamspot in x. */
  double beamspotXSize_{0};

  /** Extent of the beamspot in y. */
  double beamspotYSize_{0};

  /** Extent of the beamspot in y. */
  double beamspotZSize_{0.};

  /**
   * Should we time-shift so that the primary vertices arrive (or originate)
   * at t=0ns at z=0mm?
   *
   * @note This should remain true unless the user knows what they are doing!
   */
  bool time_shift_primaries_{true};

};  // PrimaryGeneratorAction

}  // namespace simcore::g4user

#endif  // SIMCORE_G4USER_PRIMARYGENERATORACTION_H
