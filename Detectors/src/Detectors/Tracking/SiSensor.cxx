
#include "Detectors/Tracking/SiSensor.h"

namespace detectors {
namespace tracker {

SiSensor::SiSensor(dd4hep::DetElement parent, const std::string &name, int id)
    : dd4hep::DetElement(parent, name, id) {
  setType("si_sensor");
}

} // namespace tracker
} // namespace detectors
