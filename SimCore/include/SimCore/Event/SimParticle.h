#ifndef SIMCORE_EVENT_SIMPARTICLE_H
#define SIMCORE_EVENT_SIMPARTICLE_H

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TObject.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>
#include <string>
#include <vector>

namespace ldmx {

/**
 * Class representing a simulated particle.
 *
 * The particles are created from the tracks produced by simulation.
 */
class SimParticle {
 public:
  /**
   * Enum for interesting process types.
   */
  enum ProcessType {
    unknown = 0,
    annihil = 1,
    compt = 2,
    conv = 3,
    electronNuclear = 4,
    eBrem = 5,
    eIoni = 6,
    msc = 7,
    phot = 8,
    photonNuclear = 9,
    GammaToMuPair = 10,
    eDarkBrem = 11,
    Decay = 12,
    Primary = 13,
    muonNuclear = 14,
    neutronInelastic = 15,
    neutronCapture = 16,
    kaonInelastic = 17,
    pionInelastic = 18,
    protonInelastic = 19,
    // Only add additional processes to the end of this list!
  };

  /// Typedef for process map.
  typedef std::map<std::string, ProcessType> ProcessTypeMap;

  /// Constructor
  SimParticle() = default;

  /// Destructor
  virtual ~SimParticle() = default;

  /// Reset an instance of this class by clearing all of its data.
  void clear();

  /// Print a string representation of this object.
  friend std::ostream& operator<<(std::ostream& o, const SimParticle& d);

  /**
   * Get the energy of this particle [MeV].
   *
   * @return The energy of this particle.
   */
  double getEnergy() const { return energy_; }

  /**
   * Get the kinetic energy of this particle [MeV].
   *
   * @return The kinetic energy of this particle.
   */
  double getKineticEnergy() const { return energy_ - mass_; }
  /**
   * Get the PDG ID of this particle.
   *
   * @return The PDG ID of this particle.
   */
  int getPdgID() const { return pdg_id_; }

  /**
   * Get the generator status of this particle.  A non-zero status
   * indicates that this particle originates from an external
   * generator (e.g. LHE).
   *
   * @return The generator status.
   */
  int getGenStatus() const { return gen_status_; }

  /**
   * Get the global time of this particle's creation [ns].
   *
   * @return The global time of this particle's creation.
   */
  double getTime() const { return time_; }

  /**
   * Get a vector containing the vertex of this particle in mm.
   *
   * In this case, the vertex refers to the position where the
   * particle is first created in the simulation.   For a particle
   * with generator status equal to 1, this will equal the position
   * from which this particle is fired from.
   *
   * @return The vertex of this particle.
   */
  std::vector<double> getVertex() const { return {vtx_x_, vtx_y_, vtx_z_}; }

  /**
   * Get the volume name in which this particle was created in.
   *
   * The volumes names are set in the GDML detector description.
   *
   * @return The volume name in which this particle was created in.
   */
  std::string getVertexVolume() const { return vertex_volume_; }

  /**
   * Get the material name in which this particle was created in.
   *
   * The material names are set in the GDML detector description.
   *
   * @return The material name in which this particle was created in.
   */
  std::string getInteractionMaterial() const { return interaction_material_; }

  /**
   * Get the end_point of this particle where it was destroyed
   * or left the world volume [mm].
   *
   * @return The end_point of this particle
   */
  std::vector<double> getEndPoint() const { return {end_x_, end_y_, end_z_}; }

  /**
   * Get a vector containing the momentum of this particle [MeV].
   *
   * The momentum of this particle is set at the time of its creation.
   *
   * @return The momentum of this particle.
   */
  std::vector<double> getMomentum() const { return {px_, py_, pz_}; }

  /**
   * Get the mass of this particle [GeV].
   *
   * @return The mass of this particle in GeV.
   */
  double getMass() const { return mass_; }

  /**
   * Get the charge of this particle.
   *
   * @return The charge of this particle.
   */
  double getCharge() const { return charge_; }

  /**
   * Get a vector containing the track IDs of all daughter particles.
   *
   * @return A vector containing the track IDs of all daughter
   *      particles.
   */
  std::vector<int> getDaughters() const { return daughters_; }

  /**
   * Get a vector containing the track IDs of the parent particles.
   *
   * @return A vector containing the track IDs the parent particles.
   */
  std::vector<int> getParents() const { return parents_; }

  /**
   * Set the energy of this particle [MeV].
   *
   * @param[in] energy the energy of this particle.
   */
  void setEnergy(const double& energy) { energy_ = energy; }

  /**
   * Set the PDG ID of this particle.
   *
   * @param[in] pdg_id the PDG ID of the hit.
   */
  void setPdgID(const int& pdg_id) { pdg_id_ = pdg_id; }

  /**
   * Set the generator status of this particle.
   *
   * @param[in] gen_status the generator status of the hit.
   */
  void setGenStatus(const int& gen_status) { gen_status_ = gen_status; }

  /**
   * Set the global time of this particle's creation [ns].
   *
   * @param[in] time The global time of this particle's creation.
   */
  void setTime(const double& time) { time_ = time; }

