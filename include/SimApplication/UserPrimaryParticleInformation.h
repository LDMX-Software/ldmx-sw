#ifndef SIMAPPLICATION_USERPRIMARYPARTICLEINFORMATION_H_
#define SIMAPPLICATION_USERPRIMARYPARTICLEINFORMATION_H_

// Geant4
#include "G4VUserPrimaryParticleInformation.hh"

namespace sim {

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

}

#endif
