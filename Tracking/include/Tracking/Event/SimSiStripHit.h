
#ifndef TRACKING_EVENT_SIMSISTRIPHIT_H_
#define TRACKING_EVENT_SIMSISTRIPHIT_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <vector>

//----------//
//   LDMX   //
//----------//
#include "Tracking/Event/SiStripHit.h"

namespace ldmx {

/**
 * Truth (Monte Carlo) representation of a digitized silicon strip detector hit.
 *
 * This is the output of the realistic tracker digitization of simulated data.
 * In addition to the ADC samples and time stamp held by the SiStripHit base
 * class, it carries the sensor position (layer/strip) and the MC-truth matching
 * information (originating track, particle, SimTrackerHit and deposited energy).
 * The reco-level (real data) counterpart is RawSiStripHit.
 */
class SimSiStripHit : public SiStripHit {
 public:
  /// Default constructor
  SimSiStripHit() = default;

  /**
   * Constructor.
   *
   * @param[in] layer_id    Sensor layer identifier (from tracking geometry).
   * @param[in] strip_id    Readout strip index within the sensor.
   * @param[in] samples     The ADC samples composing this hit (16-bit each).
   * @param[in] time        Hit timestamp [ns].
   * @param[in] track_id    Geant4 track ID of the particle that created this
   *    hit.
   * @param[in] pdg_id      PDG particle ID of the particle that created this
   *    hit.
   * @param[in] sim_hit_id  Detector ID of the originating SimTrackerHit.
   * @param[in] edep        Energy deposited by the parent SimTrackerHit [MeV].
   */
  SimSiStripHit(int layer_id, int strip_id, std::vector<short> samples,
                long time, int track_id = -1, int pdg_id = 0,
                int sim_hit_id = -1, float edep = 0.f);

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~SimSiStripHit() = default;

  /**
   * Clear the samples, time stamp, sensor position and truth fields.
   *
   * This method is needed by ROOT when building the dictionary.
   */
  void clear() override;

  /// Get the sensor layer identifier.
  int getLayerID() const { return layer_id_; }

  /// Get the readout strip index within the sensor.
  int getStripID() const { return strip_id_; }

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

  /// Set the sensor layer identifier.
  void setLayerID(int v) { layer_id_ = v; }
  /// Set the readout strip index within the sensor.
  void setStripID(int v) { strip_id_ = v; }
  /// Set the Geant4 track ID of the particle that created this hit.
  void setTrackID(int v) { track_id_ = v; }
  /// Set the PDG particle ID of the particle that created this hit.
  void setPdgID(int v) { pdg_id_ = v; }
  /// Set the detector ID of the originating SimTrackerHit.
  void setSimHitID(int v) { sim_hit_id_ = v; }
  /// Set the energy deposited by the parent SimTrackerHit [MeV].
  void setEdep(float v) { edep_ = v; }

  /**
   * Overload the stream insertion operator to output a string representation
   * of this SimSiStripHit.
   *
   * @param[in] output The output stream where the string representation will
   *    be inserted.
   * @param[in] hit The SimSiStripHit to output.
   *
   * @return An ostream object with the string representation of
   *    SimSiStripHit inserted.
   */
  friend std::ostream &operator<<(std::ostream &output,
                                  const SimSiStripHit &hit);

 protected:
  /// Sensor layer identifier (from tracking geometry).
  int layer_id_{-1};

  /// Readout strip index within the sensor.
  int strip_id_{-1};

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
  ClassDefOverride(SimSiStripHit, 1);

};  // SimSiStripHit
}  // namespace ldmx

#endif  // TRACKING_EVENT_SIMSISTRIPHIT_H_
