/**
 * @file SimTrackerHit.h
 * @brief Class which encapsulates information from a hit in a simulated
 * tracking detector
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_EVENT_SIMTRACKERHIT_H_
#define SIMCORE_EVENT_SIMTRACKERHIT_H_

// ROOT
#include "TObject.h"  //For ClassDef

// STL
#include <iostream>

namespace ldmx {

/**
 * @class SimTrackerHit
 * @brief Represents a simulated tracker hit in the simulation
 */
class SimTrackerHit {
 public:
  /**
   * Class constructor.
   */
  SimTrackerHit() = default;

  /**
   * Class destructor.
   */
  virtual ~SimTrackerHit();

  /**
   * Print a description of this object.
   */
  friend std::ostream& operator<<(std::ostream& o, const SimTrackerHit& d);

  /**
   * Reset the SimTrackerHit object.
   */
  void clear();

  /**
   * Get the detector ID of the hit.
   * @return The detector ID of the hit.
   */
  int getID() const { return id_; };

  /**
   * Get the geometric layer ID of the hit.
   * @return The layer ID of the hit.
   */
  int getLayerID() const { return layer_id_; };

  /**
   * Get the module ID associated with a hit.  This is used to
   * uniquely identify a sensor within a layer_.
   * @return The module ID associated with a hit.
   */
  int getModuleID() const { return module_id_; };

  /**
   * Get the XYZ position of the hit [mm].
   * @return The position of the hit.
   */
  std::vector<float> getPosition() const { return {x_, y_, z_}; };

  /**
   * Get the energy deposited on the hit [MeV].
   * @return The energy deposited on the hit.
   */
  float getEdep() const { return edep_; };

  /**
   * Get the energy
   * @return The energy of the hit.
   */
  float getEnergy() const { return energy_; };

  /**
   * Get the global time of the hit [ns].
   * @return The global time of the hit.
   */
  float getTime() const { return time_; };

  /**
   * Get the path length between the start and end points of the
   * hit [mm].
   * @return The path length of the hit.
   */
  float getPathLength() const { return path_length_; };

  /**
   * Get the XYZ momentum of the particle at the position at which
   * the hit took place [MeV].
   * @return The momentum of the particle.
   */
  std::vector<double> getMomentum() const { return {px_, py_, pz_}; };

  /**
   * Get the Sim particle track ID of the hit.
   * @return The Sim particle track ID of the hit.
   */
  int getTrackID() const { return track_id_; };

  /**
   * Get the Sim particle track ID of the hit.
   * @return The Sim particle track ID of the hit.
   */
  int getPdgID() const { return pdg_id_; };

  /**
   * Set the detector ID of the hit.
   * @param id The detector ID of the hit.
   */
  void setID(const long id) { this->id_ = id; };

  /**
   * Set the geometric layer ID of the hit.
   * @param layerID The layer ID of the hit.
   */
  void setLayerID(const int layerID) { this->layer_id_ = layerID; };

  /**
   * Set the module ID associated with a hit.  This is used to
   * uniquely identify a sensor within a layer_.
   * @return moduleID The module ID associated with a hit.
   */
  void setModuleID(const int moduleID) { this->module_id_ = moduleID; };

  /**
   * Set the position of the hit [mm].
   * @param x_ The X position.
   * @param y_ The Y position.
   * @param z_ The Z position.
   */
  void setPosition(const float x_, const float y_, const float z_);

  /**
   * Set the energy deposited on the hit [MeV].
   * @param edep The energy deposited on the hit.
   */
  void setEdep(const float edep) { this->edep_ = edep; };

  /**
   * Set the energy of the hit.
   * @param e The energy of the hit.
   */
  void setEnergy(const float energy) { energy_ = energy; };

  /**
   * Set the global time of the hit [ns].
   * @param time The global time of the hit.
   */
  void setTime(const float time) { this->time_ = time; };

  /**
   * Set the path length of the hit [mm].
   * @param pathLength The path length of the hit.
   */
  void setPathLength(const float pathLength) {
    this->path_length_ = pathLength;
  };

  /**
   * Set the momentum of the particle at the position at which
   * the hit took place [GeV].
   * @param px The X momentum.
   * @param py The Y momentum.
   * @param pz The Z momentum.
   */
  void setMomentum(const float px, const float py, const float pz);

  /**
   * Set the Sim particle track ID of the hit.
   * @return The Sim particle track ID of the hit.
   */
  void setTrackID(const int simTrackID) { this->track_id_ = simTrackID; };

  /**
   * Set the Sim particle track ID of the hit.
   * @return The Sim particle track ID of the hit.
   */
  void setPdgID(const int simPdgID) { this->pdg_id_ = simPdgID; };

  /**
   * Sort by time of hit
   */
  bool operator<(const ldmx::SimTrackerHit& rhs) const {
    return this->getTime() < rhs.getTime();
  }

 private:
  /**
   * The detector ID.
   */
  int id_{0};

  /**
   * The layer ID.
   */
  int layer_id_{0};

  /** The module ID. */
  int module_id_{0};

  /**
   * The energy deposited on the hit.
   */
  float edep_{0};

  /**
   * The global time of the hit.
   */
  float time_{0};

  /**
   * The X momentum.
   */
  float px_{0};

  /**
   * The Y momentum.
   */
  float py_{0};

  /**
   * The Z momentum.
   */
  float pz_{0};

  /**
   * The total energy.
   */
  float energy_{0};

  /**
   * The X position.
   */
  float x_{0};

  /**
   * The Y position.
   */
  float y_{0};

  /**
   * The Z position.
   */
  float z_{0};

  /**
   * The path length of the hit.
   */
  float path_length_{0};

  /**
   * The Sim Track ID.
   */
  int track_id_{0};

  /**
   * The Sim PDG ID.
   */
  int pdg_id_{0};

  /**
   * The ROOT class definition.
   */
  ClassDef(SimTrackerHit, 4);

};  // SimTrackerHit
}  // namespace ldmx

#endif  // EVENT_SIMTRACKERHIT_H_
