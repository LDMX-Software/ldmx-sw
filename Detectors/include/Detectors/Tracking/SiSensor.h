#ifndef DETECTORS_TRACKING_SISENSOR_H_
#define DETECTORS_TRACKING_SISENSOR_H_

/*~~~~~~~~~~~~*/
/*   DD4hep   */
/*~~~~~~~~~~~~*/
#include "DD4hep/DetElement.h"

namespace detectors {
namespace tracker {

/**
 */
class SiSensor : public dd4hep::DetElement {
 public:
  /**
   * Constructor
   * @param[in] parent The parent detector element in which this element is
   * defined in.
   * @param[in] name The name of this sensor
   * @param[in] id The detector ID of this sensor.
   */
  SiSensor(dd4hep::DetElement parent, const std::string &name, int id);

  /// Destructor
  ~SiSensor() = default;
};
} // namespace tracker
} // namespace detectors

#endif // DETECTORS_TRACKING_SISENSOR_H_
