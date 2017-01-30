#include <map>

namespace ldmx {

enum SubDet {kBack_hcal,kWrap_top_hcal,kWrap_bot_hcal,kWrap_left_hcal,kWrap_right_hcal};

/**
 * @class HcalDetIdHelper
 * @brief parameterizes DetIds versus layer and subdetector
 */
class HcalDetId{
    private:
        int num_back_hcal_layers{0};
        int num_wrap_hcal_layers{0};
        static const int wrap_hcal_start_detid{64};
        static const int num_wrap_hcal_section{4};
    
    public:
        HcalDetId(){};
        HcalDetId(int num_back_hcal_layers_, int num_wrap_hcal_layers_);
        void setNumLayers(int num_back_hcal_layers_, int num_wrap_hcal_layers_);
        // currently 'hard coding' detid parameterization 
        // to conform with copynumbers in hcal.gdml -- we'll
        // need to be vigilant about keeping these in synch 
        // until we have a more robust method -- A. Whitbeck
        int getDetId(SubDet subdet,int layer);
        // same statements made above apply here -- A. Whitbeck
        int getLayer(int detid);
        // same statements made above apply here -- A. Whitbeck
        int getSubDet(int detid);
        template<typename T> std::map<int,T> getMap();
};

}
