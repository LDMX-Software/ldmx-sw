#pragma once

//~~ StdLib ~~//
#include <array>

#include "SimCore/Event/SimTrackerHit.h"

//~~ ROOT ~~//
#include "TObject.h"  // Needed for ClassDef, ClassImp

namespace ldmx {
class Measurement {
 public:
  /// Default constructor
  Measurement() = default;

  /**
   * Constructor that uses a SimTrackerHit to populate the global position,
   * deposited energy, ID and measurement time. The cov(U, U) and cov(V, V)
   * are optional.
   *
   * @param hit The SimTrackerHit used to set the initial Measurement values.
   * @param cov_uu cov(U, U)
   * @param cov_vv cov(V, V)
   */
  Measurement(const ldmx::SimTrackerHit& hit, const float& cov_uu = 0.05,
              const float& cov_vv = 1.0);

  /// Default destructor
  virtual ~Measurement() = default;

  /**
   * Set the global position i.e. position of the measurement in the detector
   * frame.
   *
   * @param x Position in x_ in mm.
   * @param y Position in y in mm.
   * @param z Position in z in mm.
   */
  void setGlobalPosition(const float& meas_x, const float& meas_y, const float& meas_z) {
    meas_x_ = meas_x;
    meas_y_ = meas_y;
    meas_z_ = meas_z;
  };

  /// @return The global position of the measurement as an array.
  [[nodiscard]] std::array<float, 3> getGlobalPosition() const {
    return std::array<float, 3>{meas_x_, meas_y_, meas_z_};
  };

  /**
   * Set the local position i.e. position of the measurement in the reference
   * frame of the surface where the hit was created.
   *
   * @param u Position in U in mm.
   * @param v Position in V in mm.
   */
  void setLocalPosition(const float& meas_u, const float& meas_v) {
    meas_u_ = meas_u;
    meas_v_ = meas_v;
  };

  /// @return The local position of the measurement as an array.
  [[nodiscard]] std::array<float, 2> getLocalPosition() const {
    return std::array<float, 2>{meas_u_, meas_v_};
  };

  /**
   * Set cov(U,U) and cov(V, V).
   *
   * @param cov_uu cov(U, U).
   * @param cov_vv cov(V, V).
   */
  void setLocalCovariance(const float& cov_uu, const float& cov_vv) {
    cov_uu_ = cov_uu;
    cov_vv_ = cov_vv;
  }

  /// @return The covariance of the local coordinates as an array { cov(U, U),
  /// cov(V, V) }.
  [[nodiscard]] std::array<float, 2> getLocalCovariance() const {
    return std::array<float, 2>{cov_uu_, cov_vv_};
  };

  /**
   * Set the measurement time in ns.
   *
   * @param meas_t The time in ns.
   */
  void setTime(const float& meas_t) { meas_t_ = meas_t; };

  /// @return The hit time in ns.
  [[nodiscard]] float getTime() const { return meas_t_; };

  /**
   * Set the layer_ ID of the sensor where this measurement took place.
   *
   * @param layer_id The layer_ ID of the sensor associated with this measurement.
   */
  void setLayerID(const int& layer_id) {
    layer_id_ = layer_id;
    layer_ = ((layer_id_ / 100) % 10 - 1) * 2 + layer_id % 2;
  };

  /// @return The layer_ ID of the sensor associated with this measurement.
  [[nodiscard]] int getLayerID() const { return layer_id_; };

  /// @return The layer_ number internal to the tracker.
  int getLayer() const { return layer_; }

  /// Add a trackId to the internal vector
  void addTrackId(int trk_id) { track_ids_.push_back(trk_id); };
  /// @return the sim particle IDs that compose the measurement
  std::vector<unsigned int> getTrackIds() const { return track_ids_; };

  ///  @return The energy deposited in the sensor where the measurement took
  ///  place.
  float getEdep() const { return edep_; };

  /**
   * Overload the stream insertion operator to output a string representation of
   * this Measurement.
   *
   * @param[in] output The output stream where the string representation will be
   *  inserted.
   * @param[in] measurement The Measurement object to print.
   *
   * @return[out] An ostream object with the string representation of
   * Measurement.
   *
   */
  friend std::ostream& operator<<(std::ostream& output,
                                  const Measurement& measurement);

 private:
  /// The global position in x_ (mm).
  float meas_x_{0.};
  /// The global position in x_ (mm).
  float meas_y_{0.};
  /// The global position in x_ (mm).
  float meas_z_{0.};
  /// Measurement time (ns).
  float meas_t_{0.};
  /// Local position in u (mm).
  float meas_u_{0.};
  /// Local position in v (mm).
  float meas_v_{0.};
  /// The ID of the sensor where the measurement took place.
  int layer_id_{0};
  /// The layer_ number internal to the tracker.
  int layer_{0};
  /// The energy deposited in the sensor where the measurement took place.
  float edep_{0.};
  /// cov(U, U)
  float cov_uu_{0.};
  /// cov(V, V)
  float cov_vv_{0.};
  /// The ID of the hit.
  int id_{0};
  /// TrackIDs the vector of TrackIDs that form the measurement
  std::vector<unsigned int> track_ids_{};

  ClassDef(Measurement, 2);
};  // Measurement

typedef std::vector<Measurement> Measurements;
// typedef std::vector<std::reference_wrapper<const Measurement>> Measurements;

}  // namespace ldmx
