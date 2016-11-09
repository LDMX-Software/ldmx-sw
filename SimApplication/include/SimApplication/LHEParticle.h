#ifndef SIMAPPLICATION_LHEPARTICLE_H_
#define SIMAPPLICATION_LHEPARTICLE_H_

// STL
#include <string>
#include <vector>

namespace sim {

class LHEParticle {

    public:

        LHEParticle(std::string&);

        int getIDUP() const;

        int getISTUP() const;

        int getMOTHUP(int) const;

        int getICOLUP(int) const;

        double getPUP(int) const;

        double getVTIMUP() const;

        double getSPINUP() const;

        void setMother(int, LHEParticle*);

        LHEParticle* getMother(int) const;

        void print(std::ostream& stream) const;

        friend std::ostream& operator<< (std::ostream& stream, const LHEParticle& particle);

    private:

        LHEParticle* mothers_[2];

        int idup_;
        int istup_;
        int mothup_[2];
        int icolup_[2];
        double pup_[5];
        double vtimup_;
        int spinup_;
};

}

#endif
