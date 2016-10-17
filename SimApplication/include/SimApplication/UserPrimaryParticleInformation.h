#ifndef SimApplication_UserPrimaryParticleInformation_h
#define SimApplication_UserPrimaryParticleInformation_h

// Geant4
#include "G4VUserPrimaryParticleInformation.hh"

class UserPrimaryParticleInformation : public G4VUserPrimaryParticleInformation {

    public:

        UserPrimaryParticleInformation() : hepEvtStatus(-1) {
        }

        virtual ~UserPrimaryParticleInformation() {
        }

        void setHepEvtStatus(int theHepEvtStatus) {
            hepEvtStatus = theHepEvtStatus;
        }

        int getHepEvtStatus() {
            return hepEvtStatus;
        }

        void Print() const {
        }

    private:

        int hepEvtStatus;
};

#endif
