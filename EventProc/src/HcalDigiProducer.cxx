#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/EventConstants.h"
#include "Event/Event.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/HcalHit.h"
#include "EventProc/HcalDigiProducer.h"
#include "Framework/ParameterSet.h"

#include <iostream>

using event::SimCalorimeterHit;;

#include "Event/SimCalorimeterHit.h"

const float HcalDigiProducer::firstLayerZpos = 569.5;
const float HcalDigiProducer::layerZwidth = 60.;
const int HcalDigiProducer::numHcalLayers = 15;
const float HcalDigiProducer::MeVperMIP = 1.40;
const float HcalDigiProducer::PEperMIP = 13.5*6./4.;


HcalDigiProducer::HcalDigiProducer(const std::string& name, ldmxsw::Process& process) : ldmxsw::Producer(name,process) {
  hits=new TClonesArray("event::HcalHit");
}

void HcalDigiProducer::configure(const ldmxsw::ParameterSet& ps) {
    detID = new DefaultDetectorID();
    random_=new TRandom(ps.getInteger("randomSeed",1000));
    meanNoise_=ps.getDouble("meanNoise");
}
  
void HcalDigiProducer::produce(event::Event& event){
  
    std::map<int,int> hcalLayerNum,hcalLayerPEs,hcalDetId;
    std::map<int,float> hcalZpos;
    std::map<int,float> hcalLayerEdep,hcalLayerTime;

    // looper over sim hits and aggregate energy depositions for each detID
    TClonesArray* hcalHits = (TClonesArray*) event.getCollection(event::EventConstants::HCAL_SIM_HITS, "sim");

    int numHCalSimHits = hcalHits->GetEntries();
    for(int iHit = 0; iHit < numHCalSimHits; iHit++){
        SimCalorimeterHit* simHit = (SimCalorimeterHit*) hcalHits->At(iHit);
        //int detID=simHit->getID();
        int detIDraw=simHit->getID();
        if(verbose) std::cout << "detIDraw: " << detIDraw << std::endl;
        detID->setRawValue(detIDraw);
        detID->unpack();
        int layer=detID->getFieldValue("layer"); 
        if(verbose) std::cout << "layer: " << layer << std::endl;
        if(hcalLayerEdep.find(simHit->getID()) == hcalLayerEdep.end()){
            // first hit, initialize 
            hcalLayerEdep[detIDraw]=simHit->getEdep();	
            hcalLayerTime[detIDraw]=simHit->getTime()*simHit->getEdep();	
            hcalDetId[detIDraw]=detIDraw;
            hcalZpos[detIDraw]=simHit->getPosition()[2];
            hcalLayerNum[detIDraw]=layer;
        }else{
            // not first hit, aggregate 
            hcalLayerEdep[detIDraw]+=simHit->getEdep();	
            hcalLayerTime[detIDraw]+=simHit->getTime()*simHit->getEdep();	
        }
    }// end loop over sim hits
  
    // loop over detID (layers) and simulate number of PEs
    int ihit=0;
    for(std::map<int,float>::iterator it = hcalLayerEdep.begin(); it != hcalLayerEdep.end(); ++it){    
        int detIDraw = it->first;
        double depEnergy = hcalLayerEdep[detIDraw];
        hcalLayerTime[detIDraw] = hcalLayerTime[detIDraw]/hcalLayerEdep[detIDraw];
        double meanPE = depEnergy/MeVperMIP*PEperMIP;
    
	//        std::default_random_engine generator;
        //std::poisson_distribution<int> distribution(meanPE);
    
        hcalLayerPEs[detIDraw] = random_->Poisson(meanPE);
	hcalLayerPEs[detIDraw] += random_->Gaus(meanNoise_);

        if(verbose){
            std::cout << "detID: " << detIDraw << std::endl;
            std::cout << "Layer: " << hcalLayerNum[detIDraw] << std::endl;
            std::cout << "Edep: " << hcalLayerEdep[detIDraw] << std::endl;
            std::cout << "numPEs: " << hcalLayerPEs[detIDraw] << std::endl;
            std::cout << "time: " << hcalLayerTime[detIDraw] << std::endl;
            std::cout << "z: " << hcalZpos[detIDraw] << std::endl;
        }// end verbose 
        
        int layer = hcalLayerNum[detIDraw];
	double energy=hcalLayerPEs[detIDraw]/PEperMIP*MeVperMIP; // need to add in a weighting factor eventually

	event::HcalHit *hit = (event::HcalHit*)(hits->ConstructedAt(ihit));
	
	//	hit->setLayer(layer);
	hit->setPE(hcalLayerPEs[detIDraw]);
	hit->setAmplitude(hcalLayerPEs[detIDraw]);
	hit->setEnergy(energy);
	hit->setTime(hcalLayerTime[detIDraw]);
        hit->setID(detIDraw);
	//	hit->setZpos(hcalZpos[detIDraw]);
	ihit++;
    }// end loop over detIDs (layers)
    // put it into the event
    event.add("hcalDigis",hits);
}


DECLARE_PRODUCER(HcalDigiProducer);

