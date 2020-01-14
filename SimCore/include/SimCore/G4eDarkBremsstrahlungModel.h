/**
 * @file G4eDarkBremsstrahlungModel.h
 * @brief Class provided to simulate the dark brem cross section and interaction.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_G4EDARKBREMSSTRAHLUNGMODEL_H_
#define SIMCORE_G4EDARKBREMSSTRAHLUNGMODEL_H_

// Geant
#include "G4VEmModel.hh"

class G4ParticleDefinition;
class G4String;
class G4DataVector;
class G4Material;
class G4DynamicParticle;
class G4MaterialCutsCouple;
class G4Element;
class G4ParticleChangeForLoss;

// ROOT
class TLorentzVector;

/**
 * @class G4eDarkBremsstrahlungModel
 *
 * Geant4 implementation of the model for a particle undergoing a dark brem.
 */
class G4eDarkBremsstrahlungModel : public G4VEmModel {

    public:

        /**
         * @enum DarkBremMethod
         *
         * Possible methods to dark brem inside of this model.
         */
        enum DarkBremMethod{
            ForwardOnly,
            CMScaling,
            Undefined
        };

        /**
         * @struct ParamsForChi
         *
         * Stores parameters for chi function used in integration.
         */
        struct ParamsForChi {double AA; double ZZ; double MMA; double EE0;};

        /**
         * @struct frame
         *
         * Data frame to store mad graph data read in from LHE files.
         */
        struct frame {TLorentzVector* fEl; TLorentzVector* cm; G4double E;};

        /**
         * Set the particle definition that this model will represent.
         */
        G4eDarkBremsstrahlungModel(const G4ParticleDefinition* p = 0,
                                   const G4String& name = "eDBrem");

        /**
         * Destructor
         *
         * Attempt to cleanup hanging pointers and leftover memory.
         */
        virtual ~G4eDarkBremsstrahlungModel();

        /**
         * Set the cuts using the particle definition and comput the partial sum sigma.
         */
        virtual void Initialise(const G4ParticleDefinition*, const G4DataVector&);

        
        /**
         * Calculates the cross section per atom in GEANT4 internal units.
         * Uses WW approximation to find the total cross section, performing numerical integrals over x and theta.
         *
         * Non named parameters are not used.
         *
         * Numerical integrals are done using the GNU Scientific Library (gsl).
         * TODO Translate these integrals to boost, so we can remove gsl as a dependency.
         *
         * @param E0 energy of beam (incoming particle)
         * @param Z atomic number of atom
         * @param A atomic mass of atom
         * @param cut minimum energy cut to calculate cross section
         * @return cross section (0. if outside energy cuts)
         */
        virtual G4double ComputeCrossSectionPerAtom(const G4ParticleDefinition*,
                                                   G4double E0, 
                                                   G4double Z,   G4double A,
                                                   G4double cut, G4double);


        /** 
         * Build the table of cross section per element. 
         *
         * The table is built for MATERIALS. 
         * This table is used by DoIt to select randomly an element in the material.
         *
         * @param material Material to build cross section table for
         * @param kineticEnergy kinetic energy of incoming particle
         * @param cut minimum energy cut on cross section
         * @return G4DataVector of elements to cross sections for the input material. 
         */
        G4DataVector* ComputePartialSumSigma(const G4Material* material,
                                            G4double kineticEnergy, G4double cut);

