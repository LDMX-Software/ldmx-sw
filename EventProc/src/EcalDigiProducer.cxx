#include "EventProc/EcalDigiProducer.h"

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/Event.h"
#include "Event/EcalHit.h"
#include "Framework/ParameterSet.h"

namespace ldmx {

const int EcalDigiProducer::NUM_ECAL_LAYERS = 33;
const int EcalDigiProducer::BACK_ECAL_STARTING_LAYER = 20;
const int EcalDigiProducer::NUM_LAYERS_FOR_MED_CAL = 10;
//const float EcalDigiProducer::meanNoise           = .015;
//const float EcalDigiProducer::readoutThreshold    = 3*meanNoise;

EcalDigiProducer::EcalDigiProducer(const std::string& name, const Process& process) :
        Producer(name, process) {
}

void EcalDigiProducer::configure(const ParameterSet& ps) {

    hexReadout_ = new EcalHexReadout();
    noiseInjector_ = new TRandom2(ps.getInteger("randomSeed", 0));
    meanNoise_ = ps.getDouble("meanNoise");
    readoutThreshold_ = ps.getDouble("readoutThreshold");

    ecalDigis_ = new TClonesArray(EventConstants::ECAL_HIT.c_str(), 10000);
}

void EcalDigiProducer::produce(Event& event) {

    TClonesArray* ecalSimHits = (TClonesArray*) event.getCollection(EventConstants::ECAL_SIM_HITS);
    int numEcalSimHits = ecalSimHits->GetEntries();

    std::cout << "[ EcalDigiProducer ] : Got " << numEcalSimHits << " ECal hits in event " << event.getEventHeader()->getEventNumber() << std::endl;

    //First we simulate noise injection into each hit and store layer-wise max cell ids
    int iHitOut = 0;
    for (int iHit = 0; iHit < numEcalSimHits; iHit++) {
        SimCalorimeterHit* simHit = (SimCalorimeterHit*) ecalSimHits->At(iHit);

        double hitNoise = noiseInjector_->Gaus(0, meanNoise_);
        layer_cell_pair hit_pair = hitToPair(simHit);

        EcalHit* digiHit = (EcalHit*) (ecalDigis_->ConstructedAt(iHit));

        //	hit->setLayer(hit_pair.first);
        digiHit->setID(simHit->getID());
        digiHit->setAmplitude(simHit->getEdep());
        double energy = simHit->getEdep() + hitNoise;
        if (energy > readoutThreshold_) {
            digiHit->setEnergy(energy);
            digiHit->setTime(simHit->getTime());
        } else {
            digiHit->setEnergy(0);
            digiHit->setTime(-1000);
        }
    }

    event.add("ecalDigis", ecalDigis_);
}

}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
