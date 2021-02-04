/**
 * @file TargetDarkBremFilter.h
 * @class TargetDarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out
 *        events that don't result in a dark brem inside a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef BIASING_TARGETDARKBREMFILTER_H_
#define BIASING_TARGETDARKBREMFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"
#include "G4VPhysicalVolume.hh"

//------------//
//    LDMX    //
//------------//
#include "SimCore/UserAction.h"

namespace biasing {

/**
 * @class TargetDarkBremFilter
 *
 * This class is meant to filter for events that produce a dark brem matching
 * originating in the target and matching the following parameters.
 *
 *      threshold: minimum energy [MeV] A' needs to have
 *
 * @see TargetBremFilter
 * This filter is designed similar to the target brem filter where we check the
 * secondaries of the primary electron if it is stopping within the target or
 * if it is leaving the target region.
 */
class TargetDarkBremFilter : public simcore::UserAction {
 public:
  /**
   * Class constructor.
   *
   * Retrieve the necessary configuration parameters
   */
  TargetDarkBremFilter(const std::string& name,
                       framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  ~TargetDarkBremFilter() {}

  /**
   * Get the types of actions this class can do
   *
   * @return list of action types this class does
   */
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::STEPPING};
  }

  /**
   * Looking for A' while primary is stepping.
   *
   * We make sure that the current track
   * is the primary electron that is within
   * the target region. Then if the track
   * is either stopped or leaving the target region,
   * we look through its secondaries for a good A'.
   *
   * @param[in] step current G4Step
   */
  void stepping(const G4Step* step);

 private:
  /**
   * Check if the volume is outside the target region
   *
   * @note will return true if vol is nullptr
   *
   * @param[in] vol G4VPhysicalVolume to check region
   * @returns true if vol is outside target region or nullptr
   */
  inline bool isOutsideTargetRegion(const G4VPhysicalVolume* vol) const {
    return vol ? isOutsideTargetRegion(vol->GetLogicalVolume()) : true;
  }

  /**
   * Check if the volume is outside the target region
   *
   * @note will return true if vol is nullptr.
   *
   * @param[in] vol G4LogicalVolume to check region
   * @returns true if vol is outside target region or nullptr or doesn't have a
   * region
   */
  inline bool isOutsideTargetRegion(const G4LogicalVolume* vol) const {
    if (!vol) return true;
    auto region = vol->GetRegion();
    return region ? (region->GetName().compareTo("target") != 0) : true;
  }

  /**
   * Helper to abort an event with a message
   *
   * Tells the RunManger to abort the current event
   * after displaying the input message.
   *
   * @param[in] reason reason for aborting the event
   */
  void AbortEvent(const std::string& reason) const;

 private:
  /**
   * Minimum energy [MeV] that the A' should have to keep the event.
   *
   * Also used by PartialEnergySorter to determine
   * which tracks should be processed first.
   *
   * Parameter Name: 'threshold'
   */
  double threshold_;

};  // TargetDarkBremFilter
}  // namespace biasing

#endif  // BIASING_TARGETDARKBREMFILTER_H__
