#ifndef SIMCORE_EVENT_PHOTONUCLEARINTERACTION_H
#define SIMCORE_EVENT_PHOTONUCLEARINTERACTION_H

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
 * @class PhotonuclearInteraction
 * @brief Stores detailed information about a photonuclear interaction
 *
 * This class records comprehensive details about photonuclear (PN) interactions
 * to enable validation of cascade mechanisms, identification of edge cases,
 * and analysis of final state particle production.
 *
 * For each PN interaction, it captures:
 * - Incident photon information (energy, momentum, position, time)
 * - Target nucleus (Z, A, material)
 * - All immediate secondary particles from the cascade
 * - Mapping from cascade products to their final state descendants
 */
class PhotonuclearInteraction {
 public:
  /**
   * @struct ParticleInfo
   * @brief Stores kinematic and identity information for a particle
   */
  struct ParticleInfo {
    /// Track ID from Geant4
    int track_id_{0};

    /// PDG particle ID
    int pdg_id_{0};

    /// Total energy [MeV]
    double energy_{0.0};

    /// Momentum components [MeV]
    double px_{0.0};
    double py_{0.0};
    double pz_{0.0};

    /// Position components [mm]
    double x_{0.0};
    double y_{0.0};
    double z_{0.0};

    /// Global time [ns]
    double time_{0.0};

    /// Default constructor
    ParticleInfo() = default;

    /// Virtual destructor (required by ClassDef)
    virtual ~ParticleInfo() = default;

    /// ROOT dictionary generation
    ClassDef(ParticleInfo, 1);
  };

  /// Constructor
  PhotonuclearInteraction() = default;

  /// Destructor
  virtual ~PhotonuclearInteraction() = default;

  /// Clear the interaction data
  void clear();

  /**
   * Set the incident photon information
   *
   * @param track_id Geant4 track ID
   * @param energy Total energy [MeV]
   * @param px Momentum x-component [MeV]
   * @param py Momentum y-component [MeV]
   * @param pz Momentum z-component [MeV]
   * @param x Position x-component [mm]
   * @param y Position y-component [mm]
   * @param z Position z-component [mm]
   * @param time Global time [ns]
   */
  void setIncidentPhoton(int track_id, int pdg_id, double energy, double px,
                         double py, double pz, double x, double y, double z,
                         double time);

  /**
   * Get the incident photon information
   *
   * @return ParticleInfo for the incident photon
   */
  ParticleInfo getIncidentPhoton() const { return incident_photon_; }

  /**
   * Set the target nucleus information
   *
   * @param Z Atomic number
   * @param A Mass number
   * @param material Material name
   */
  void setTarget(int Z, int A, const std::string& material);

  /**
   * Get the target atomic number
   *
   * @return Atomic number Z
   */
  int getTargetZ() const { return target_z_; }

  /**
   * Get the target mass number
   *
   * @return Mass number A
   */
  int getTargetA() const { return target_a_; }

  /**
   * Get the target material name
   *
   * @return Material name
   */
  std::string getTargetMaterial() const { return target_material_; }

  /**
   * Add an immediate secondary particle from the PN interaction
   *
   * @param particle ParticleInfo for the secondary
   */
  void addImmediateSecondary(const ParticleInfo& particle);

  /**
   * Get all immediate secondary particles
   *
   * @return Vector of ParticleInfo for all immediate secondaries
   */
  std::vector<ParticleInfo> getImmediateSecondaries() const {
    return immediate_secondaries_;
  }

  /**
   * Get the number of immediate secondary particles
   *
   * @return Number of immediate secondaries
   */
  int getNumImmediateSecondaries() const {
    return immediate_secondaries_.size();
  }

  /**
   * Add a final state descendant track for an immediate secondary
   *
   * This builds the map from immediate cascade products to their
   * eventual final state particles.
   *
   * @param immediate_secondary_id Track ID of the immediate secondary
   * @param final_state_track_id Track ID of the final state descendant
   */
  void addDescendant(int immediate_secondary_id, int final_state_track_id);

  /**
   * Get the descendant map
   *
   * Maps immediate secondary track ID -> list of final state track IDs
   *
   * @return Map of secondary IDs to their descendant IDs
   */
  std::map<int, std::vector<int>> getDescendantMap() const {
    return descendant_map_;
  }

  /**
   * Get descendants for a specific immediate secondary
   *
   * @param immediate_secondary_id Track ID of the immediate secondary
   * @return Vector of track IDs for descendants (empty if none)
   */
  std::vector<int> getDescendants(int immediate_secondary_id) const;

  /**
   * Set the volume where the interaction occurred
   *
   * @param volume Volume name
   */
  void setInteractionVolume(const std::string& volume) {
    interaction_volume_ = volume;
  }

  /**
   * Get the interaction volume name
   *
   * @return Volume name
   */
  std::string getInteractionVolume() const { return interaction_volume_; }

  /**
   * Set the process name
   *
   * @param process Process name (e.g., "photonNuclear")
   */
  void setProcessName(const std::string& process) { process_name_ = process; }

  /**
   * Get the process name
   *
   * @return Process name
   */
  std::string getProcessName() const { return process_name_; }

  /**
   * Print information about this interaction
   */
  void print() const;

  /**
   * Stream operator for printing
   */
  friend std::ostream& operator<<(std::ostream& o,
                                  const PhotonuclearInteraction& pn);

 private:
  /// Incident photon information
  ParticleInfo incident_photon_;

  /// Target atomic number
  int target_z_{0};

  /// Target mass number
  int target_a_{0};

  /// Target material name
  std::string target_material_{""};

  /// Immediate secondary particles from the cascade
  std::vector<ParticleInfo> immediate_secondaries_;

  /// Maps immediate secondary track ID -> final state track IDs
  std::map<int, std::vector<int>> descendant_map_;

  /// Volume where interaction occurred
  std::string interaction_volume_{""};

  /// Process name (e.g., "photonNuclear")
  std::string process_name_{""};

  /// ROOT dictionary generation
  ClassDef(PhotonuclearInteraction, 1);

};  // PhotonuclearInteraction

}  // namespace ldmx

#endif  // SIMCORE_EVENT_PHOTONUCLEARINTERACTION_H
