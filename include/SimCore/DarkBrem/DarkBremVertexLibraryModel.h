#ifndef SIMCORE_DARKBREM_DARKBREMVERTEXLIBRARYMODLE_H_
#define SIMCORE_DARKBREM_DARKBREMVERTEXLIBRARYMODLE_H_

#include "Framework/Configure/Parameters.h"
#include "SimCore/DarkBrem/G4eDarkBremsstrahlung.h"

// ROOT
#include "TLorentzVector.h"

namespace simcore {
namespace darkbrem {

/**
 * @class DarkBremVertexLibraryModel
 *
 * Geant4 implementation of the model for a particle undergoing a dark brem
 * where we use an imported vertex library to decide the outgoing kinematics.
 *
 * This is where all the heavy lifting in terms of calculating cross sections
 * and actually having an electron do a dark brem occurs. This model depends
 * on severl configurable parameters.
 *
 * - library_path : the full path to the directory containing the LHE dark brem
 *   vertices that will be read in to make the vertex library
 * - epsilon : strength of the dark photon - photon mixing
 * - threshold : minimum energy in GeV for the electron to have a non-zero
 *   cross section for going dark brem
 * - method : scaling method to use to scale the dark brem vertices from
 *   the library to the actual electron energy when a dark brem occurs
 *
 * The required parameter is a vertex library generated in MadGraph
 * (library_path). The other parameters have helpful defaults set in the python
 * configuration class DarkBrem and are there for you to be able to tune this
 * model's behavior. An example library for each of the major mass points is
 * installed with SimCore and is compressed as stored in the data directory.
 */
class DarkBremVertexLibraryModel : public G4eDarkBremsstrahlungModel {
 public:
  /**
   * Constructor
   * Set the parameters for this model.
   *
   * The method name is converted to an enum through a hard-coded switch
   * statement.
   *
   * The threshold is set to the maximum of the passed value or twice
   * the A' mass (so that it kinematically makes sense).
   *
   * The library path is immediately passed to SetMadGraphDataLibrary.
   */
  DarkBremVertexLibraryModel(framework::config::Parameters& params);

  /**
   * Destructor
   */
  virtual ~DarkBremVertexLibraryModel() {}

  /**
   * Print the configuration of this model
   */
  virtual void PrintInfo() const;

  /**
   * Record the configuration of this model into the RunHeader
   */
  virtual void RecordConfig(ldmx::RunHeader& h) const;

  /**
   * Calculates the cross section per atom in GEANT4 internal units.
   * Uses WW approximation to find the total cross section, performing numerical
   * integrals over x and theta.
   *
   * Numerical integrals are done using boost::numeric::odeint.
   *
   * Integrate Chi from \f$m_A^4/(4E_0^2)\f$ to \f$m_A^2\f$
   *
   * Integrate DiffCross from 0 to \f$min(1-m_e/E_0,1-m_A/E_0)\f$
   *
   * Total cross section is given by
   * \f[ \sigma = 4 \frac{pb}{GeV} \epsilon^2 \alpha_{EW}^3 \int \chi(t)dt \int
   * \frac{d\sigma}{dx}(x)dx \f]
   *
   * @param E0 energy of beam (incoming particle)
   * @param Z atomic number of atom
   * @param A atomic mass of atom
   * @param cut minimum energy cut to calculate cross section
   * @return cross section (0. if outside energy cuts)
   */
  virtual G4double ComputeCrossSectionPerAtom(G4double electronKE,
                                              G4double atomicA,
                                              G4double atomicZ);

  /**
   * Simulates the emission of a dark photon + electron.
   *
   * Gets an energy fraction and Pt from madgraph files.
   * The scaling of this energy fraction and Pt to the actual electron
   * energy depends on the input method.
   *
   * ## Forward Only
   * Scales the energy so that the fraction of kinectic energy is constant,
   * keeps the Pt constant. If the Pt is larger than the new energy, that event
   * is skipped, and a new one is taken from the file. Choses the Pz of the
   * recoil electron to always be positive.
   *
   * ## CM Scaling
   * Scale MadGraph vertex to actual energy of electron using Lorentz boosts,
   * and then extract the momentum from that.
   *
   * ## Undefined
   * Don't scale the MadGraph vertex to the actual energy of the electron.
   *
   * @param[in,out] particleChange structure holding changes to make to particle
   * track
   * @param[in] track current track being processesed
   * @param[in] step current step of the track
   */
  virtual void GenerateChange(G4ParticleChange& particleChange,
                              const G4Track& track, const G4Step& step);

