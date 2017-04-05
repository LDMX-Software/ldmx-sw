#include "SimApplication/UserRegionInformation.h"

namespace ldmx {

    UserRegionInformation::UserRegionInformation(bool aStoreSecondaries) :
            storeSecondaries_(aStoreSecondaries) {
    }

    UserRegionInformation::~UserRegionInformation() {
    }

    bool UserRegionInformation::getStoreSecondaries() const {
        return storeSecondaries_;
    }

    void UserRegionInformation::Print() const {
    }

}

