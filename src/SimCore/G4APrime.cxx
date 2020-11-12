/**
 * @file G4APrime.cxx
 * @brief Class creating the A' particle in Geant.
 * @author Michael Revering, University of Minnesota
 */

#include "SimCore/G4APrime.h"

#include "Framework/Exception/Exception.h"

#include "globals.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"

G4APrime* G4APrime::theAPrime = 0;

G4APrime::G4APrime(
    const G4String&     aName, 
    G4double            mass, 
    G4double            width,       
    G4double            charge,
    G4int               iSpin, 
    G4int               iParity,   
    G4int               iConjugation, 
    G4int               iIsospin, 
    G4int               iIsospin3,
    G4int               gParity,
    const G4String&     pType,
    G4int               lepton,
    G4int               baryon,
    G4int               encoding,
    G4bool              stable, 
    G4double            lifetime,
    G4DecayTable        *decaytable) 
    : G4ParticleDefinition( aName, mass, width, charge, iSpin, iParity, 
                           iConjugation, iIsospin, iIsospin3, gParity, pType,    
                           lepton, baryon, encoding, stable, lifetime, decaytable ) { /* Nothing on purpose */ }

G4APrime* G4APrime::APrime(G4double theMass) {

    if(!theAPrime) {

        if ( theMass < 0 )
            EXCEPTION_RAISE("APMass","APrime doesn't have a mass set!");
     
        const G4String&     name = "A^1";
        G4double            mass = theMass;
        G4double            width = 0.;       
        G4double            charge = 0;
        G4int               iSpin = 0;
        G4int               iParity = 0; 
        G4int               iConjugation = 0; 
        G4int               iIsospin = 0;
        G4int               iIsospin3 = 0;
        G4int               gParity = 0;
        const G4String&     pType = "APrime";
        G4int               lepton = 0;
        G4int               baryon = 0;
        G4int               encoding = 622; //PDG ID
        G4bool              stable = true; //stable - no decay
        G4double            lifetime = -1; //stable - no decay
        G4DecayTable*       decaytable = 0;

        theAPrime = new G4APrime(
                name, mass, width, charge, iSpin, iParity,
                iConjugation, iIsospin, iIsospin3, gParity, pType,
                lepton, baryon, encoding, stable, lifetime, decaytable
                );
    }

    return theAPrime;
}


