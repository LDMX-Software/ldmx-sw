#ifndef GENERICDATABLOCK_H
#define GENERICDATABLOCK_H
#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>
#include "../Fragment.hh"

namespace eventbuilder {

struct GenericDataBlock {
    uint64_t subsystem_id = 0;
    uint64_t timestamp_ns = 0;
    std::vector<uint8_t> data; // raw encoded payload
    uint32_t checksum = 0;

    size_t size() const { return data.size(); }
};

} // namespace eventbuilder

#endif // GENERICDATABLOCK_H
