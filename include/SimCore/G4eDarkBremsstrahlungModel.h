/**
 * @file G4eDarkBremsstrahlungModel.h
 * @brief Class provided to simulate the dark brem cross section and
 * interaction.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_G4EDARKBREMSSTRAHLUNGMODEL_H_
#define SIMCORE_G4EDARKBREMSSTRAHLUNGMODEL_H_

// Geant
#include "G4VEmModel.hh"

// ROOT
#include "TLorentzVector.h"

/**
 * @class G4eDarkBremsstrahlungModel
 *
 * Geant4 implementation of the model for a particle undergoing a dark brem.
 *
 * This is where all the heavy lifting in terms of calculating cross sections
 * and actually having an electron do a dark brem occurs.
 */
class G4eDarkBremsstrahlungModel : public G4VEmModel {
 public:
  /**
   * @enum DarkBremMethod
   *
   * Possible methods to dark brem inside of this model.
   */
  enum DarkBremMethod {
    ForwardOnly = 1,  /// Use actual electron energy and get pT from LHE (such
                      /// that pT^2+me^2 < Eacc^2)
    CMScaling,        /// Boost LHE vertex momenta to the actual electron energy
    Undefined         /// Use LHE vertex as is
  };

  /**
   * Set the particle definition that this model will represent.
   */
  G4eDarkBremsstrahlungModel(const G4ParticleDefinition* p = 0,
                             const G4String& theName = "eDBrem");

  /**
   * Destructor
   *
   * Attempt to cleanup hanging pointers and leftover memory.
   */
  virtual ~G4eDarkBremsstrahlungModel();

  /**
   * Set the cuts using the particle definition and comput the partial sum
   * sigma.
   */
  virtual void Initialise(const G4ParticleDefinition* p,
                          const G4DataVector& cuts);

  /**
   * Simulates the emission of a dark photon + electron.
   *
   * Gets an energy fraction and Pt from madgraph files.
   * Scales the energy so that the fraction of kinectic energy is constant,
   * keeps the Pt constant. If the Pt is larger than the new energy, that event
   * is skipped, and a new one is taken from the file.
   *
   * Deactivates the dark brem, ensuring only one dark brem per step and per
   * event. Needs to be reactived in the end of event action.
   *
   * @param secondaries vector of primary particle's offspring
   * @param primary particle that could go dark brem
   * @param tmin minimum energy possible for sampling
   * @param maxEnergy maximum energy possible for sampling
   */
  virtual void SampleSecondaries(std::vector<G4DynamicParticle*>* secondaries,
                                 const G4MaterialCutsCouple*,
                                 const G4DynamicParticle* primary,
                                 G4double tmin, G4double maxEnergy);
  /**
   * Set the method for the dark brem simulation
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
   * Uses weights for materials calculated before and stored in partialSumSigma
   *
   * @param couple G4 package containing material and other relevant information
   * @return the chosen G4Element
   */
  const G4Element* SelectRandomAtom(const G4MaterialCutsCouple* couple);

 private:
  /**
   * Helpful typedef for boost integration.
   */
  typedef std::vector<double> StateType;

  /**
   * @struct Chi
   *
   * Stores parameters for chi function used in integration.
   * Implements function as member operator compatible with
   * boost::numeric::odeint
   *
   * \f[ \chi(t) = \left(
   * \frac{Z^2a^4t^2}{(1+a^2t)^2(1+t/d)^2}+\frac{Za_p^4t^2}{(1+a_p^2t)^2(1+t/0.71)^8}\left(\frac{1+t(m_{up}^2-1)}{4m_p^2}\right)^2\right)\frac{t-m_A^4/(4E_0^2)}{t^2}
   * \f]
   *
   * where
   * \f$m_A\f$ = mass of A' in GeV
   * \f$m_e\f$ = mass of electron in GeV
   * \f$E_0\f$ = incoming energy of electron in GeV
   * \f$A\f$ = atomic number of target atom
   * \f$Z\f$ = atomic mass of target atom
   * \f[a = \frac{111.0}{m_e Z^{1/3}}\f]
   * \f[a_p = \frac{773.0}{m_e Z^{2/3}}\f]
   * \f[d = \frac{0.164}{A^{2/3}}\f]
   * \f$m_{up}\f$ = mass of up quark
   * \f$m_{p}\f$ = mass of proton
   */
  struct Chi {
    double A;    /// atomic number
    double Z;    /// atomic mass
    double E0;   /// incoming beam energy [GeV]
    double MA;   /// A' mass [GeV]
    double Mel;  /// electron mass [GeV]

    /**
     * Access function in style required by boost::numeric::odeint
     *
     * Calculates dxdt from t and other paramters.
     */
    void operator()(const StateType&, StateType& dxdt, double t);
  };

