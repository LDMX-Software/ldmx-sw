/**
 * @file EcalDigiProducer.h
 * @brief Class that performs basic ECal digi
 * @author Owen Colegrove, UCSB
 */

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom2.h"
#include "TClonesArray.h"

#include "Event/SimCalorimeterHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"

using event::SimCalorimeterHit;
using detdescr::DetectorID;
using detdescr::EcalDetectorID;
using detdescr::EcalHexReadout;


/**
 * @class EcalDigiProducer
 * @brief Performs basic ECal digi and determines if event is vetoable
 */
  class EcalDigiProducer : public ldmxsw::Producer {

public:
    typedef std::pair<int, int>   layer_cell_pair;

    typedef std::pair<int, float> cell_energy_pair;

    EcalDigiProducer(const std::string& name, ldmxsw::Process& process);

    virtual void configure(const ldmxsw::ParameterSet&);
    virtual void produce(event::Event& event);      

    //    void finish();

 private:

    TRandom2 *noiseInjector;
    TClonesArray* ecalDigis;
    EcalDetectorID detID;
    EcalHexReadout* hexReadout;
    static const int numEcalLayers,numLayersForMedCal,backEcalStartingLayer;
    float meanNoise_,readoutThreshold_;

    inline layer_cell_pair hitToPair(SimCalorimeterHit* hit){
        int detIDraw = hit->getID();
        detID.setRawValue(detIDraw);
        detID.unpack();
        int layer = detID.getFieldValue("layer");
        int cellid = detID.getFieldValue("cell");
        return (std::make_pair(layer, cellid));
    };

};


