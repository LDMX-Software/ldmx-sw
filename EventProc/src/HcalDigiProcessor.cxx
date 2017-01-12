#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "Event/SimEvent.h"
#include "EventProc/EventLoop.h"
#include "EventProc/RootEventSource.h"
#include "EventProc/HcalDigiProcessor.h"

using event::SimEvent;
using event::SimCalorimeterHit;;
using eventproc::EventLoop;
using eventproc::RootEventSource;

#include "EventProc/EventProcessor.h"
#include "Event/SimCalorimeterHit.h"

const float eventproc::HcalDigiProcessor::firstLayerZpos = 569.5;
const float eventproc::HcalDigiProcessor::layerZwidth = 60.;
const int eventproc::HcalDigiProcessor::numHcalLayers = 15;
const float eventproc::HcalDigiProcessor::MeVperMIP = 1.40;
const float eventproc::HcalDigiProcessor::PEperMIP = 13.5*6./4.;
const float eventproc::HcalDigiProcessor::meanNoise = 2.;

void eventproc::HcalDigiProcessor::initialize(){
    detID = new DefaultDetectorID();

    //////////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - output tree setup - - - - - - - - - - - - -  //
    //////////////////////////////////////////////////////////////////////////

    hcalDetId_ = new std::vector<int>();
    hcalLayerNum_ = new std::vector<int>();
    hcalLayerPEs_ = new std::vector<int>();
    hcalLayerEdep_ = new std::vector<float>();
    hcalLayerTime_ = new std::vector<float>();
    hcalLayerZpos_ = new std::vector<float>();

    outputTree->Branch("hcalDetId",&(hcalDetId_));
    outputTree->Branch("hcalLayerNum",&(hcalLayerNum_));
    outputTree->Branch("hcalLayerPEs",&(hcalLayerPEs_));
    outputTree->Branch("hcalLayerEdep",&(hcalLayerEdep_));
    outputTree->Branch("hcalLayerTime",&(hcalLayerTime_));
    outputTree->Branch("hcalLayerZpos",&(hcalLayerZpos_));


}
  
void eventproc::HcalDigiProcessor::execute(){
  
    std::map<int,int> hcalLayerNum,hcalLayerPEs,hcalDetId;
    std::map<int,float> hcalZpos;
    std::map<int,float> hcalLayerEdep,hcalLayerTime;

    // looper over sim hits and aggregate energy depositions for each detID
    TClonesArray* hcalHits = (TClonesArray*) getEvent()->get(event::EventConstants::HCAL_SIM_HITS, "recon");
    int numHCalSimHits = hcalHits->GetEntries();
    for(int iHit = 0; iHit < numHCalSimHits; iHit++){
        SimCalorimeterHit* hcalHit = (SimCalorimeterHit*) hcalHits->At(iHit);
        //int detID=hcalHit->getID();
        int detIDraw=hcalHit->getID();
        if(verbose) std::cout << "detIDraw: " << detIDraw << std::endl;
        detID->setRawValue(detIDraw);
        detID->unpack();
        int layer=detID->getFieldValue("layer"); 
        if(verbose) std::cout << "layer: " << layer << std::endl;
        if(hcalLayerEdep.find(hcalHit->getID()) == hcalLayerEdep.end()){
            // first hit, initialize 
            hcalLayerEdep[detIDraw]=hcalHit->getEdep();	
            hcalLayerTime[detIDraw]=hcalHit->getTime()*hcalHit->getEdep();	
            hcalDetId[detIDraw]=detIDraw;
            hcalZpos[detIDraw]=hcalHit->getPosition()[2];
            hcalLayerNum[detIDraw]=layer;
        }else{
            // not first hit, aggregate 
            hcalLayerEdep[detIDraw]+=hcalHit->getEdep();	
            hcalLayerTime[detIDraw]+=hcalHit->getTime()*hcalHit->getEdep();	
        }
    }// end loop over sim hits
  
    hcalDetId_->clear();
    hcalLayerNum_->clear();
    hcalLayerPEs_->clear();
    hcalLayerEdep_->clear();
    hcalLayerTime_->clear();
    hcalLayerZpos_->clear();

    // loop over detID (layers) and simulate number of PEs
    for(std::map<int,float>::iterator it = hcalLayerEdep.begin(); it != hcalLayerEdep.end(); ++it){    
        int detIDraw = it->first;
        depEnergy = hcalLayerEdep[detIDraw];
        hcalLayerTime[detIDraw] = hcalLayerTime[detIDraw]/hcalLayerEdep[detIDraw];
        meanPE = depEnergy/MeVperMIP*PEperMIP+meanNoise;
    
        std::default_random_engine generator;
        std::poisson_distribution<int> distribution(meanPE);
    
        hcalLayerPEs[detIDraw] = distribution(generator);

        if(verbose){
            std::cout << "detID: " << detIDraw << std::endl;
            std::cout << "Layer: " << hcalLayerNum[detIDraw] << std::endl;
            std::cout << "Edep: " << hcalLayerEdep[detIDraw] << std::endl;
            std::cout << "numPEs: " << hcalLayerPEs[detIDraw] << std::endl;
            std::cout << "time: " << hcalLayerTime[detIDraw] << std::endl;
            std::cout << "z: " << hcalZpos[detIDraw] << std::endl;
        }// end verbose 
        
        int layer = hcalLayerNum[detIDraw];
        hcalLayerNum_->push_back(layer);
        hcalDetId_->push_back(detIDraw);
        hcalLayerPEs_->push_back(hcalLayerPEs[detIDraw]);
        hcalLayerEdep_->push_back(hcalLayerEdep[detIDraw]);
        hcalLayerTime_->push_back(hcalLayerTime[detIDraw]);
        hcalLayerZpos_->push_back(hcalZpos[detIDraw]);

    }// end loop over detIDs (layers)
    outputTree->Fill();
}
  
void eventproc::HcalDigiProcessor::finish(){  
    delete hcalLayerNum_;
    delete hcalDetId_;
    delete hcalLayerPEs_;
    delete hcalLayerEdep_;
    delete hcalLayerTime_;
    delete hcalLayerZpos_;
}
