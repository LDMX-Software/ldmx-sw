#include "EventProc/EcalDigiProducer.h"

//----------//
//   ROOT   //
//----------//
#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/EventConstants.h"
#include "Event/EcalHit.h"

namespace ldmx {

    const std::vector<double> LAYER_WEIGHTS 
        = {1.641, 3.526, 5.184, 6.841,
        8.222, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775,
        8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 12.642, 16.51,
        16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 8.45}; 
   
    const double ELECTRONS_PER_MIP = 33000.0; // e-

    const double MIP_SI_RESPONSE = 0.130; // MeV

    //const float EcalDigiProducer::meanNoise           = .015;
    //const float EcalDigiProducer::readoutThreshold    = 3*meanNoise;

    EcalDigiProducer::EcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
    }

    void EcalDigiProducer::configure(const ParameterSet& ps) {

        hexReadout_ = new EcalHexReadout();
        noiseInjector_ = new TRandom2(ps.getInteger("randomSeed", 0));
        meanNoise_ = ps.getDouble("meanNoise");
        readoutThreshold_ = ps.getDouble("readoutThreshold");
        padCapacitance_ = ps.getDouble("padCapacitance");  

        ecalDigis_ = new TClonesArray(EventConstants::ECAL_HIT.c_str(), 10000);
    }

    void EcalDigiProducer::produce(Event& event) {

        TClonesArray* ecalSimHits = (TClonesArray*) event.getCollection(EventConstants::ECAL_SIM_HITS);
        int numEcalSimHits = ecalSimHits->GetEntries();

        std::cout << "[ EcalDigiProducer ] : Got " << numEcalSimHits 
                  << " ECal hits in event " << event.getEventHeader()->getEventNumber()
                  << std::endl;

        //First we simulate noise injection into each hit and store layer-wise max cell ids
        for (int iHit = 0; iHit < numEcalSimHits; iHit++) {
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) ecalSimHits->At(iHit);

            double hitNoise = noiseInjector_->Gaus(0, meanNoise_);
            layer_cell_pair hit_pair = hitToPair(simHit);

            EcalHit* digiHit = (EcalHit*) (ecalDigis_->ConstructedAt(iHit));

            //  hit->setLayer(hit_pair.first);
            digiHit->setID(simHit->getID());
            digiHit->setAmplitude(simHit->getEdep());
            double energy = simHit->getEdep() + hitNoise;
            if (energy > readoutThreshold_) {
            
                digiHit->setEnergy(((energy/MIP_SI_RESPONSE)*LAYER_WEIGHTS[hit_pair.first]+energy)*0.948);
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
