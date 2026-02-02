#include "Packing/LDMXRoRHeader.h"

#include <cassert>

namespace packing {

const std::unordered_map<std::string, int> LDMXRoRHeader::SUBSYSTEM_ID = {
    {"tdaq", 1}, {"ts", 2}, {"tracker", 4}, {"ecal", 5}, {"hcal", 5}};

const std::unordered_map<std::string, int> LDMXRoRHeader::CONTRIBUTOR_ID = {
    {"tdaq", -1}, {"ts", 1}, {"tracker", -1}, {"ecal", 20}, {"hcal", 40}};

std::tuple<int, int> LDMXRoRHeader::subsystem(const std::string& name) {
  auto subsys_it{SUBSYSTEM_ID.find(name)};
  if (subsys_it == SUBSYSTEM_ID.end()) {
    return std::make_tuple(-1, -1);
  }

  int subsys{subsys_it->second}, contrib{-1};
  auto contrib_it{CONTRIBUTOR_ID.find(name)};
  if (contrib_it != CONTRIBUTOR_ID.end()) {
    contrib = contrib_it->second;
  }
  return std::make_tuple(subsys, contrib);
}

utility::Reader& LDMXRoRHeader::read(utility::Reader& r) {
  uint8_t sentinel;
  uint32_t zero;
  if (!(r >> version_ >> subsystem_ >> contributor_ >> sentinel)) {
    return r;
  }

  // sentinel should be 0xa5
  assert(sentinel == 0xa5);

  if (!(r >> zero)) {
    return r;
  }

  // next 32b should be zero since they are unused
  assert(zero == 0);

  return (r >> timestamp_);
}

}  // namespace packing
