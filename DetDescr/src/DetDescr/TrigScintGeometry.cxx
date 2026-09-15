#include "DetDescr/TrigScintGeometry.h"

namespace ldmx {

TrigScintGeometry::TrigScintGeometry(const framework::config::Parameters& ps)
    : framework::ConditionsObject(TrigScintGeometry::CONDITIONS_OBJECT_NAME) {
  module_z_ = ps.get<std::vector<double>>("module_z");
  y0_ = ps.get<double>("y0");
  layer_pitch_ = ps.get<double>("layer_pitch");
  layer_y_shift_ = ps.get<double>("layer_y_shift");
  layer_z_sep_ = ps.get<double>("layer_z_sep");
  n_bars_ = ps.get<int>("n_bars");
}

ROOT::Math::XYZVector TrigScintGeometry::getBarPosition(int module,
                                                        int bar) const {
  if (module < 0 || module >= static_cast<int>(module_z_.size())) {
    EXCEPTION_RAISE("TrigScintGeometry",
                    "Requested module " + std::to_string(module) +
                        " is out of range [0," +
                        std::to_string(module_z_.size()) + ").");
  }
  if (bar < 0 || bar >= n_bars_) {
    EXCEPTION_RAISE("TrigScintGeometry",
                    "Requested bar " + std::to_string(bar) +
                        " is out of range [0," + std::to_string(n_bars_) + ").");
  }

  // two staggered layers: bar/2 = in-layer index, bar%2 = layer
  const int in_layer = bar / 2;
  const int layer = bar % 2;
  const double y = y0_ - (layer_pitch_ * in_layer + layer_y_shift_ * layer);
  const double z = module_z_[module] + (layer == 1 ? 0.5 : -0.5) * layer_z_sep_;
  return ROOT::Math::XYZVector(0.0, y, z);
}

}  // namespace ldmx
