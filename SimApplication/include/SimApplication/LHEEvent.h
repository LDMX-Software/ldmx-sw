#ifndef SIMAPPLICATION_LHEEVENT_H_
#define SIMAPPLICATION_LHEEVENT_H_

// LDMX
#include "SimApplication/LHEParticle.h"

// STL
#include <vector>

namespace sim {

class LHEEvent {

    public:

        LHEEvent(std::string&);

        virtual ~LHEEvent();

        int getNUP();

        int getIDPRUP();

        double getXWGTUP();

        double getSCALUP();

        double getAQEDUP();

        double getAQCDUP();

        void addParticle(LHEParticle* particle);

        const std::vector<LHEParticle*>& getParticles();

    private:

        int nup;
        int idprup;
        double xwgtup;
        double scalup;
        double aqedup;
        double aqcdup;

        std::vector<LHEParticle*> particles;
};

}

#endif
