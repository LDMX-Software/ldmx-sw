#pragma once

#include <vector>
#include <map>


namespace tracking::sensor {

    class RawTrackerHit{
      public: 
        RawTrackerHit(int channel_id, int module_id, double[] adc_counts); 
        int getModuleId(return module_id_;); 
        int getChannelId(return channel_id_;); 

      private:
        int channel_id_; 
        int module_id_; 
        double[] adc_counts_;        
    }
}
