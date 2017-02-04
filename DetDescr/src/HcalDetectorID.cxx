#include "DetDescr/HcalDetectorID.h"

namespace ldmx{

    HcalDetectorID::HcalDetectorID(){
        std::cout << "HcalDetectorID::HcalDetectorID" << std::endl;
        this->getFieldList()->push_back(new IDField("hcalsubdet", 2, 12, 14));
        init();
    };

    HcalDetectorID::HcalDetectorID(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
        num_back_hcal_layers=num_back_hcal_layers_;
        num_wrap_hcal_layers=num_wrap_hcal_layers_;
        HcalDetectorID();
    };

    void HcalDetectorID::dumpInfo(){
        IDField::IDFieldList* fl = getFieldList(); 
        std::cout << "id field list: " << fl << std::endl;
        for (IDField::IDFieldList::iterator it = fl->begin(); it != fl->end(); it++) {
            std::cout << "field name: " << (*it)->getFieldName() << std::endl;
        }
        std::cout << "rawValue:  " << rawValue_ << std::endl;
        for( unsigned int i = 0 ; i < fieldValues_.size() ; i++){
            std::cout << "fieldValue " << i << ": " << fieldValues_.at(i) << std::endl;
        }

    }
    
    void HcalDetectorID::setNumLayers(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
        num_back_hcal_layers=num_back_hcal_layers_;
        num_wrap_hcal_layers=num_wrap_hcal_layers_;
    };

    int HcalDetectorID::getSubDet(){
        if( this->getFieldValue(0) != 6 )
            std::cout << "Error: Hcal DetId not set, this class is not gaurenteed to work." << std::cout;        
        return this->getFieldValue(0);
    };

    int HcalDetectorID::getHcalSubDet(){
        if( this->getFieldValue(0) != 6 )
            std::cout << "Error: Hcal DetId not set, this class is not gaurenteed to work." << std::cout;        
        return this->getFieldValue(2);
    };

    int HcalDetectorID::getLayer(){
        if( this->getFieldValue(0) != 6 )
            std::cout << "Error: Hcal DetId not set, this class is not gaurenteed to work." << std::cout;        
        return this->getFieldValue(1);
    };

    template<> std::map<int,int> HcalDetectorID::getMap(){
        //cache original raw id
        int origRawID = getRawValue();

        std::map<int,int> detid_map;
        setFieldValue(0,6);
        setFieldValue(2,0);
        for(int iLayer = 0 ; iLayer < num_back_hcal_layers ; iLayer++){
            setFieldValue(1,iLayer);
            pack();
            detid_map[getRawValue()] = int(0);
        }
        for(int iSec = 1 ; iSec <= num_wrap_hcal_section; iSec++){
            setFieldValue(2,iSec);
            for(int iLayer = 0 ; iLayer < num_wrap_hcal_layers ; iLayer++){
                setFieldValue(1,iLayer);
                pack();
                detid_map[getRawValue()] = int(0);
            }
        }
        setRawValue(origRawID);
        unpack();
        return detid_map;
    };

    template<> std::map<int,float> HcalDetectorID::getMap(){
        //cache original raw id
        int origRawID = getRawValue();

        std::map<int,float> detid_map;
        setFieldValue(0,6);
        setFieldValue(2,0);
        for(int iLayer = 0 ; iLayer < num_back_hcal_layers ; iLayer++){
            setFieldValue(1,iLayer);
            pack();
            detid_map[getRawValue()] = float(0.);
        }
        for(int iSec = 1 ; iSec <= num_wrap_hcal_section; iSec++){
            setFieldValue(2,iSec);
            for(int iLayer = 0 ; iLayer < num_wrap_hcal_layers ; iLayer++){
                setFieldValue(1,iLayer);
                pack();
                detid_map[getRawValue()] = float(0.);
            }
        }
        setRawValue(origRawID);
        unpack();
        return detid_map;
    };
    
}
