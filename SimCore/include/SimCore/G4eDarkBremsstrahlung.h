/**
 * @file G4eDarkBremsstrahlung.h
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 */

#ifndef SIMCORE_G4EDARKBREMSSTRAHLUNG_H_
#define SIMCORE_G4EDARKBREMSSTRAHLUNG_H_

// Geant
#include "G4VEnergyLossProcess.hh"

class G4String;
class G4ParticleDefinition;
class G4eDarkBremsstrahlungModel;

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
         *
         * Deletes the model if it has been created
         */
        virtual ~G4eDarkBremsstrahlung();
  
        /** 
         * Checks if the passed particle should be able to do this process
         *
         * @return true if particle is electron or positron
         */
        virtual G4bool IsApplicable(const G4ParticleDefinition& p);
  
        //TODO: write a helpful print function
        virtual void PrintInfo() { std::cout << "G4eDarkBremsstrahlung" << std::endl; }
  
        /** Pass the method for this process to the model */
        void SetMethod(std::string method_in);
 
    protected:
  
        /** Setup this process to get ready for simulation */
        virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                                 const G4ParticleDefinition*);
  
        /** Has this process been setup yet? */
        G4bool isInitialised;
 
    private:
  
        /** remove ability to copy construct or assign this object */
        G4eDarkBremsstrahlung & operator=(const G4eDarkBremsstrahlung &right);
        G4eDarkBremsstrahlung(const G4eDarkBremsstrahlung&);
  
        /** Instance of model for this process (does the heavy lifting) */
        G4eDarkBremsstrahlungModel *theModel_{nullptr};

};


#endif // SIMCORE_G4EDARKBREMSSTRAHLUNG_H_
