#ifndef TRACKING_RECO_TRACKERDAQMAP_H_
#define TRACKING_RECO_TRACKERDAQMAP_H_

#include <cstdint>
#include <map>
#include <string>

namespace tracking::reco {

/**
 * Tracker DAQ map: the correspondence between an electronics sensor address
 * (feb, hybrid) and the detector layer it reads, plus the strip transform that
 * turns a physical channel (pchannel) into a sensor strip index.
 */
class TrackerDaqMap {
 public:
  /// Everything the mapper needs to know about one physical sensor.
  struct SensorInfo {
    int layer_id_{-1};  ///< Acts surface id (volume*1000 + layer*100 + sensor).
    int n_strips_{0};   ///< Number of bonded strips on the sensor.
    int first_strip_{0};    ///< Sensor strip index that pchannel 0 maps to.
    bool reversed_{false};  ///< True if the hybrid reads the sensor descending.
  };

  TrackerDaqMap() = default;

  /**
   * Load a DAQ map from a JSON file.
   *
   * Expected schema:
   * @code
   * { "sensors": [ { "feb": 0, "hybrid": 0, "layer_id": 3101,
   *                  "n_strips": 640, "first_strip": 0, "reversed": false },
   *                ... ] }
   * @endcode
   * Extra keys (e.g. "station", "orientation", "_comment") are ignored.
   *
   * @throws framework::exception::Exception if the file cannot be opened, is
   * not valid JSON, lacks a non-empty "sensors" array, has a malformed entry,
   * or defines the same (feb, hybrid) twice.  There is no silent empty-map
   * failure mode.
   */
  static TrackerDaqMap fromJsonFile(const std::string& path);

  /// True if the map has an entry for this electronics sensor.
  bool has(uint8_t feb, uint8_t hybrid) const {
    return sensors_.find(key(feb, hybrid)) != sensors_.end();
  }

  /**
   * Look up the sensor read by (feb, hybrid).
   * @throws framework::exception::Exception if there is no such entry; callers
   *         that want to skip unmapped sensors must guard with has() first.
   */
  const SensorInfo& at(uint8_t feb, uint8_t hybrid) const;

  /// Number of sensors in the map.
  std::size_t size() const { return sensors_.size(); }

  /// All sensors, keyed by (feb << 8) | hybrid.  For consumers that need to
  /// index by something other than the electronics address (e.g. build a
  /// layer_id -> n_strips lookup).
  const std::map<uint16_t, SensorInfo>& sensors() const { return sensors_; }

 private:
  static uint16_t key(uint8_t feb, uint8_t hybrid) {
    return static_cast<uint16_t>((static_cast<uint16_t>(feb) << 8) | hybrid);
  }

  std::map<uint16_t, SensorInfo> sensors_;  ///< key = (feb << 8) | hybrid
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_TRACKERDAQMAP_H_
