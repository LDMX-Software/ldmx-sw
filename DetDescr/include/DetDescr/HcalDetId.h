// STL
#include <map>
#include <iostream>

// LDMX
#include "DetDescr/IDField.h"
#include "DetDescr/DetectorID.h"

namespace ldmx {

enum SubDet {kBack_hcal,kWrap_top_hcal,kWrap_bot_hcal,kWrap_left_hcal,kWrap_right_hcal};

/**
 * @class HcalDetIdHelper
 * @brief parameterizes DetIds versus layer and subdetector
 */
class HcalDetId : public DetectorID {
    private:
        int num_back_hcal_layers{0};
        int num_wrap_hcal_layers{0};
        static const int wrap_hcal_start_detid{64};
        static const int num_wrap_hcal_section{4};
    
    public:
        HcalDetId();
        HcalDetId(int num_back_hcal_layers_, int num_wrap_hcal_layers_);
        void setNumLayers(int num_back_hcal_layers_, int num_wrap_hcal_layers_);
        void dumpInfo();
        int getSubDet();
        int getHcalSubDet();
        int getLayer();
        template<typename T> std::map<int,T> getMap();
};

}
