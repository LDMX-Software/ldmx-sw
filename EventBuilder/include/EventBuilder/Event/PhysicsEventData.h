#ifndef PHYSICSEVENTDATA_H
#define PHYSICSEVENTDATA_H

#include "GenericDataBlock.h"

namespace eventbuilder {

// Combined payload that keeps subsystem data encoded as generic blocks.
struct PhysicsEventData {
  long long timestamp_ = 0;
  long long event_id_ = 0;

  // Generic, encoded blocks for downstream DAQ to decode per-subsystem
  std::vector<GenericDataBlock> blocks_;

  // List of subsystem ids present in the assembled event
  std::vector<uint64_t> systems_readout_;

  // member to reset the stored payload.
  void clear() {
    timestamp_ = 0;
    event_id_ = 0;
    blocks_.clear();
    systems_readout_.clear();
  }
};

}  // namespace eventbuilder

#endif
