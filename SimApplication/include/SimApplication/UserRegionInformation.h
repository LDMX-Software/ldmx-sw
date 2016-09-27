#ifndef SIMAPPLICATION_USERREGIONINFORMATION_HH_
#define SIMAPPLICATION_USERREGIONINFORMATION_HH_ 1

// Geant4
#include "G4VUserRegionInformation.hh"

class UserRegionInformation: public G4VUserRegionInformation {

public:

    UserRegionInformation(bool storeSecondaries);

    virtual ~UserRegionInformation();

    void Print() const;

    bool getStoreSecondaries() const;

private:

    bool storeSecondaries;
};

#endif
