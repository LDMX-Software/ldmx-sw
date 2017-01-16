#include "EventProc/EcalDigiProducer.h"

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/Event.h"
#include "Event/EcalHit.h"

using event::SimCalorimeterHit;

const int EcalDigiProducer::numEcalLayers         = 33;
const int EcalDigiProducer::backEcalStartingLayer = 20;
const int EcalDigiProducer::numLayersForMedCal    = 10;
const float EcalDigiProducer::meanNoise           = .015;
const float EcalDigiProducer::readoutThreshold    = 3*meanNoise;

EcalDigiProducer::EcalDigiProducer(const std::string& name, const ldmxsw::Process& process) : ldmxsw::Producer(name,process) { }

void EcalDigiProducer::configure(const ldmxsw::ParameterSet& ps){

    hexReadout = new EcalHexReadout();
    noiseInjector = new TRandom2(0);

    ecalDigis = new TClonesArray("event::EcalHit",10000);
}

void EcalDigiProducer::produce(event::Event& event) {

    // looper over sim hits

    TClonesArray* ecalSimHits = (TClonesArray*) event.getCollection(event::EventConstants::ECAL_SIM_HITS);
    int numEcalSimHits = ecalSimHits->GetEntries();

    std::cout << "[ EcalDigiProducer ] : Got " << numEcalSimHits << " ECal hits in event " << event.getEventHeader()->getEventNumber() << std::endl;

    //First we simulate noise injection into each hit and store layer-wise max cell ids
    int iHitOut=0;
    for(int iHit = 0; iHit < numEcalSimHits; iHit++){
        SimCalorimeterHit* EcalHit = (SimCalorimeterHit*) ecalSimHits->At(iHit);

	double hitNoise = noiseInjector->Gaus(0,meanNoise);
        layer_cell_pair hit_pair = hitToPair(EcalHit);

	event::EcalHit* hit=(event::EcalHit*)(ecalDigis->ConstructedAt(iHit));

	hit->setLayer(hit_pair.first);
	hit->setID(hit_pair.second);
	hit->setEdepSim(EcalHit->getEdep());
	double energy=EcalHit->getEdep()+hitNoise;
	if (energy>readoutThreshold) {
	  hit->setEnergy(energy);
	  hit->setTime(EcalHit->getTime());
	} else {
	  hit->setEnergy(0);
	  hit->setTime(-1000);
	}
	
    }

    event.add("ecalDigis",ecalDigis);
}

DECLARE_PRODUCER(EcalDigiProducer);
