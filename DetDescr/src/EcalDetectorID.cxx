#include "DetDescr/EcalDetectorID.h"

namespace ldmx{

    static const int NUM_ECAL_LAYERS = 33;
    static const int ECAL_SUBDET_ID = 5;
    
    std::map<int,float> EcalDetectorID::getMap(){
        int origRawID = getRawValue();
        
        std::map<int,float> detid_map;
        setFieldValue(0,ECAL_SUBDET_ID); // set subdet bits
        
        for( int iLayer = 0 ; iLayer < NUM_ECAL_LAYERS ; iLayer++){
            setFieldValue(1,iLayer); // set layer bits
            for( int iCell = 0 ; iCell < hexReadout_->getNcells() ; iCell++ ){
                setFieldValue(2,iCell); // set cell bits
                pack();
                detid_map[getRawValue()] = float(0.);
            }
        }
        setRawValue(origRawID);
        unpack();
        return detid_map;
    };
    
}
