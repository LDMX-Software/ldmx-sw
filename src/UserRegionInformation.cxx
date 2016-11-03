#include "SimApplication/UserRegionInformation.h"

namespace sim {

UserRegionInformation::UserRegionInformation(bool aStoreSecondaries)
    : storeSecondaries_(aStoreSecondaries) {
}

UserRegionInformation::~UserRegionInformation() {
}

bool UserRegionInformation::getStoreSecondaries() const {
    return storeSecondaries_;
}

void UserRegionInformation::Print() const {
}

}

