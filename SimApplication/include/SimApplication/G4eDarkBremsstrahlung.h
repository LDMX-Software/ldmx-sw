/**
 * @file G4eDarkBremsstrahlung.h
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMAPPLICATION_G4EDARKBREMSSTRAHLUNG_H_
#define SIMAPPLICATION_G4EDARKBREMSSTRAHLUNG_H_

#include "SimApplication/G4eDarkBremsstrahlungModel.h"

// Geant
#include "G4VEnergyLossProcess.hh"

class G4String;
class G4ParticleDefinition;

/**
 * @class G4eDarkBremsstrahlung
 *
 * Class that represents the dark brem process.
 * A electron or positron is allowed to brem a dark photon
 */
class G4eDarkBremsstrahlung : public G4VEnergyLossProcess {

    public:
  
        /**
         * Constructor
         *
         * Sets this process up
         */
        G4eDarkBremsstrahlung(const G4String& name = "eDBrem");
  
        /**
         * Destructor
         */
        virtual ~G4eDarkBremsstrahlung() { /*Nothing on purpose*/ }
  
        /** 
         * Checks if the passed particle should be able to do this process
         *
         * @return true if particle is electron
         */
        virtual G4bool IsApplicable(const G4ParticleDefinition& p);
  
        /**
         * Reports the file name and the method (in string form)
         */
        virtual void PrintInfo();
  
        /** Pass the method for this process to the model */
        void SetMethod(G4eDarkBremsstrahlungModel::DarkBremMethod method);

        /** Pass LHE file of dark brem events to the model */
        void SetMadGraphDataFile(std::string file);
 
    protected:
  
        /** Setup this process to get ready for simulation */
        virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                                 const G4ParticleDefinition*);
  
        /** Has this process been setup yet? */
        G4bool isInitialised;
 
    private:
  
        /** remove ability to assign this object */
        G4eDarkBremsstrahlung & operator=(const G4eDarkBremsstrahlung &right);

        /** remove ability to copy construct */
        G4eDarkBremsstrahlung(const G4eDarkBremsstrahlung&);

        /** Method that was passed to the model */
        G4eDarkBremsstrahlungModel::DarkBremMethod method_{G4eDarkBremsstrahlungModel::DarkBremMethod::Undefined};

        /** Mad Graph file passed to model */
        std::string madGraphFile_;
  
};


#endif // SIMAPPLICATION_G4EDARKBREMSSTRAHLUNG_H_
