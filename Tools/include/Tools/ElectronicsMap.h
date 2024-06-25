/**
 * @file ElectronicsMap.h
 * @brief Class designed for efficient mapping between electronics ids (using
 * packed index techniques) and detector ids (which are generally arbitrary in
 * format)
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef TOOLS_ELECTRONICSMAP_H_
#define TOOLS_ELECTRONICSMAP_H_

#include <map>
#include <sstream>
#include <vector>

namespace ldmx {

/**
 * ElectronicsMap
 *
 * A class for efficient mapping between electronics IDs (using packed index
 * techniques) and detector IDs which are arbitrarily formatted.
 *
 * @tparam[in] ElectronicsID class of electronics ID
 * @tparam[in] DetID class of detector IDs
 */
template <class ElectronicsID, class DetID>
class ElectronicsMap {
 public:
  ElectronicsMap(bool want_d2e = false)
      : eid2did_{ElectronicsID::MAX_INDEX, 0}, makeD2E_(want_d2e) {}

  /**
   * Remove all entries from the map
   */
  void clear() {
    eid2did_ = std::vector<DetectorID::RawValue>(ElectronicsID::MAX_INDEX, 0);
    did2eid_.clear();
  }

  /**
   * Add an entry to the map
   */
  void addEntry(ElectronicsID eid, DetID did) {
    unsigned int index = eid.index();
    if (index >= eid2did_.size()) {
      EXCEPTION_RAISE("ElectronicsMapOverflow",
                      "Attempted to insert electronics with index " +
                          std::to_string(index) +
                          " which is larger than allowed in this map");
    }
    eid2did_[index] = did.raw();
    if (makeD2E_) did2eid_.insert(std::make_pair(did.raw(), eid));
  }

  /**
   * Tests if a given electronics id is in the map
   */
  bool exists(ElectronicsID eid) const {
    return (eid.index() < eid2did_.size() && eid2did_[eid.index()] != 0);
  }

  /**
   * Tests if a given detector id is in the map
   * This method is slow O(N) if the map is not configured for detector id to
   * electronics id.
   */
  bool exists(DetID did) const {
    if (makeD2E_) {
      return did2eid_.find(did.raw()) == did2eid_.end();
    } else {
      for (auto i : eid2did_) {
        if (i == did.raw()) return true;
      }
      return false;
    }
  }

  /**
   * Get the detector ID for this electronics ID
   */
  DetID get(ElectronicsID eid) const {
    if (eid.index() >= eid2did_.size())
      return DetID();
    else
      return DetID(eid2did_[eid.index()]);
  }

  /**
   * Get the electronics ID for this detector ID
   * This method is slow O(N) if the map is not configured for detector id to
   * electronics id. Throws an exception on failure as there is no
   * globally-defined invalid electronics
   */
  ElectronicsID get(DetID did) const {
    if (makeD2E_) {
      auto itr = did2eid_.find(did.raw());
      if (itr == did2eid_.end()) {
        std::stringstream ss;
        ss << "Unable to find mapping for det id " << did;
        EXCEPTION_RAISE("ElectronicsMapNotFound", ss.str());
      }
      return itr->second;
    } else {
      for (unsigned int i = 0; i < eid2did_.size(); i++) {
        if (eid2did_[i] == did.raw()) return ElectronicsID::idFromIndex(i);
      }
      std::stringstream ss;
      ss << "Unable to find mapping for det id " << did;
      EXCEPTION_RAISE("ElectronicsMapNotFound", ss.str());
    }
  }

 private:
  /**
   * Linear-time map for electronics (packed index) to raw detector id
   */
  std::vector<DetectorID::RawValue> eid2did_;
  /**
   * Flag to determine if did2eid should be filled (resource optimization)
   */
  bool makeD2E_;

  /**
   * Log(N) map for raw detector id to electronics id
   */
  std::map<DetectorID::RawValue, ElectronicsID> did2eid_;
};

}  // namespace ldmx
#endif  // TOOLS_ELECTRONICSMAP_H_
