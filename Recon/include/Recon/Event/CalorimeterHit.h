/**
 * @file CalorimeterHit.h
 * @brief Class that represents a reconstructed hit in a calorimeter cell within
 * the detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef RECON_EVENT_CALORIMETERHIT_H_
#define RECON_EVENT_CALORIMETERHIT_H_

// ROOT
#include "TObject.h"  //For ClassDef

namespace ldmx {

/**
 * @class CalorimeterHit
 * @brief Represents a reconstructed hit in a calorimeter cell within the
 * detector
 *
 * @note This class represents the reconstructed hit information
 * from a calorimeter including detector ID, raw amplitude, corrected energy
 * and time.
 */
class CalorimeterHit {
 public:
  /**
   * Class constructor.
   */
  CalorimeterHit() {}

  /**
   * Class destructor.
   */
  virtual ~CalorimeterHit() {}

  /**
   * Clear the data in the object.
   */
  void Clear();

  /**
   * Print out the object.
   */
  void Print() const;

  /**
   * Get the detector ID.
   * @return The detector ID.
   */
  int getID() const { return id_; }

  /**
   * Set the detector ID.
   * @id The detector ID.
   */
  void setID(int id) { id_ = id; }

  /**
   * Get the amplitude of the hit, which is proportional to the
   * signal in the calorimeter cell without sampling factor
   * corrections.  Units depend on the calorimeter.
   * @return The amplitude of the hit
   */
  float getAmplitude() const { return amplitude_; }

  /**
   * Set the amplitude of the hit, which is proportional to the
   * signal in the calorimeter cell without sampling factor
   * corrections.  Units depend on the calorimeter.
   * @param amplitude The amplitude of the hit
   */
  void setAmplitude(float amplitude) { amplitude_ = amplitude; }

  /**
   * Get the calorimetric energy of the hit, corrected for
   * sampling factors [MeV].
   * @return The energy of the hit
   */
  float getEnergy() const { return energy_; }

  /**
   * Set the calorimetric energy of the hit, corrected for
   * sampling factors [MeV].
   * @param energy The energy of the hit
   */
  void setEnergy(float energy) { energy_ = energy; }

  /**
   * Get the time of the hit [ns].
   * @return The time of the hit
   */
  float getTime() const { return time_; }

  /**
   * Set the time of the hit [ns].
   * @param time The time of the hit
   */
  void setTime(float time) { time_ = time; }

  /**
   * Get the X position of the hit [mm].
   * @return the x position of the hit
   */
  float getXPos() const { return xpos_; }

  /**
   * Set the X position of the hit [mm].
   * @param xpos the x position of the hit
   */
  void setXPos(float xpos) { xpos_ = xpos; }

  /**
   * Get the Y position of the hit [mm].
   * @return the y position of the hit
   */
  float getYPos() const { return ypos_; }

  /**
   * Set the Y position of the hit [mm].
   * @param ypos the y position of the hit
   */
  void setYPos(float ypos) { ypos_ = ypos; }

  /**
   * Get the Z position of the hit [mm].
   * @return the z position of the hit
   */
  float getZPos() const { return zpos_; }

  /**
   * Set the Z position of the hit [mm].
   * @param zpos the z position of the hit
   */
  void setZPos(float zpos) { zpos_ = zpos; }

  /**
   * Is this hit a noise hit?
   * @return true if this hit is a noise hit
   */
  bool isNoise() const { return isNoise_; }

  /**
   * Set if this hit is a noise hit.
   * @param yes true if this hit is a noise hit
   */
  void setNoise(bool yes) { isNoise_ = yes; }

  /**
   * Sort by time of hit
   */
  bool operator<(const CalorimeterHit &rhs) const {
    return this->getTime() < rhs.getTime();
  }

 private:
  /** The detector ID of the hit. */
  int id_{0};

  /** The amplitude value before sampling corrections. */
  float amplitude_{0};

  /** The energy of the hit corrected by a sampling fraction. */
  float energy_{0};

  /** The time of the hit. */
  float time_{0};

  /** X Position of the hit */
  float xpos_;

  /** Y Position of the hit */
  float ypos_;

  /** Z Position of the hit */
  float zpos_;

  /** Is this a noise hit? */
  bool isNoise_;

  /**
   * The ROOT class definition.
   */
  ClassDef(CalorimeterHit, 1);
};
}  // namespace ldmx

#endif /* RECON_EVENT_CALORIMETERHIT_H_ */
