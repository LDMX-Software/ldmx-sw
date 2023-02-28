#ifndef TRACKING_EVENT_MEASUREMENT_H_
#define TRACKING_EVENT_MEASUREMENT_H_

#include <array>

#include "SimCore/Event/SimTrackerHit.h"
#include "TObject.h"

namespace ldmx {

class Measurement {
 public:
  /// Default constructor
  Measurement() = default;

  /**
   */
  Measurement(const ldmx::SimTrackerHit& hit, const float& v_r = 0.05,
              const float& v_z = 1.0);

  /// Default destructor
  ~Measurement() = default;

  void setGlobalPosition(float x, float y, float z) {
    x_ = x;
    y_ = y;
    z_ = z;
  };
  std::array<float, 3> getGlobalPosition() const {
    return std::array<float, 3>{{x_, y_, z_}};
  };

  void setLocalPosition(float u, float v) {
    u_ = u;
    v_ = v;
  };
  std::array<float, 2> getLocalPosition() const {
    return std::array<float, 2>{{u_, v_}};
  };

  void setLocalCovariance(const float& v_r, const float& v_z) {
    var_r_ = v_r;
    var_z_ = v_z;
  }

  void setTime(float t) { t_ = t; };
  float getTime() const { return t_; };

  void setLayer(int layerid) { layerid_ = layerid; };
  int getLayer() const { return layerid_; };

  int getLyID() const {
    int lyID = (getLayer() / 100) % 10;
    int sID = getLayer() % 2;
    return (lyID - 1) * 2 + sID;
  }

  static bool compareXLocation(ldmx::Measurement& m1, ldmx::Measurement& m2) {
    return m1.getGlobalPosition()[0] < m2.getGlobalPosition()[0];
  }

 private:
  float x_, y_, z_, t_;
  float u_, v_;
  int layerid_;
  float edep_;
  float var_r_{0};
  float var_z_{0};
  int id_;

  ClassDef(Measurement, 1);
};  // Measurement

typedef std::vector<std::reference_wrapper<const Measurement>> Measurements;

}  // namespace ldmx

#endif  // TRACKING_EVENT_MEASUREMENT_H_
