/**
 * @file HcalDigiProducer.h
 * @brief Class that performs digitization of simulated HCal data
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENTPROC_HCALDIGIPRODUCER_H_
#define EVENTPROC_HCALDIGIPRODUCER_H_

// ROOT
#include "TString.h"
#include "TRandom.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "Event/SimCalorimeterHit.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    enum SubDet {kBack_hcal,kWrap_top_hcal,kWrap_bot_hcal,kWrap_left_hcal,kWrap_right_hcal};

/**
 * @class HcalDetIdHelper
 * @brief parameterizes DetIds versus layer and subdetector
 */
class HcalDetId{
    public:
        int num_back_hcal_layers{0};
        int num_wrap_hcal_layers{0};
        static const int wrap_hcal_start_detid{64};
        static const int num_wrap_hcal_section{4};

        HcalDetId();

        HcalDetId(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
            num_back_hcal_layers=num_back_hcal_layers_;
            num_wrap_hcal_layers=num_wrap_hcal_layers_;
        };

        void setNumLayers(int num_back_hcal_layers_, int num_wrap_hcal_layers_){
            num_back_hcal_layers=num_back_hcal_layers_;
            num_wrap_hcal_layers=num_wrap_hcal_layers_;
        };

        // currently 'hard coding' detid parameterization 
        // to conform with copynumbers in hcal.gdml -- we'll
        // need to be vigilant about keeping these in synch 
        // until we have a more robust method -- A. Whitbeck
        int getDetId(SubDet subdet,int layer){
            if( subdet == 0 )
                return layer;
            else{
                return layer*num_wrap_hcal_section+wrap_hcal_start_detid+int(subdet);
            }
        };

        // same statements made above apply here -- A. Whitbeck
        int getLayer(int detid){
            if( detid < 64 ){
                return detid;
            }else{
                return (detid-64)/num_wrap_hcal_section;
            }
        };

        // same statements made above apply here -- A. Whitbeck
        int getSubDet(int detid){
            if( detid < 64 ){
                return 0;
            }else{
                return detid%num_wrap_hcal_section;
            }
        };
          
        template<class T> std::map<int,T> getMap(){
            std::map<int,T> detid_map;
            for(int iLayer = 0 ; iLayer < num_back_hcal_layers ; iLayer++){
                detid_map[getDetId(kBack_hcal,iLayer)] = T(0);
            }
            for(int iSec = 1 ; iSec <= num_wrap_hcal_section; iSec++){
                for(int iLayer = 0 ; iLayer < num_wrap_hcal_layers ; iLayer++){
                    detid_map[getDetId(SubDet(iSec),iLayer)] = T(0);
                }
            }
            return detid_map;
        };
};

/**
 * @class HcalDigiProducer
 * @brief Performs digitization of simulated HCal data
 */
class HcalDigiProducer : public Producer {

    public:

        typedef int layer;

        typedef std::pair<double, double> zboundaries;

        HcalDigiProducer(const std::string& name, const Process& process);

        virtual ~HcalDigiProducer() {
            delete hits_;
            if (random_)
                delete random_;
        }

        virtual void configure(const ParameterSet&);

        virtual void produce(Event& event);

    private:

        TClonesArray* hits_{nullptr};
        TRandom* random_{0};
        std::map<layer, zboundaries> hcalLayers_;
        bool verbose_{false};
        DetectorID* detID_{nullptr};
        
        static const int NUM_HCAL_LAYERS;
        float meanNoise_{0};
        float mev_per_mip_{0};
        float pe_per_mip_{0};
        //int num_back_hcal_layers_{0};
        //int num_wrap_hcal_layers_{0};
        HcalDetId hcalDetIds;
        int nProcessed_{0};
};

}

#endif
