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

int eventproc::HcalDigiProcessor::getLayer( SimCalorimeterHit* hcalHit ){
      
    for(int iLayer = 0; iLayer < numHcalLayers; iLayer++){
        if(hcalHit->getPosition()[2] > hcalLayers[iLayer].first &&
           hcalHit->getPosition()[2] < hcalLayers[iLayer].second )
            return iLayer;
    }
    return -1;
}
  
void eventproc::HcalDigiProcessor::initialize(){

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - Layer numbering scheme - - - - - - - - - -  //
    /////////////////////////////////////////////////////////////////////
    for(int iLayer = 0; iLayer < numHcalLayers; iLayer++){
        hcalLayers[iLayer] = zboundaries(firstLayerZpos+iLayer*layerZwidth-10.,firstLayerZpos+iLayer*layerZwidth+10.);
    }
  
    //////////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - output tree setup - - - - - - - - - - - - -  //
    //////////////////////////////////////////////////////////////////////////
    outputFile = new TFile(outputFileName,"RECREATE");
    outputTree = new TTree("hcalDigi","hcalDigi");
  
    hcalDetId_ = new std::vector<int>(numHcalLayers,-1);
    hcalLayerNum_ = new std::vector<int>(numHcalLayers,-1);
    hcalLayerPEs_ = new std::vector<int>(numHcalLayers,0);
    hcalLayerEdep_ = new std::vector<float>(numHcalLayers,0.);
    hcalLayerTime_ = new std::vector<float>(numHcalLayers,-1);
    hcalLayerZpos_ = new std::vector<float>(numHcalLayers,-1);
  
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
    TClonesArray* hcalHits = getEvent()->getCollection(event::HCAL_SIM_HITS);
    int numHCalSimHits = hcalHits->GetEntries();
    for(int iHit = 0; iHit < numHCalSimHits; iHit++){
        SimCalorimeterHit* hcalHit = (SimCalorimeterHit*) hcalHits->At(iHit);
        int detID = hcalHit->getID();
        if(hcalLayerEdep.find(hcalHit->getID()) == hcalLayerEdep.end()){
            // first hit, initialize 
            hcalLayerEdep[detID]=hcalHit->getEdep();	
            hcalLayerTime[detID]=hcalHit->getTime()*hcalHit->getEdep();	
            hcalDetId[detID]=detID;
            hcalZpos[detID]=hcalHit->getPosition()[2];
            hcalLayerNum[detID]=getLayer(hcalHit);
        }else{
            // not first hit, aggregate 
            hcalLayerEdep[detID]+=hcalHit->getEdep();	
            hcalLayerTime[detID]+=hcalHit->getTime()*hcalHit->getEdep();	
        }
    }// end loop over sim hits

  
    hcalDetId_->assign(numHcalLayers,-1);
    hcalLayerNum_->assign(numHcalLayers,-1);
    hcalLayerPEs_->assign(numHcalLayers,-1);
    hcalLayerEdep_->assign(numHcalLayers,-1.);
    hcalLayerTime_->assign(numHcalLayers,-1);
    hcalLayerZpos_->assign(numHcalLayers,-1);

    // loop over detID (layers) and simulate number of PEs
    for(std::map<int,float>::iterator it = hcalLayerEdep.begin(); it != hcalLayerEdep.end(); ++it){    
        int detID = it->first;
        depEnergy = hcalLayerEdep[detID];
        hcalLayerTime[detID] = hcalLayerTime[detID]/hcalLayerEdep[detID];
        meanPE = depEnergy/MeVperMIP*PEperMIP+meanNoise;
    
        std::default_random_engine generator;
        std::poisson_distribution<int> distribution(meanPE);
    
        hcalLayerPEs[detID] = distribution(generator);
    
        if(hcalLayerNum[detID] < 0 || hcalLayerNum[detID] >= numHcalLayers){
            std::cerr << "Layer calculation error.  Please check geometry definition." << std::endl;
            exit(1);
        }else{
            int layer = hcalLayerNum[detID];
            hcalLayerNum_->at(layer) = layer;
            hcalDetId_->at(layer) = detID;
            hcalLayerPEs_->at(layer) = hcalLayerPEs[detID];
            hcalLayerEdep_->at(layer) = hcalLayerEdep[detID];
            hcalLayerTime_->at(layer) = hcalLayerTime[detID];
            hcalLayerZpos_->at(layer) = hcalZpos[detID];
        }	 
        if(verbose){
            std::cout << "detID: " << detID << std::endl;
            std::cout << "Layer: " << hcalLayerNum[detID] << std::endl;
            std::cout << "Edep: " << hcalLayerEdep[detID] << std::endl;
            std::cout << "numPEs: " << hcalLayerPEs[detID] << std::endl;
            std::cout << "time: " << hcalLayerTime[detID] << std::endl;
            std::cout << "z: " << hcalZpos[detID] << std::endl;
        }// end verbose 
    }// end loop over detIDs (layers)
    outputTree->Fill();
}
  
void eventproc::HcalDigiProcessor::finish(){
    outputFile->cd();
    outputTree->Write();
    outputFile->Close();
  
    delete hcalLayerNum_;
    delete hcalDetId_;
    delete hcalLayerPEs_;
    delete hcalLayerEdep_;
    delete hcalLayerTime_;
    delete hcalLayerZpos_;
  
}
