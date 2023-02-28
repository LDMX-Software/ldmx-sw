
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
   * @param v_r cov(U, U)
   * @param v_z cov(V, V)
   */
  Measurement(const ldmx::SimTrackerHit& hit, const float& v_r = 0.05,
              const float& v_z = 1.0);

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
   * @param v_r cov(U, U).
   * @param v_z cov(V, V).
   */
  void setLocalCovariance(const float& v_r, const float& v_z) {
    var_r_ = v_r;
    var_z_ = v_z;
  }

  /// @return The covariance of the local coordinates as an array { cov(U, U),
  /// cov(V, V) }.
  [[nodiscard]] std::array<float, 2> getLocalCovariance() const {
    return std::array<float, 2>{var_r_, var_z_};
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
  void setLayer(const int& layerid) { layerid_ = layerid; };

  /// @return The layer ID of the sensor associated with this measurement.
  [[nodiscard]] int getLayer() const { return layerid_; };

  // TODO: This is confusing ... the name should be changed.
  int getLyID() const {
    int lyID = (getLayer() / 100) % 10;
    int sID = getLayer() % 2;
    return (lyID - 1) * 2 + sID;
  }

  // TODO: This can be handled using an operator.
  static bool compareXLocation(ldmx::Measurement& m1, ldmx::Measurement& m2) {
    return m1.getGlobalPosition()[0] < m2.getGlobalPosition()[0];
  }

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
  /// The energy deposited in the sensor where the measurement took place.
  float edep_{0.};
  /// cov(U, U)
  float var_r_{0.};
  /// cov(V, V)
  float var_z_{0.};
  /// The ID of the hit.
  int id_{0};

  ClassDef(Measurement, 1);
};  // Measurement

typedef std::vector<std::reference_wrapper<const Measurement>> Measurements;

}  // namespace ldmx