  /**
   * @struct DiffCross
   *
   * Implementation of the differential scattering cross section.
   * Stores parameters.
   *
   * Implements function as member operator compatible with
   * boost::numeric::odeint
   *
   * \f[ \frac{d\sigma}{dx}(x) =
   * \sqrt{1-\frac{m_A^2}{E_0^2}}\frac{1-x+x^2/3}{m_A^2(1-x)/x+m_e^2x} \f]
   *
   * where
   * \f$m_A\f$ = mass of A' in GeV
   * \f$m_e\f$ = mass of electron in GeV
   * \f$E_0\f$ = incoming energy of electron in GeV
   */
  struct DiffCross {
    double E0;   /// incoming beam energy [GeV]
    double MA;   /// A' mass [GeV]
    double Mel;  /// electron mass [GeV]

    /**
     * Access function in style required by boost::numeric::odeint
     *
     * Calculates DsigmaDx from x and other paramters.
     */
    void operator()(const StateType&, StateType& DsigmaDx, double x);
  };

  /**
   * @struct OutgoingKinematics
   *
   * Data frame to store mad graph data read in from LHE files.
   */
  struct OutgoingKinematics {
    TLorentzVector electron;  /// 4-momentum of electron in center of momentum
                              /// frame for electron-A' system
    TLorentzVector
        centerMomentum;  /// 4-vector pointing to center of momentum frame
    G4double E;  /// energy of electron before brem (used as key in mad graph
                 /// data map)
  };

  /**
   * Sets the particle being looked at and checks whether it is an electron or
   * not.
   */
  void SetParticle(const G4ParticleDefinition* p);

  /*
   * Parse an LHE File
   *
   * Parses an LHE file to extract the kinetic energy fraction and pt of the
   * outgoing electron in each event. Loads the two numbers from every event
   * into a map of vectors of pairs (mgdata). Map is keyed by energy, vector
   * pairs are energy fraction + pt. Also creates an list of energies and
   * placeholders (energies), so that different energies can be looped
   * separately.
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
   * Scales from the closest imported beam energy above the given value (scales
   * down to avoid biasing issues).
   *
   * @param E0 energy of particle undergoing dark brem [GeV]
   * @return total energy and transverse momentum of particle [GeV]
   */
  G4eDarkBremsstrahlungModel::OutgoingKinematics GetMadgraphData(double E0);

  /**
   * Calculates the cross section per atom in GEANT4 internal units.
   * Uses WW approximation to find the total cross section, performing numerical
   * integrals over x and theta.
   *
   * Non named parameters are not used.
   *
   * Numerical integrals are done using boost::numeric::odeint.
   *
   * Integrate Chi from \f$m_A^4/(4E_0^2)\f$ to \f$m_A^2\f$
   *
   * Integrate diffcross from 0 to \f$min(1-m_e/E_0,1-m_A/E_0)\f$
   *
   * Total cross section is given by
   * \f[ \sigma = 4 \frac{pb}{GeV}\alpha_{EW}^3 \int \chi(t)dt \int
   * \frac{d\sigma}{dx}(x)dx \f]
   *
   * @param E0 energy of beam (incoming particle)
   * @param Z atomic number of atom
   * @param A atomic mass of atom
   * @param cut minimum energy cut to calculate cross section
   * @return cross section (0. if outside energy cuts)
   */
  virtual G4double ComputeCrossSectionPerAtom(const G4ParticleDefinition*,
                                              G4double E0, G4double Z,
                                              G4double A, G4double cut,
                                              G4double);

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

  /** Hide assignment operator */
  G4eDarkBremsstrahlungModel& operator=(
      const G4eDarkBremsstrahlungModel& right);

  /** Hide copy constructor */
  G4eDarkBremsstrahlungModel(const G4eDarkBremsstrahlungModel&);

 protected:
  /** definition of particle */
  const G4ParticleDefinition* particle_;

  /** loss in particle energy */
  G4ParticleChangeForLoss* fParticleChange_;

 private:
  /** maximum number of iterations to check before giving up on an event */
  unsigned int maxIterations_{10000};

  /** mass of the A Prime [GeV] */
  double MA_;

  /** mass of an electron [GeV] */
  double Mel_;

  /** method for this model */
  DarkBremMethod method_{DarkBremMethod::Undefined};

  /**
   * should we always create a totally new electron when we dark brem?
   *
   * TODO make this configurable? I (Tom E) can't think of a reason NOT to have
   * it...
   */
  bool alwaysCreateNewElectron_{true};

  /**
   * Storage of data from mad graph
   *
   * Maps incoming electron energy to various options for outgoing kinematics.
   */
  std::map<double, std::vector<OutgoingKinematics> > madGraphData_;

  /**
   * Stores a map of current access points to mad graph data.
   *
   * Maps incoming electron energy to the index of the data vector
   * that we will get the data from.
   *
   * Also sorts the incoming electron energy so that we can find
   * the sampling energy that is closest above the actual incoming energy.
   */
  std::map<double, unsigned int> currentDataPoints_;

  std::vector<G4DataVector*> partialSumSigma_;
};

#endif  // SIMCORE_G4EDARKBREMSSTRAHLUNGMODEL_H_