  /**
   * Set the vertex of this particle [mm].
   *
   * @param[in] x The vertex x position.
   * @param[in] y The vertex y position.
   * @param[in] z The vertex z position.
   */
  void setVertex(const double& vtx_x, const double& vtx_y,
                 const double& vtx_z) {
    vtx_x_ = vtx_x;
    vtx_y_ = vtx_y;
    vtx_z_ = vtx_z;
  }

  /**
   * Set the name of the volume that this particle was created in.
   *
   * @param[in] vertexVolume volume name that this particle was
   *      created in.
   */
  void setVertexVolume(const std::string& vertexVolume) {
    vertex_volume_ = vertexVolume;
  }

  /**
   * Set the name of the material that this particle was created in.
   *
   * @param[in] interaction_material volume name that this particle was
   *      created in.
   */
  void setInteractionMaterial(const std::string& interaction_material) {
    interaction_material_ = interaction_material;
  }

  /**
   * Set the end point position of this particle [mm].
   *
   * @param[in] end_x The x position of the end point.
   * @param[in] end_y The y position of the end point.
   * @param[in] end_z The z position of the end point.
   */
  void setEndPoint(const double& end_x, const double& end_y,
                   const double& end_z) {
    end_x_ = end_x;
    end_y_ = end_y;
    end_z_ = end_z;
  }

  /**
   * Set the momentum of this particle [MeV].
   *
   * @param[in] px The momentum x-component.
   * @param[in] py The momentum y-component.
   * @param[in] pz The momentum z-component.
   */
  void setMomentum(const double& px, const double& py, const double& pz) {
    px_ = px;
    py_ = py;
    pz_ = pz;
  }

  /**
   * Set the mass of this particle [GeV].
   *
   * @param[in] mass The mass of this particle.
   */
  void setMass(const double& mass) { mass_ = mass; }

  /**
   * Set the charge of this particle.
   *
   * @param[in] charge The charge of this particle.
   */
  void setCharge(const double& charge) { charge_ = charge; }

  /**
   * Add a reference to a daughter particle by its track ID.
   *
   * This adds the track ID of the daughter particle to the vector of
   * daughter particle IDs.
   *
   * @param[in] daughter_track_id The daughter particle track ID.
   */
  void addDaughter(const int& daughter_track_id) {
    daughters_.push_back(daughter_track_id);
  }

  /**
   * Add a reference to a parent particle by its track ID.
   *
   * @param[in] parent_track_id The track ID of the parent particle.
   */
  void addParent(const int& parent_track_id) {
    parents_.push_back(parent_track_id);
  }

  /**
   * Get the creator process type of this particle.
   *
   * @return The creator process type of this particle.
   */
  int getProcessType() const { return process_type_; }

  /**
   * Set the creator process type of this particle.
   *
   * @param[in] process_type the creator process type of this particle.
   */
  void setProcessType(const int& process_type) { process_type_ = process_type; }

  /**
   * Set the momentum at this particle's end point.
   *
   * @param[in] end_px The x component of the momentum.
   * @param[in] end_py The y component of the momentum.
   * @param[in] end_pz The z component of the momentum.
   */
  void setEndPointMomentum(const double& end_px, const double& end_py,
                           const double& end_pz) {
    end_px_ = end_px;
    end_py_ = end_py;
    end_pz_ = end_pz;
  }

  /**
   * Get the momentum at this particle's end point.
   *
   * @return The momentum at this particle's end point as a vector.
   */
  std::vector<double> getEndPointMomentum() const {
    return {end_px_, end_py_, end_pz_};
  }

  /**
   * Get the process type enum from a G4VProcess name.
   *
   * @return The process type from the string.
   */
  static ProcessType findProcessType(std::string processName);

 private:
  static ProcessTypeMap createProcessTypeMap();

 private:
  /// The energy of this particle.
  double energy_{0};

  /// The PDG ID of this particle.
  int pdg_id_{0};

  /// The generator status.
  int gen_status_{-1};

  /// The global creation time.
  double time_{0};

  /// The x component of the vertex.
  double vtx_x_{0};

  /// The y component of the vertex.
  double vtx_y_{0};

  /// The z component of the vertex.
  double vtx_z_{0};

  /// The x component of the end point.
  double end_x_{0};

  /// The y component of the end point.
  double end_y_{0};

  /// The z component of the end point.
  double end_z_{0};

  /// The x component of the momentum.
  double px_{0};

  /// The y component of the momentum.
  double py_{0};

  /// The z component of the momentum.
  double pz_{0};

  /// The x component of the end_point momentum.
  double end_px_{0};

  /// The y component of the end_point momentum.
  double end_py_{0};

  /// The z component of the end_point momentum.
  double end_pz_{0};

  /// The particle's mass.
  double mass_{0};

  /// The particle's charge.
  double charge_{0};

  /// The list of daughter particle track IDs.
  std::vector<int> daughters_;

  /// The list of parent particles track IDs.
  std::vector<int> parents_;

  /// Encoding of Geant4 process type.
  int process_type_{-1};

  /// Volume the track was created in.
  std::string vertex_volume_{""};

  /// Volume the track was created in.
  std::string interaction_material_{""};

  /// Map containing the process types.
  static ProcessTypeMap process_map;

  ClassDef(SimParticle, 8);

};  // SimParticle
}  // namespace ldmx

#endif  // SIMCORE_EVENT_SIMPARTICLE_H
