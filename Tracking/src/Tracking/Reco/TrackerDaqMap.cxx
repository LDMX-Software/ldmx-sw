#include "Tracking/Reco/TrackerDaqMap.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "Framework/Exception/Exception.h"

namespace tracking::reco {

TrackerDaqMap TrackerDaqMap::fromJsonFile(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    EXCEPTION_RAISE(
        "FileNotFound",
        "TrackerDaqMap could not open DAQ map file '" + path + "'.");
  }

  nlohmann::json doc;
  try {
    in >> doc;
  } catch (const nlohmann::json::parse_error& e) {
    EXCEPTION_RAISE("BadFormat",
                    "TrackerDaqMap failed to parse DAQ map file '" + path +
                        "': " + e.what());
  }

  if (!doc.contains("sensors") || !doc.at("sensors").is_array() ||
      doc.at("sensors").empty()) {
    EXCEPTION_RAISE("BadFormat", "TrackerDaqMap file '" + path +
                                     "' has no non-empty 'sensors' array.");
  }

  TrackerDaqMap map;
  for (const auto& s : doc.at("sensors")) {
    for (const char* required :
         {"feb", "hybrid", "layer_id", "n_strips", "first_strip", "reversed"}) {
      if (!s.contains(required)) {
        EXCEPTION_RAISE("BadFormat", "TrackerDaqMap file '" + path +
                                         "' has a sensor entry missing '" +
                                         required + "': " + s.dump());
      }
    }

    const auto feb = s.at("feb").get<uint8_t>();
    const auto hybrid = s.at("hybrid").get<uint8_t>();
    const uint16_t k = key(feb, hybrid);
    if (map.sensors_.count(k) != 0u) {
      EXCEPTION_RAISE("BadFormat", "TrackerDaqMap file '" + path +
                                       "' defines (feb=" + std::to_string(feb) +
                                       ", hybrid=" + std::to_string(hybrid) +
                                       ") more than once.");
    }

    SensorInfo info;
    info.layer_id = s.at("layer_id").get<int>();
    info.n_strips = s.at("n_strips").get<int>();
    info.first_strip = s.at("first_strip").get<int>();
    info.reversed = s.at("reversed").get<bool>();
    map.sensors_[k] = info;
  }

  return map;
}

const TrackerDaqMap::SensorInfo& TrackerDaqMap::at(uint8_t feb,
                                                   uint8_t hybrid) const {
  auto it = sensors_.find(key(feb, hybrid));
  if (it == sensors_.end()) {
    EXCEPTION_RAISE("NotFound", "TrackerDaqMap has no entry for (feb=" +
                                    std::to_string(feb) +
                                    ", hybrid=" + std::to_string(hybrid) +
                                    "); guard with has() before calling at().");
  }
  return it->second;
}

}  // namespace tracking::reco
