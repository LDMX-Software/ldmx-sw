/**
 * @file HcalDigiProcessor.h
 * @brief Class that performs digitization of simulated HCal data
 * @author Andrew Whitbeck, FNAL
 */

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"

#include "Event/SimEvent.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/DefaultDetectorID.h"

using event::SimEvent;
using event::SimCalorimeterHit;
using detdescr::DetectorID;
using detdescr::DefaultDetectorID;

#include "EventProc/EventProcessor.h"
#include "Event/SimCalorimeterHit.h"

namespace eventproc {
  
typedef int layer;
typedef std::pair<double,double> zboundaries;

/**
 * @class HcalDigiProcessor
 * @brief Performs digitization of simulated HCal data
 */
class HcalDigiProcessor : public EventProcessor {
  
public:
  
    HcalDigiProcessor(TTree* outputTree_,bool verbose_=false):outputTree(outputTree_),verbose(verbose_){};
  
    void initialize();
    void execute();      
    void finish();
   
 private:
  
    TTree* outputTree;

    std::vector<int> *hcalDetId_,*hcalLayerNum_,*hcalLayerPEs_;
    std::vector<float> *hcalLayerEdep_,*hcalLayerTime_,*hcalLayerZpos_;
    std::map<layer,zboundaries> hcalLayers;
    bool verbose;  
    DetectorID* detID;
    static const float firstLayerZpos;
    static const float layerZwidth;
    static const int numHcalLayers;
    static const float MeVperMIP;
    static const float PEperMIP;
    static const float meanNoise;
    float depEnergy;
    float meanPE;
    int nProcessed_{0};
  
};

}
