/**
 * @file HcalDigiProducer.h
 * @brief Class that performs digitization of simulated HCal data
 * @author Andrew Whitbeck, FNAL
 */

#include "TString.h"
#include "TRandom.h"

#include "DetDescr/DetectorID.h"
#include "DetDescr/DefaultDetectorID.h"

using detdescr::DetectorID;
using detdescr::DefaultDetectorID;

#include "Framework/EventProcessor.h"
#include "Event/SimCalorimeterHit.h"


/**
 * @class HcalDigiProducer
 * @brief Performs digitization of simulated HCal data
 */
  class HcalDigiProducer : public ldmxsw::Producer {
  
public:
    typedef int layer;
    typedef std::pair<double,double> zboundaries;
  
    HcalDigiProducer(const std::string& name, const ldmxsw::Process& process);
    virtual ~HcalDigiProducer() { delete hits; if (random_) delete random_;}
    
    virtual void configure(const ldmxsw::ParameterSet&);
    virtual void produce(event::Event& event);      
   
 private:
    TClonesArray* hits;
    TRandom* random_{0};
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


