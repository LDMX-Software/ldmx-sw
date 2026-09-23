/**
 * @file TrigScintGeometry.h
 * @brief Class that translates a trigger-scintillator (module, bar) into a
 *        global bar-center position.
 */

#ifndef DETDESCR_TRIGSCINTGEOMETRY_H_
#define DETDESCR_TRIGSCINTGEOMETRY_H_

#include "DetDescr/TrigScintID.h"
#include "Framework/ConditionsObject.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"  // IWYU pragma: keep (EXCEPTION_RAISE in TrigScintGeometry.cxx)
#include "Math/Vector3D.h"  // IWYU pragma: keep (XYZVector returned by value)

#include <vector>

namespace trigscint {
class TrigScintGeometryProvider;
}

namespace ldmx {

/**
 * @class TrigScintGeometry
 * @brief Bar-center positions of the trigger-scintillator modules.
 *
 * The TS bars measure y. Each module is two staggered layers of `n_bars/2`
 * bars; the decoded channel id (`bar`, 0..n_bars-1) is split as
 *   layer = bar % 2,  in-layer index = bar / 2.
 * Positions are computed from parameters that mirror the detector GDML
 * constants (per-module z, in-layer y pitch, layer y stagger, layer z
 * separation, and the y of channel 0). This is delivered as a conditions
 * object so reco (e.g. the TS -> ldmx::Measurement producer) reads geometry
 * from here instead of hard-coding it.
 */
class TrigScintGeometry : public framework::ConditionsObject {
 public:
  /// Name this conditions object is registered under (must match the python
  /// provider field name).
  static constexpr const char* CONDITIONS_OBJECT_NAME{"trig_scint_geometry"};

  /// Build from a python-configured parameter set.
  TrigScintGeometry(const framework::config::Parameters& ps);

  ~TrigScintGeometry() = default;

  /**
   * Global bar-center position for a (module, bar).
   * @param module module index (0-based; index into module_z)
   * @param bar    decoded channel id, 0..n_bars-1
   * @return (x, y, z) [mm]; x is the bar center (0), y the measured coordinate.
   */
  ROOT::Math::XYZVector getBarPosition(int module, int bar) const;

  /// Convenience overload taking a TrigScintID.
  ROOT::Math::XYZVector getBarPosition(ldmx::TrigScintID id) const {
    return getBarPosition(id.module(), id.bar());
  }

  /// Number of configured modules.
  int getNumModules() const { return static_cast<int>(module_z_.size()); }
  /// Number of bars (channels) per module.
  int getNumBars() const { return n_bars_; }
  /// z separation between the two layers within a module [mm].
  double getLayerZSeparation() const { return layer_z_sep_; }

 private:
  /// global z of each module's (layer-pair) center [mm]
  std::vector<double> module_z_;
  /// global y of bar 0 [mm] (bar 0 is most +y)
  double y0_;
  /// in-layer y pitch [mm]
  double layer_pitch_;
  /// y stagger of layer 1 relative to layer 0 [mm]
  double layer_y_shift_;
  /// z separation between the two layers within a module [mm]
  double layer_z_sep_;
  /// number of bars (channels) per module
  int n_bars_;
};

}  // namespace ldmx

#endif  // DETDESCR_TRIGSCINTGEOMETRY_H_
