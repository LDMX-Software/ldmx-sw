/**
 * @file G4eDarkBremsstrahlung.h
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 */

#ifndef G4eDarkBremsstrahlung_h
#define G4eDarkBremsstrahlung_h

// Geant
#include "G4VEnergyLossProcess.hh"
#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4SystemOfUnits.hh"
#include "G4APrime.h"
#include "G4eDarkBremsstrahlungModel.h"
#include "G4UnitsTable.hh"
#include "G4ProductionCutsTable.hh"
#include "G4MaterialCutsCouple.hh"


class G4Material;

class G4eDarkBremsstrahlung : public G4VEnergyLossProcess
{

   public:

      G4eDarkBremsstrahlung(const G4String& name = "eDBrem");

      virtual ~G4eDarkBremsstrahlung();

      virtual G4bool IsApplicable(const G4ParticleDefinition& p);

      virtual void PrintInfo();

      void SetMethod(std::string method_in);

   protected:

      virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                               const G4ParticleDefinition*);
      G4bool isInitialised;

   private:

      G4eDarkBremsstrahlung & operator=(const G4eDarkBremsstrahlung &right);
      G4eDarkBremsstrahlung(const G4eDarkBremsstrahlung&);

};


#endif
