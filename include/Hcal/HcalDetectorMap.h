/**
 * @file HcalDetectorMap.h
 * @brief Class which contains logic for how the detector items connect to and
 * relate with the reconstruction cells
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef HCAL_HCALDETECTORMAP_H_
#define HCAL_HCALDETECTORMAP_H_

#include <vector>

#include "Conditions/GeneralCSVLoader.h"
#include "DetDescr/HcalElectronicsID.h"
#include "DetDescr/HcalDigiID.h"
#include "Framework/ConditionsObject.h"
#include "Tools/ElectronicsMap.h"

namespace hcal {

/**
 * \brief Class which provides various information about how the parts of the
 * Hcal connect to each other.
 *
 * The class is loaded from a single connection table, currently in the form of a CSV file,
 * using the HcalDetectorMapLoader declared and defined in the source.
 *
 * We inherit from two classes: (1) the ConditionsObject because we are a
 * conditions object and (2) from the ElectronicsMap template because we
 * are a electronics map.
 */
class HcalDetectorMap
    : public framework::ConditionsObject,
      public ldmx::ElectronicsMap<ldmx::HcalElectronicsID, ldmx::HcalDigiID> {
 public:
  /// The name of the EID <-> DetID map for the ECal
  static constexpr const char* CONDITIONS_OBJECT_NAME{"HcalDetectorMap"};

  /**
   * Default constructor which builds the necessary maps.
   *
   * @param[in] want_d2e true if we want to build a reverse mapping
   * Building a reverse (detector->electronics) map takes extra time
   * and memory so it should be off by default.
   */
  HcalDetectorMap(const std::string& connections_table, bool want_d2e);

  /// Provider which loads the map
  friend class HcalDetectorMapLoader;
};

}  // namespace hcal

#endif  // HCAL_HCALDETECTORMAP_H_

