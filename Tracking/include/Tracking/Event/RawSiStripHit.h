
#ifndef TRACKING_EVENT_RAWSISTRIPHIT_H_
#define TRACKING_EVENT_RAWSISTRIPHIT_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

namespace ldmx {

/**
 * Implementation of a raw digitized hit from a silicon strip detector.
 *
 * This class is meant to encapsulate the raw data coming from a silicon strip
 * detector prior to any additional processing. Typically, the raw data will
 * contain a header with ID information and timestamp, a tail with error info
 * and multiple 32 bit data samples.
 */
class RawSiStripHit {
 public:
  /// Default constructor
  RawSiStripHit() = default;

  /**
   * Constructor.
   *
   * @param[in] layer_id    Sensor layer identifier (from tracking geometry).
   * @param[in] strip_id    Readout strip index within the sensor.
   * @param[in] samples     The ADC samples composing this hit (16-bit each).
   * @param[in] time        Hit timestamp [ns].
   * @param[in] track_id    Geant4 track ID of the particle that created this
   * hit.
   * @param[in] pdg_id      PDG particle ID of the particle that created this
   * hit.
   * @param[in] sim_hit_id  Detector ID of the originating SimTrackerHit.
   */
  RawSiStripHit(int layer_id, int strip_id, std::vector<short> samples,
                long time, int track_id = -1, int pdg_id = 0,
                int sim_hit_id = -1, float edep = 0.f);

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~RawSiStripHit(){};

  /**
   * Clear the vector of samples and set the timestamp to 0.
   *
   * This class is needed by ROOT when building the dictionary.
   */
  void clear();

  /**
   * Print the string representation of this object.
   *
   * This class is needed by ROOT when building the dictionary.
   */
  friend std::ostream &operator<<(std::ostream &o, const RawSiStripHit &d);

  /**
   * Get the digitized (ADC) samples composing this hit.
   *
   * This can be a single value or multiple values depending on the readout
   * being used.
   *
   * @param[in] samples_ The ADC values composing this hit. For now, the size
   *    of a sample is assumed to be 16 bits.
   *
   * @return[out] A std::vector of 16 bit samples.
   */
  std::vector<short> getSamples() const { return samples_; }

  /// Get the sensor layer identifier.
  int getLayerID() const { return layer_id_; }

  /// Get the readout strip index within the sensor.
  int getStripID() const { return strip_id_; }

  /**
   * Get the time stamp of this hit.
   *
   * This is the time stamp as set by the data aquisition system. This will
   * typically be in units of ns.
   *
   * @return[out] The timestamp of this hit in ns.
   */
  long getTime() const { return time_; }

  /// Get the Geant4 track ID of the particle that created this hit (-1 if
  /// unknown).
  int getTrackID() const { return track_id_; }

  /// Get the PDG particle ID of the particle that created this hit (0 if
  /// unknown).
  int getPdgID() const { return pdg_id_; }

  /// Get the detector ID of the originating SimTrackerHit (-1 if unknown).
  int getSimHitID() const { return sim_hit_id_; }

  /// Get the energy deposited by the parent SimTrackerHit [MeV] (0 if unknown).
  float getEdep() const { return edep_; }

  /**
   * When the less than operator is used for comparison, return true if this
   * hit's time is less than the hit we are comparing against.
   *
   * @param[in] rhs The RawStripHit on the right side of the comparison.
   *
   * @return[out] True if the timestamp of this hit is less than the hit being
   *    compared against.
   */
  bool operator<(const RawSiStripHit &rhs) const {
    return getTime() < rhs.getTime();
  }

  /**
   * Overload the stream insertion operator to output a string representation
   * of this RawStripHit.
   *
   * @param[in] output The output stream where the string representation will
   *    be inserted.
   * @param[in] hit The RawSiStripHit to output.
   *
   * @return[out] An ostream object with the string representation of
   *    RawSiStripHit inserted.
   */
  friend std::ostream &operator<<(std::ostream &output,
                                  const RawSiStripHit &hit);

 protected:
  /// Sensor layer identifier (from tracking geometry).
  int layer_id_{-1};

  /// Readout strip index within the sensor.
  int strip_id_{-1};

  /// 16 bit ADC samples associated with this hit.
  std::vector<short> samples_;

  /// The hit time stamp in units of ns.
  long time_{0};

  // Truth information (for MC truth matching; -1/0 means not set)
  /// Geant4 track ID of the particle that created this hit.
  int track_id_{-1};
  /// PDG particle ID of the particle that created this hit.
  int pdg_id_{0};
  /// Detector ID of the originating SimTrackerHit.
  int sim_hit_id_{-1};
  /// Energy deposited by the parent SimTrackerHit [MeV].
  float edep_{0.f};

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(RawSiStripHit, 5);

};  // RawSiStripHit
}  // namespace ldmx

#endif  // TRACKING_EVENT_RAWSISTRIPHIT_H_
