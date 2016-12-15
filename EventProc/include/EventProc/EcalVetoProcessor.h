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

typedef std::pair<int, int>   layer_cell_pair;

typedef std::pair<int, float> cell_energy_pair;

namespace eventproc {

class EcalVetoProcessor : public EventProcessor {

    public:

        /** Constuctor */
        EcalVetoProcessor(TTree* outputTree);

        /** Destructor */
        ~EcalVetoProcessor(); 

        void initialize();

        void execute();

        void finish();

    private:

        EcalDetectorID* detID{new EcalDetectorID};
        EcalHexReadout* hexReadout{new EcalHexReadout};
        TTree* outputTree_{nullptr};
        TRandom2 *noiseInjector{new TRandom2{0}};

        static constexpr int numEcalLayers{40};

        // TODO: Most of these should be settable and not constant.
        static const float meanNoise;
        static const float readoutThreshold;
        static const float totalDepCut;
        static const float totalIsoCut;
        static const float backEcalCut; 
        static const float ratioCut;
        static const int numLayersForMedCal;
        static const int backEcalStartingLayer;

        std::vector<float> ecalLayerEdepRaw_{numEcalLayers, 0.0};
        std::vector<float> ecalLayerEdepReadout_{numEcalLayers, 0.0};
        std::vector<float> ecalLayerTime_{numEcalLayers, 0.0};
        std::vector<float> ecalLayerIsoRaw_{numEcalLayers, 0.0};
        std::vector<float> ecalLayerIsoReadout_{numEcalLayers, 0.0};

        std::vector<float> ecalHitId_;
        std::vector<float> ecalHitLayer_;
        std::vector<float> ecalHitDep_;
        std::vector<float> ecalHitNoise_;

        bool verbose; 
        bool doesPassVeto;


        inline layer_cell_pair hitToPair(SimCalorimeterHit* hit){
            int detIDraw = hit->getID();
            detID->setRawValue(detIDraw);
            detID->unpack();
            int layer = detID->getFieldValue("layer");
            int cellid = detID->getFieldValue("cell");
            return (std::make_pair(layer, cellid));
        };

};

}
