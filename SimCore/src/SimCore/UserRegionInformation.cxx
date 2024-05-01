#include "SimCore/UserRegionInformation.h"

namespace simcore {

UserRegionInformation::UserRegionInformation(bool aStoreSecondaries)
    : storeSecondaries_(aStoreSecondaries) {}

bool UserRegionInformation::getStoreSecondaries() const {
  return storeSecondaries_;
}

void UserRegionInformation::Print() const {}

}  // namespace simcore
