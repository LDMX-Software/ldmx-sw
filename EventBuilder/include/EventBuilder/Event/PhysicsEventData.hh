#ifndef PHYSICSEVENTDATA_H
#define PHYSICSEVENTDATA_H
#pragma once
#include "GenericDataBlock.hh"

namespace eventbuilder {

// Combined payload that keeps subsystem data encoded as generic blocks.
struct PhysicsEventData {
    long long timestamp = 0;
    long long event_id = 0;

    // Generic, encoded blocks for downstream DAQ to decode per-subsystem
    std::vector<GenericDataBlock> blocks;
    
    // List of subsystem ids present in the assembled event
    std::vector<uint64_t> systems_readout;

    // member to reset the stored payload.
    void clear() {
        timestamp = 0;
        event_id = 0;
        blocks.clear();
        systems_readout.clear();
    }
};

} // namespace eventbuilder

#endif
