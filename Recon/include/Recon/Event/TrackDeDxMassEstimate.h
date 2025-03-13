/**
 * @file TrackDeDxMassEstimate.h
 * @brief Class that represents the estimated mass of a particle
 * using tracker dE/dx information
 * @author Danyi Zhang, Tamas Almos Vami (UCSB)
 */

#ifndef RECON_TRACKDEDXMASSESTIMATE_H_
#define RECON_TRACKDEDXMASSESTIMATE_H_

#include <iostream>

// ROOT
#include "TObject.h"  //For ClassDef

// LDMX
// #include "Event/Track.h"
// #include "Event/Measurement.h"

namespace ldmx {
/**
 * @class TrackDeDxMassEstimate
 * @brief Represents the estimated mass of a particle
 * using tracker dE/dx information
 * @note This class represents the estimated mass information
 * from a tracker including mass, track index, and the track type
 */

class TrackDeDxMassEstimate {
 public:
  /**
   * Class constructor.
   */
  TrackDeDxMassEstimate();

  /**
   * Class destructor.
   */
  virtual ~TrackDeDxMassEstimate() = default;

  /**
   * Clear the data in the object.
   */
  void Clear();

  /**
   * Print out the object.
   */
  void Print() const;

  /**
   * Set the momentum of the particle/track.
   * @param momentum The momentum of the particle/track.
   */
  void setMomentum(float momentum) { momentum_ = momentum; }

  /**
   * Set the Ih of the particle/track.
   * @param theIh The Ih of the particle/track.
   */
  void setIh(float theIh) { theIh_ = theIh; }

  /**
   * Set the estimated mass of the particle/track.
   * @param mass The estimated mass of the particle/track.
   */
  void setMass(float mass) { mass_ = mass; }

  /**
   * Set the index of the track.
   * @param track_index The index of the track.
   */
  void setTrackIndex(int track_index) { track_index_ = track_index; }

  /**
   * Set the type of the track.
   * @param track_type The type of the track.
   * 1: tagger track, 2: recoil track
   * Possibly consider truth with 0
   * and ECAL with 3
   */
  void setTrackType(int track_type) { track_type_ = track_type; }

  /**
   * Get the momentum of the particle/track.
   * @return The momentum of the particle/track.
   */
  float getMomentum() const { return momentum_; }

  /**
   * Get the Ih of the particle/track.
   * @return The Ih of the particle/track.
   */
  float getIh() const { return theIh_; }

  /**
   * Get the estimated mass of the particle/track.
   * @return The estimated mass of the particle/track.
   */
  float getMass() const { return mass_; }

  /**
   * Get the index of the track.
   * @return The index of the track.
   */
  int getTrackIndex() const { return track_index_; }

  /**
   * Get the type of the track.
   * @return The type of the track.
   */
  int getTrackType() const { return track_type_; }

 private:
  /* The momentum of the particle/track */
  float momentum_{0.};

  /* The Ih of the particle/track */
  float theIh_{0.};

  /* The estimated mass of the particle/track */
  float mass_{0.};

  /* The index of the track */
  int track_index_{-1};

  /* The type of the track */
  int track_type_{-1};

  /**
   * The ROOT class definition.
   */
  ClassDef(TrackDeDxMassEstimate, 2);
};
}  // namespace ldmx

#endif  // RECON_TRACKDEDXMASSESTIMATE_H_
