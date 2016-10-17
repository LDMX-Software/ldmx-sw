#ifndef SimApplication_UserRegionInformation_h
#define SimApplication_UserRegionInformation_h

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
