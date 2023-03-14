
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
  ~Measurement() = default;

  /**
   * Set the global position i.e. position of the measurement in the detector
   * frame.
   *
   * @param x Position in x in mm.
   * @param y Position in y in mm.
   * @param z Position in z in mm.
   */
  void setGlobalPosition(const float& x, const float& y, const float& z) {
    x_ = x;
    y_ = y;
    z_ = z;
  };

  /// @return The global position of the measurement as an array.
  [[nodiscard]] std::array<float, 3> getGlobalPosition() const {
    return std::array<float, 3>{x_, y_, z_};
  };

  /**
   * Set the local position i.e. position of the measurement in the reference
   * frame of the surface where the hit was created.
   *
   * @param u Position in U in mm.
   * @param v Position in V in mm.
   */
  void setLocalPosition(const float& u, const float& v) {
    u_ = u;
    v_ = v;
  };

  /// @return The local position of the measurement as an array.
  [[nodiscard]] std::array<float, 2> getLocalPosition() const {
    return std::array<float, 2>{u_, v_};
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
   * @param t The time in ns.
   */
  void setTime(const float& t) { t_ = t; };

  /// @return The hit time in ns.
  [[nodiscard]] float getTime() const { return t_; };

  /**
   * Set the layer ID of the sensor where this measurement took place.
   *
   * @param layerid The layer ID of the sensor associated with this measurement.
   */
  void setLayerID(const int& layerid) {
    layerid_ = layerid;
    layer_ = ((layerid_ / 100) % 10 - 1) * 2 + layerid % 2;
  };

  /// @return The layer ID of the sensor associated with this measurement.
  [[nodiscard]] int getLayerID() const { return layerid_; };

  /// @return The layer number internal to the tracker.
  int getLayer() const { return layer_; }

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

    

  /// The global position in x (mm).
  float x_{0.};
  /// The global position in x (mm).
  float y_{0.};
  /// The global position in x (mm).
  float z_{0.};
  /// Measurement time (ns).
  float t_{0.};
  /// Local position in u (mm).
  float u_{0.};
  /// Local position in v (mm).
  float v_{0.};
  /// The ID of the sensor where the measurement took place.
  int layerid_{0};
  /// The layer number internal to the tracker.
  int layer_{0};
  /// The energy deposited in the sensor where the measurement took place.
  float edep_{0.};
  /// cov(U, U)
  float cov_uu_{0.};
  /// cov(V, V)
  float cov_vv_{0.};
  /// The ID of the hit.
  int id_{0};

  ClassDef(Measurement, 1);
};  // Measurement

typedef std::vector<std::reference_wrapper<const Measurement>> Measurements;

}  // namespace ldmx
