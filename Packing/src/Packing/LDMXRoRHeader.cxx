#include "Packing/LDMXRoRHeader.h"

#include <cassert>

namespace packing {

const std::unordered_map<std::string, int> LDMXRoRHeader::SUBSYSTEM_ID = {
    {"tdaq", 1}, {"ts", 2}, {"tracker", 4}, {"ecal", 5}, {"hcal", 5}};

const std::unordered_map<std::string, int> LDMXRoRHeader::CONTRIBUTOR_ID = {
    {"tdaq", -1}, {"ts", 1}, {"tracker", -1}, {"ecal", 40}, {"hcal", 20}};

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

std::string LDMXRoRHeader::getSubsystemName(uint8_t subsystem_id,
                                            uint8_t contributor_id) {
  // Check each subsystem's IDs to find a match
  for (const auto& name_id_pair : SUBSYSTEM_ID) {
    const auto& name = name_id_pair.first;
    int subsys_id = name_id_pair.second;

    if (subsys_id != subsystem_id) {
      continue;  // Not a match on subsystem ID
    }

    // Check if contributor ID matches (if it's set)
    auto contrib_it = CONTRIBUTOR_ID.find(name);
    if (contrib_it != CONTRIBUTOR_ID.end()) {
      int expected_contrib = contrib_it->second;
      if (expected_contrib != -1 && expected_contrib != contributor_id) {
        continue;  // Contributor ID doesn't match, try next
      }
    }

    // Found a match
    return name;
  }

  // No match found, return generic name
  return "subsystem" + std::to_string(subsystem_id) + "c" +
         std::to_string(contributor_id);
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
