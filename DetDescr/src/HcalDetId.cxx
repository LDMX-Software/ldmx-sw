#include "DetDescr/HcalDetId.h"

namespace ldmx{

    HcalDetId::HcalDetId(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
        num_back_hcal_layers=num_back_hcal_layers_;
        num_wrap_hcal_layers=num_wrap_hcal_layers_;
    };

    void HcalDetId::setNumLayers(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
        num_back_hcal_layers=num_back_hcal_layers_;
        num_wrap_hcal_layers=num_wrap_hcal_layers_;
    };

    int HcalDetId::getDetId(SubDet subdet,int layer){
        if( subdet == 0 )
            return layer;
        else{
            return layer*num_wrap_hcal_section+wrap_hcal_start_detid+int(subdet);
        }
    };

    int HcalDetId::getLayer(int detid){
        if( detid < 64 ){
            return detid;
        }else{
            return (detid-64)/num_wrap_hcal_section;
        }
    };

    int HcalDetId::getSubDet(int detid){
        if( detid < 64 ){
            return 0;
        }else{
            return detid%num_wrap_hcal_section;
        }
    };

    template<> std::map<int,int> HcalDetId::getMap(){
        std::map<int,int> detid_map;
        for(int iLayer = 0 ; iLayer < num_back_hcal_layers ; iLayer++){
            detid_map[getDetId(kBack_hcal,iLayer)] = int(0);
        }
        for(int iSec = 1 ; iSec <= num_wrap_hcal_section; iSec++){
            for(int iLayer = 0 ; iLayer < num_wrap_hcal_layers ; iLayer++){
                detid_map[getDetId(SubDet(iSec),iLayer)] = int(0);
            }
        }
        return detid_map;
    };

    template<> std::map<int,float> HcalDetId::getMap(){
        std::map<int,float> detid_map;
        for(int iLayer = 0 ; iLayer < num_back_hcal_layers ; iLayer++){
            detid_map[getDetId(kBack_hcal,iLayer)] = float(0);
        }
        for(int iSec = 1 ; iSec <= num_wrap_hcal_section; iSec++){
            for(int iLayer = 0 ; iLayer < num_wrap_hcal_layers ; iLayer++){
                detid_map[getDetId(SubDet(iSec),iLayer)] = float(0);
            }
        }
        return detid_map;
    };
    
}
