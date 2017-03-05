#ifndef SIMCORE_USERTRACKINFORMATION_H_
#define SIMCORE_USERTRACKINFORMATION_HH 1

#include "G4VUserTrackInformation.hh"

namespace ldmx {

    class UserTrackInformation : public G4VUserTrackInformation {

        public:

            UserTrackInformation() {;}

            virtual ~UserTrackInformation() {;}

            virtual void Print() const {}

            bool getSaveFlag() { return saveFlag_; }

            void setSaveFlag(bool saveFlag) { saveFlag_ = saveFlag; }

        private:

            bool saveFlag_;
    };
}

#endif
