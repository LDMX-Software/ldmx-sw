
#include "Detectors/Tracking/SiSensor.h"

//---< DD4hep >---//
#include "DD4hep/Shapes.h"

namespace detectors {
namespace tracker {

float SiSensor::mm{10};

SiSensor::SiSensor(const dd4hep::DetElement det_element)
    : det_element_(det_element) {

  // Set the sensor thicknesss. Doing this here once helps avoid the overhead
  // involved in casting the solid to a box when the thickness is desired.
  // NOTE: dd4hep::Box::z gives you the half-length so it needs to be
  //       multiplies by 2.
  thickness_ = static_cast<dd4hep::Box>(det_element_.solid()).z() * 2 * mm;
}

} // namespace tracker
} // namespace detectors
