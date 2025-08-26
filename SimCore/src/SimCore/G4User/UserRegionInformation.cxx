#include "SimCore/G4User/UserRegionInformation.h"

namespace simcore {

UserRegionInformation::UserRegionInformation(bool aStoreSecondaries)
    : store_secondaries_(aStoreSecondaries) {}

bool UserRegionInformation::getStoreSecondaries() const {
  return store_secondaries_;
}

void UserRegionInformation::Print() const {}

}  // namespace simcore
