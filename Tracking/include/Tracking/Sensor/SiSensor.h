#pragma once

#include <vector>
#include <map>

#include "Tracking/geo/DetectorElement.h"


namespace tracking::sensor {

  class SiSensor:public tracking::geo::DetectorElement{

  public:
    SiSensor(const std::shared_ptr<Acts::Surface>& surface,
             const Acts::Transform3& default_transform, double thickness,
             int module_id);

    
    static const int NUMBER_OF_SAMPLE{3};    
    static const int AMPLITUDE_INDEX{0};
    static const int T0_INDEX{1};
    static const int TP_INDEX{2};
     
     //     static const std::string ELECTRON_SIDE = "ELECTRON";
     // static const std::string POSITRON_SIDE = "POSITRON";

     //  electrode info, as struct?
     struct electrodes{
       std::string carrier;       
     }; 
     //

  private:
     //static const int NUMBER_OF_SHAPE_FIT_PARAMETERS = 4;
     std::map<int, double[]> pedestal_map_;
     std::map<int, double[]> noise_map_;
     std::map<int, double> gain_map_ ;
     std::map<int, double> offset_map_ ;
     std::map<int, double[]> shape_fit_parameters_map_;
     std::vector<int> bad_channels_;

     int module_id_;             
     int n_channels_;
     std::string name_; 
     static constexpr double sense_pitch_{0.030}; //mm 
     static constexpr double readout_pitch_{0.060};   //mm

     static constexpr double DEFAULT_PEDESTAL_{300.0};
     static constexpr double DEFAULT_NOISE_{10.0};
     static constexpr double DEFAULT_GAIN_{10.0};
     static constexpr double DEFAULT_OFFSET_{0.0};
     

     
     void loadChannelConstants(); 

     
  };
} // namespace tracking::sensor
