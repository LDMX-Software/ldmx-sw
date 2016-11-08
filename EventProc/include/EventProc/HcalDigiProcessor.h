#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"

#include "Event/SimEvent.h"

using event::SimEvent;
using event::SimCalorimeterHit;;

#include "EventProc/EventProcessor.h"
#include "Event/SimCalorimeterHit.h"

namespace eventproc {
  
typedef int layer;
typedef std::pair<double,double> zboundaries;
 
class HcalDigiProcessor : public EventProcessor {
  
public:
  
    HcalDigiProcessor(TString outputFileName_,bool verbose_=false):outputFileName(outputFileName_),verbose(verbose_){};
  
    int getLayer( SimCalorimeterHit* hcalHit ); 
    void initialize();
    void execute();      
    void finish();
   
 private:
  
    TFile* outputFile;
    TTree* outputTree;
    TString outputFileName;
    std::vector<int> *hcalDetId_,*hcalLayerNum_,*hcalLayerPEs_;
    std::vector<float> *hcalLayerEdep_,*hcalLayerTime_,*hcalLayerZpos_;
    std::map<layer,zboundaries> hcalLayers;
    bool verbose;  

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
