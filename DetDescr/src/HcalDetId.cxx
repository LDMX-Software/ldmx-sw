#include "DetDescr/HcalDetId.h"

namespace ldmx{

    HcalDetId::HcalDetId(){
        std::cout << "HcalDetId::HcalDetId" << std::endl;
        IDField::IDFieldList* fieldList = new IDField::IDFieldList();
        fieldList->push_back(new IDField("subdet", 0, 0, 3));
        fieldList->push_back(new IDField("hcalsubdet", 1, 4, 5));
        fieldList->push_back(new IDField("layer", 2, 6, 13));
        setFieldList(fieldList);
        dumpInfo();
    };

    HcalDetId::HcalDetId(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
        num_back_hcal_layers=num_back_hcal_layers_;
        num_wrap_hcal_layers=num_wrap_hcal_layers_;
        HcalDetId();
    };

    void HcalDetId::dumpInfo(){
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
    
    void HcalDetId::setNumLayers(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
        num_back_hcal_layers=num_back_hcal_layers_;
        num_wrap_hcal_layers=num_wrap_hcal_layers_;
    };

    int HcalDetId::getSubDet(){
        if( this->getFieldValue(0) != 6 )
            std::cout << "Error: Hcal DetId not set, this class is not gaurenteed to work." << std::cout;        
        return this->getFieldValue(0);
    };

    int HcalDetId::getHcalSubDet(){
        if( this->getFieldValue(0) != 6 )
            std::cout << "Error: Hcal DetId not set, this class is not gaurenteed to work." << std::cout;        
        return this->getFieldValue(1);
    };

    int HcalDetId::getLayer(){
        if( this->getFieldValue(0) != 6 )
            std::cout << "Error: Hcal DetId not set, this class is not gaurenteed to work." << std::cout;        
        return this->getFieldValue(2);
    };

    template<> std::map<int,int> HcalDetId::getMap(){

        std::cout << "HcalDetId::getMap" << std::endl;
        
        //cache original raw id
        int origRawID = getRawValue();
        dumpInfo();

        std::map<int,int> detid_map;
        std::cout << "initialized map" << std::endl;
        
        std::cout << "number of fields: " << fieldValues_.size() << std::endl;

        setFieldValue(0,6);
        std::cout << "set first field value" << std::endl;
        setFieldValue(1,0);
        std::cout << "set second field value" << std::endl;
        std::cout << "back hcal map: " << std::endl;
        for(int iLayer = 0 ; iLayer < num_back_hcal_layers ; iLayer++){
            std::cout << "layer: " << iLayer << std::endl;
            setFieldValue(2,iLayer);
            pack();
            detid_map[getRawValue()] = int(0);
        }
        for(int iSec = 1 ; iSec <= num_wrap_hcal_section; iSec++){
            setFieldValue(1,iSec);
            for(int iLayer = 0 ; iLayer < num_wrap_hcal_layers ; iLayer++){
                setFieldValue(2,iLayer);
                pack();
                detid_map[getRawValue()] = int(0);
            }
        }
        setRawValue(origRawID);
        unpack();
        return detid_map;
    };

    template<> std::map<int,float> HcalDetId::getMap(){
        //cache original raw id
        int origRawID = getRawValue();

        std::map<int,float> detid_map;
        setFieldValue(0,6);
        setFieldValue(1,0);
        for(int iLayer = 0 ; iLayer < num_back_hcal_layers ; iLayer++){
            setFieldValue(2,iLayer);
            pack();
            detid_map[getRawValue()] = float(0.);
        }
        for(int iSec = 1 ; iSec <= num_wrap_hcal_section; iSec++){
            setFieldValue(1,iSec);
            for(int iLayer = 0 ; iLayer < num_wrap_hcal_layers ; iLayer++){
                setFieldValue(2,iLayer);
                pack();
                detid_map[getRawValue()] = float(0.);
            }
        }
        setRawValue(origRawID);
        unpack();
        return detid_map;
    };
    
}
