#pragma once

#include <memory>

#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Tracking/Sim/BFieldXYZUtils.h"

namespace tracking::sim {

/// Field map that returns zero outside its grid instead of an error, so a
/// propagation leaving the map does not abort the whole track.
class ZeroOutsideBField final : public Acts::MagneticFieldProvider {
 public:
  explicit ZeroOutsideBField(
      std::shared_ptr<const InterpolatedMagneticField3> map)
      : map_(std::move(map)) {}

  Cache makeCache(const Acts::MagneticFieldContext& mctx) const override {
    return map_->makeCache(mctx);
  }

  Acts::Result<Acts::Vector3> getField(const Acts::Vector3& position,
                                       Cache& cache) const override {
    if (!map_->isInside(position)) {
      return Acts::Result<Acts::Vector3>::success(Acts::Vector3::Zero());
    }
    return map_->getField(position, cache);
  }

 private:
  std::shared_ptr<const InterpolatedMagneticField3> map_;
};

}  // namespace tracking::sim
