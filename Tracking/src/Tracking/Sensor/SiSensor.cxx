#include "Tracking/Sensor/SiSensor.cxx"

namespace tracking::sensor {

  SiSensor::SiSenor(const std::shared_ptr<Acts::Surface>& surface,
                    const Acts::Transform3& default_transform, double thickness, int module_id)
    :DetectorElement(surface, default_transform, thickness){
    this.module_id_=module_id; 
    this.loadChannelConstants();
    
  }
  
  void SiSensor::loadChannelConstants(){
    //do something
  }
}
