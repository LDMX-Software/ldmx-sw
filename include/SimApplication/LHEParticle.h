#ifndef SimApplication_LHEParticle_h
#define SimApplication_LHEParticle_h

// STL
#include <string>
#include <vector>

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
