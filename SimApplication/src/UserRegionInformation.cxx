#include "SimApplication/UserRegionInformation.h"

namespace sim {

UserRegionInformation::UserRegionInformation(bool aStoreSecondaries)
    : storeSecondaries(aStoreSecondaries) {
}

UserRegionInformation::~UserRegionInformation() {
}

bool UserRegionInformation::getStoreSecondaries() const {
    return storeSecondaries;
}

void UserRegionInformation::Print() const {
}

}

