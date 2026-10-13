#ifndef SIMCORE_EVENT_VERTEXINFO_H
#define SIMCORE_EVENT_VERTEXINFO_H

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include <Rtypes.h>

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>
#include <vector>

namespace ldmx {

/**
 * @class VertexRecord
 * @brief Stores information about a single interaction vertex
 *
 * For each interaction of a tracked particle, this class records:
 * - The incoming particle kinematics (pre-step point of the interacting step)
 * - The kinematics of all outgoing particles produced at the vertex
 * - The Geant4 process that created the vertex
 * - The volume in which the interaction occurred
 *
 * The class is intentionally generic: it can describe any vertex
 * (eBrem, photonuclear, pair conversion, dark brem, ...) since the
 * identity of the particles is carried by their PDG IDs.
 */
class VertexRecord {
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
    double energy_{0.};

    /// Momentum components [MeV]
    double px_{0.};
    double py_{0.};
    double pz_{0.};

    /// Position components [mm]
    double x_{0.};
    double y_{0.};
    double z_{0.};

    /// Global time [ns]
    double time_{0.};

    /// Default constructor
    ParticleInfo() = default;

    /// Virtual destructor (required by ClassDef)
    virtual ~ParticleInfo() = default;

    /// ROOT dictionary generation
    ClassDef(ParticleInfo, 1);
  };

  /// Constructor
  VertexRecord() = default;

  /// Destructor
  virtual ~VertexRecord() = default;

  /**
   * Set the incoming particle information
   *
   * @param p ParticleInfo for the particle entering the vertex
   */
  void setIncomingParticle(const ParticleInfo &p) { incoming_particle_ = p; }

  /**
   * Get the incoming particle information
   *
   * @return ParticleInfo for the particle entering the vertex
   */
  ParticleInfo getIncomingParticle() const { return incoming_particle_; }

  /**
   * Add an outgoing particle produced at this vertex
   *
   * @param p ParticleInfo for one of the vertex products
   */
  void addOutgoingParticle(const ParticleInfo &p) {
    outgoing_particles_.push_back(p);
  }

  /**
   * Get all outgoing particles produced at this vertex
   *
   * @return Vector of ParticleInfo for the vertex products
   */
  const std::vector<ParticleInfo> &getOutgoingParticles() const {
    return outgoing_particles_;
  }

  /**
   * Get the number of outgoing particles recorded at this vertex
   *
   * @return Number of vertex products
   */
  std::size_t getNumOutgoingParticles() const {
    return outgoing_particles_.size();
  }

  /**
   * Set the Geant4 process that created this vertex
   *
   * @param name Process name (e.g. "eBrem", "photonNuclear")
   */
  void setProcessName(const std::string &name) { process_ = name; }

  /**
   * Get the Geant4 process that created this vertex
   *
   * @return Process name
   */
  const std::string &getProcessName() const { return process_; }

  /**
   * Set the volume in which the interaction occurred
   *
   * @param v Volume name
   */
  void setVolumeName(const std::string &v) { volume_ = v; }

  /**
   * Get the volume in which the interaction occurred
   *
   * @return Volume name
   */
  const std::string &getVolumeName() const { return volume_; }

private:
  /// Incoming particle information
  ParticleInfo incoming_particle_;

  /// Outgoing particles produced at this vertex
  std::vector<ParticleInfo> outgoing_particles_;

  /// Geant4 process that created this vertex
  std::string process_{""};

  /// Volume in which the interaction occurred
  std::string volume_{""};

  /// ROOT dictionary generation
  ClassDef(VertexRecord, 1);

}; // VertexRecord

} // namespace ldmx

#endif // SIMCORE_EVENT_VERTEXINFO_H