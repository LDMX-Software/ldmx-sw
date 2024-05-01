/**
 * @file UserRegionInformation.h
 * @brief Class which provides extra information for a detector region
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_USERREGIONINFORMATION_H_
#define SIMCORE_USERREGIONINFORMATION_H_

// Geant4
#include "G4VUserRegionInformation.hh"

namespace simcore {

/**
 * @class UserRegionInformation
 * @brief Defines extra information for a detector region
 *
 * @note
 * This extension to the user region information has a flag indicating
 * whether secondary particles should be stored.  This flag is used
 * in the UserTrackingAction to determine whether or not a trajectory
 * is created for a track created in the region.
 */
class UserRegionInformation : public G4VUserRegionInformation {
 public:
  UserRegionInformation(bool storeSecondaries);

  virtual ~UserRegionInformation() = default;

  void Print() const;

  bool getStoreSecondaries() const;

 private:
  bool storeSecondaries_;
};

}  // namespace simcore

#endif
