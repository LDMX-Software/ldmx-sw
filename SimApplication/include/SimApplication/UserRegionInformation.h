#ifndef SIMAPPLICATION_USERREGIONINFORMATION_H_
#define SIMAPPLICATION_USERREGIONINFORMATION_H_

// Geant4
#include "G4VUserRegionInformation.hh"

namespace sim {

class UserRegionInformation: public G4VUserRegionInformation {

public:

    UserRegionInformation(bool storeSecondaries);

    virtual ~UserRegionInformation();

    void Print() const;

    bool getStoreSecondaries() const;

private:

    bool storeSecondaries_;
};

}

#endif