 private:
  /**
   * Set the library of dark brem events to be scaled.
   * @param file path to directory of LHE files
   */
  void SetMadGraphDataLibrary(std::string path);

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
   * \f$m_A\f$ = mass of A' in GeV,
   * \f$m_e\f$ = mass of electron in GeV,
   * \f$E_0\f$ = incoming energy of electron in GeV,
   * \f$A\f$ = atomic number of target atom,
   * \f$Z\f$ = atomic mass of target atom,
   * \f[a = \frac{111.0}{m_e Z^{1/3}}\f]
   * \f[a_p = \frac{773.0}{m_e Z^{2/3}}\f]
   * \f[d = \frac{0.164}{A^{2/3}}\f]
   * \f$m_{up}\f$ = mass of up quark, and
   * \f$m_{p}\f$ = mass of proton
   */
  struct Chi {
    /// atomic number
    double A;
    /// atomic mass
    double Z;
    /// incoming beam energy [GeV]
    double E0;
    /// A' mass [GeV]
    double MA;
    /// electron mass [GeV]
    double Mel;

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
    /// incoming beam energy [GeV]
    double E0;
    /// A' mass [GeV]
    double MA;
    /// electron mass [GeV]
    double Mel;

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
    /// 4-momentum of electron in center of momentum frame for electron-A'
    /// system
    TLorentzVector electron;
    /// 4-vector pointing to center of momentum frame
    TLorentzVector centerMomentum;
    /// energy of electron before brem (used as key in mad graph data map)
    G4double E;
  };

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
   * Fill vector of currentDataPoints_ with the same number of items as the
   * madgraph data.
   *
   * Randomly choose a starting point so that the simulation run isn't dependent
   * on the order of LHE vertices in the library.
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
  OutgoingKinematics GetMadgraphData(double E0);

 private:
  /**
   * maximum number of iterations to check before giving up on an event
   *
   * @TODO make configurable and/or optimize somehow
   */
  unsigned int maxIterations_{10000};

  /** Threshold for non-zero xsec [GeV]
   *
   * Configurable with 'threshold'
   */
  double threshold_;

  /** Epsilon value to plug into xsec calculation
   *
   * @sa ComputeCrossSectionPerAtom for how this is used
   *
   * Configurable with 'epsilon'
   */
  double epsilon_;

  /**
   * @enum DarkBremMethod
   *
   * Possible methods to use the dark brem vertices from the imported library
   * inside of this model.
   */
  enum DarkBremMethod {
    /// Use actual electron energy and get pT from LHE (such that pT^2+me^2 <
    /// Eacc^2)
    ForwardOnly = 1,
    /// Boost LHE vertex momenta to the actual electron energy
    CMScaling = 2,
    /// Use LHE vertex as is
    Undefined = 3
  };

  /** method for this model
   *
   * Configurable with 'method'
   */
  DarkBremMethod method_{DarkBremMethod::Undefined};

  /**
   * Name of method for persisting into the RunHeader
   */
  std::string method_name_;

  /**
   * Full path to the vertex library used for persisting into the RunHeader
   */
  std::string library_path_;

  /**
   * should we always create a totally new electron when we dark brem?
   *
   * @TODO make this configurable? I (Tom E) can't think of a reason NOT to have
   * it... The alternative is to allow Geant4 to decide when to make a new
   * particle by checking if the resulting kinetic energy is below some
   * threshold.
   */
  bool alwaysCreateNewElectron_{true};

  /**
   * Storage of data from mad graph
   *
   * Maps incoming electron energy to various options for outgoing kinematics.
   * This is a hefty map and is what stores **all** of the vertices
   * imported from the LHE library of dark brem vertices.
   *
   * Library is read in from configuration parameter 'darkbrem.madgraphlibrary'
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
};

}  // namespace darkbrem
}  // namespace simcore

#endif  // SIMCORE_DARKBREM_DARKBREMVERTEXLIBRARYMODLE_H_
