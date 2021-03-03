#ifndef DETECTORS_TRACKING_SISENSOR_H_
#define DETECTORS_TRACKING_SISENSOR_H_

//---< DD4hep >---//
#include "DD4hep/DetElement.h"

namespace detectors {
namespace tracker {

class SiSensor {
public:
  /**
   * Constructor
   *
   * @param[in] det_element The DD4hep detector element associated with
   *  this sensor.
   */
  SiSensor(const dd4hep::DetElement det_element);

  /// Destructor
  ~SiSensor() = default;

  /**
   * Get the ID associted with this sensor.  This is set during the translation
   * of the DD4hep compact to dd4hep::DetElement objects. The ID is retrieved
   * from the DetElement object.
   *
   * @return sensor ID
   */
  int id() const { return det_element_.id(); }

  bool operator<(const SiSensor &rhs) const { return id() < rhs.id(); }

  /// @return The thickness of the sensor in mm
  double thickness() { return thickness_; }

private:
  /// DD4hep detector element associated with this sensor object.
  dd4hep::DetElement det_element_;

  /// The sensor thickness
  double thickness_{0};

  /// Conversion from cm to mm i.e. from DD4hep to Tracking
  static float mm;
};
} // namespace tracker
} // namespace detectors

#endif // DETECTORS_TRACKING_SISENSOR_H_