        /**
         * Simulates the emission of a dark photon + electron.
         *
         * Gets an energy fraction and Pt from madgraph files. 
         * Scales the energy so that the fraction of kinectic energy is constant, keeps the Pt constant. 
         * If the Pt is larger than the new energy, that event is skipped, and a new one is taken from the file.
         *
         * Deactivates the dark brem, ensuring only one dark brem per step and per event.
         * Needs to be reactived in the end of event action.
         *
         * Gets madgraph files from 'Resources' directory in the current working directory.
         * TODO Implement a better way to pass madgraph data files
         * 
         * @param secondaries vector of primary particle's offspring
         * @param primary particle that could go dark brem
         * @param tmin minimum energy possible for sampling
         * @param maxEnergy maximum energy possible for sampling
         */
        virtual void SampleSecondaries(std::vector<G4DynamicParticle*>* secondaries,
                                      const G4MaterialCutsCouple* ,
                                      const G4DynamicParticle* primary,
                                      G4double tmin,
                                      G4double maxEnergy);
        /**
         * Set the method for the dark brem simulation
         * TODO write enum to make this safer
         */
        void SetMethod(DarkBremMethod method);

        /**
         * Set the data file for the dark brem events to be scaled.
         * @param file path to LHE file
         */
        void SetMadGraphDataFile(std::string file);

    protected:

        /**
         * Selects random element out of the material information given.
         *
         * Uses weights for materials calculated before.
         *
         * @param couple G4 package containing material and other relevant information
         * @return the chosen G4Element
         */
        const G4Element* SelectRandomAtom(const G4MaterialCutsCouple* couple);

    private:
        
        /*
         * Parse an LHE File
         *
         * Parses an LHE file to extract the kinetic energy fraction and pt of the outgoing electron in each event. 
         * Loads the two numbers from every event into a map of vectors of pairs (mgdata). 
         * Map is keyed by energy, vector pairs are energy fraction + pt. 
         * Also creates an list of energies and placeholders (energies), so that different energies can be looped separately.
         *
         * @param fname name of LHE file to parse
         */
        void ParseLHE(std::string fname);
      
        /**
         * Fill vector of energies with the same number of items as the madgraph data.
         */
        void MakePlaceholders();

        /**
         * Returns mad graph data given an energy [GeV].
         *
         * Gets the energy fraction and Pt from the imported LHE data. 
         * E0 should be in GeV, returns the total energy and Pt in GeV. 
         * Scales from the closest imported beam energy above the given value (scales down to avoid biasing issues).
         *
         * @param E0 energy of particle undergoing dark brem [GeV]
         * @return total energy and transverse momentum of particle [GeV]
         */
        G4eDarkBremsstrahlungModel::frame GetMadgraphData(double E0);

        /**
         * Sets the particle being looked at and checks whether it is an electron or not.
         */
        void SetParticle(const G4ParticleDefinition* p);
      
        static G4double chi(double t, void * pp);
 
        /**
         * Implementation of the differential scattering cross section that is compatible
         * with the integration done by the gsl library.
         */
        static G4double DsigmaDx(double x, void * pp);
        
        /** Hide assignment operator */
        G4eDarkBremsstrahlungModel & operator=(const  G4eDarkBremsstrahlungModel &right);

        /** Hide copy constructor */
        G4eDarkBremsstrahlungModel(const  G4eDarkBremsstrahlungModel&);

    protected:

        /** definition of particle */
        const G4ParticleDefinition*   particle;

        /** pointer to the definition of the A Prime */
        G4ParticleDefinition*         theAPrime;

        /** loss in particle energy */
        G4ParticleChangeForLoss*      fParticleChange;

        /** mass of the A Prime */
        G4double                      MA;

        /** is the particle an electron? */
        G4bool                        isElectron;

    private:

        /** is this model been initialised? */
        G4bool   isInitialised;

        /** method for this model */
        DarkBremMethod method_{DarkBremMethod::Undefined};

        /** Storage of data from mad graph */
        std::map< double , std::vector < frame > > madGraphData_;

        /**
         * Stores pairs of imported beam energies (sampled on madgraph data),
         * and the place holder that loops on the madGraphData_ vector. See source.
         */
        std::vector < std::pair < double, int > > energies;

        std::vector<G4DataVector*> partialSumSigma;
  
};


#endif // SIMCORE_G4EDARKBREMSSTRAHLUNGMODEL_H_
