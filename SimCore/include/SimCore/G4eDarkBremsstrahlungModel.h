/**
 * @file G4eDarkBremsstrahlungModel.h
 * @brief Class provided to simulate the dark brem cross section and interaction.
 * @author Michael Revering, University of Minnesota
 */

#ifndef G4eDarkBremsstrahlungModel_h
#define G4eDarkBremsstrahlungModel_h

// Geant
#include "G4VEmModel.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4APrime.h"
#include "Randomize.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4ElementVector.hh"
#include "G4ProductionCutsTable.hh"
#include "G4DataVector.hh"
#include "G4ParticleChangeForLoss.hh"
#include "G4ProcessTable.hh"

// gsl
#include <gsl/gsl_math.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_monte_plain.h>
#include <gsl/gsl_monte_miser.h>
#include <gsl/gsl_monte_vegas.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_rng.h>

// STL
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <dirent.h>

// ROOT
#include "TLorentzVector.h"

struct ParamsForChi {double AA; double ZZ; double MMA; double EE0;};
struct frame {TLorentzVector* fEl; TLorentzVector* cm; G4double E;};

class G4Element;
class G4ParticleChangeForLoss;

class G4eDarkBremsstrahlungModel : public G4VEmModel
{
   public:

      G4eDarkBremsstrahlungModel(const G4ParticleDefinition* p = 0,
                                 const G4String& nam = "eDBrem");

      virtual ~G4eDarkBremsstrahlungModel();

      virtual void Initialise(const G4ParticleDefinition*, const G4DataVector&);

      
     virtual G4double ComputeCrossSectionPerAtom(const G4ParticleDefinition*,
                                                 G4double tkin, 
                                                 G4double Z,   G4double,
                                                 G4double cut,
                                                 G4double maxE = DBL_MAX);


     G4DataVector* ComputePartialSumSigma(const G4Material* material,
                                          G4double tkin, G4double cut);

     virtual void SampleSecondaries(std::vector<G4DynamicParticle*>*,
                                    const G4MaterialCutsCouple*,
                                    const G4DynamicParticle*,
                                    G4double tmin,
                                    G4double maxEnergy);

     void ParseLHE(std::string fname);
     
     void MakePlaceholders();

     void SetMethod(std::string);

     frame GetMadgraphData(double E0);

   protected:

      const G4Element* SelectRandomAtom(const G4MaterialCutsCouple* couple);

   private:

      void SetParticle(const G4ParticleDefinition* p);
     
      static G4double chi(double t, void * pp);
 
      static G4double DsigmaDx(double x, void * pp);
      
      // hide assignment operator
      G4eDarkBremsstrahlungModel & operator=(const  G4eDarkBremsstrahlungModel &right);
      G4eDarkBremsstrahlungModel(const  G4eDarkBremsstrahlungModel&);

   protected:

      const G4ParticleDefinition*   particle;
      G4ParticleDefinition*         theAPrime;
      G4ParticleChangeForLoss*      fParticleChange;
      G4double                      MA;
      G4bool                        isElectron;

   private:

      G4double highKinEnergy;
      G4double lowKinEnergy;
      G4double probsup;
      G4bool   isInitialised;
      std::string method;
      G4bool lhe_loaded;
      std::map< double , std::vector < frame > > mgdata;
      std::vector < std::pair < double, int > > energies;
      std::vector<G4DataVector*> partialSumSigma;
  
	};


#endif







