/**
 * @file EcalVetoProcessor.h
 * @brief Class that performs basic ECal digi and determines if an event is vetoable
 * @author Owen Colegrove, UCSB
 */

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom2.h"

#include "Event/SimEvent.h"
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "EventProc/EventProcessor.h"

using event::SimEvent;
using event::SimCalorimeterHit;
using detdescr::DetectorID;
using detdescr::EcalDetectorID;
using detdescr::EcalHexReadout;

namespace eventproc {

/**
 * @class EcalVetoProcessor
 * @brief Performs basic ECal digi and determines if event is vetoable
 */
class EcalVetoProcessor : public EventProcessor {

public:
    typedef std::pair<int, int>   layer_cell_pair;

    typedef std::pair<int, float> cell_energy_pair;

    EcalVetoProcessor(bool verbose_ = false) :
        verbose(verbose_){};

    void initialize();

    void execute();

    void finish();

 private:

    TTree* outputTree;
    TRandom2 *noiseInjector;
    std::vector<float> *EcalLayerEdepRaw_,*EcalLayerEdepReadout_,
    *EcalLayerTime_,*EcalLayerIsoRaw_,*EcalLayerIsoReadout_,
    *EcalHitId_,*EcalHitLayer_,*EcalHitDep_,*EcalHitNoise_;
    bool verbose,doesPassVeto;
    EcalDetectorID* detID;
    EcalHexReadout* hexReadout;
    static const int numEcalLayers,numLayersForMedCal,backEcalStartingLayer;
    static const float meanNoise,readoutThreshold,
    totalDepCut,totalIsoCut,backEcalCut,ratioCut;

    inline layer_cell_pair hitToPair(SimCalorimeterHit* hit){
        int detIDraw = hit->getID();
        detID->setRawValue(detIDraw);
        detID->unpack();
        int layer = detID->getFieldValue("layer");
        int cellid = detID->getFieldValue("cellid");
        return (std::make_pair(layer, cellid));
    };

};

}
