#ifndef SIMAPPLICATION_LHEPARTICLE_H_
#define SIMAPPLICATION_LHEPARTICLE_H_ 1

// STL
#include <string>
#include <vector>

class LHEParticle {

    public:

        LHEParticle(std::string&);

        int getIDUP();

        int getISTUP();

        int getMOTHUP(int);

        int getICOLUP(int);

        double getPUP(int);

        double getVTIMUP();

        double getSPINUP();

        void setMother(int, LHEParticle*);

    private:

        LHEParticle* mothers[2];

        int idup;
        int istup;
        int mothup[2];
        int icolup[2];
        double pup[5];
        double vtimup;
        int spinup;
};

#endif
