#ifndef SIMAPPLICATION_USERPRIMARYPARTICLEINFORMATION_H_
#define SIMAPPLICATION_USERPRIMARYPARTICLEINFORMATION_H_

// Geant4
#include "G4VUserPrimaryParticleInformation.hh"

namespace sim {

class UserPrimaryParticleInformation : public G4VUserPrimaryParticleInformation {

    public:

        UserPrimaryParticleInformation() : hepEvtStatus_(-1) {
        }

        virtual ~UserPrimaryParticleInformation() {
        }

        void setHepEvtStatus(int theHepEvtStatus) {
            hepEvtStatus_ = theHepEvtStatus;
        }

        int getHepEvtStatus() {
            return hepEvtStatus_;
        }

        void Print() const {
        }

    private:

        int hepEvtStatus_;
};

}

#endif
