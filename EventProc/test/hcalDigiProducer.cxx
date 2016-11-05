#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"

#include "Event/SimEvent.h"
#include "EventProc/EventLoop.h"
#include "EventProc/RootEventSource.h"

using event::SimEvent;
using event::SimCalorimeterHit;;
using eventproc::EventLoop;
using eventproc::RootEventSource;

#include "EventProc/EventProcessor.h"
#include "Event/SimCalorimeterHit.h"

namespace eventproc {

  typedef int layer;
  typedef std::pair<double,double> zboundaries;
  std::map<layer,zboundaries> hcalLayers;
  const int firstLayerZpos = 569.5;
  const int layerZwidth = 60.;
  const int numHcalLayers = 15;

  class HcalDigiProcessor : public EventProcessor {

  public:

    HcalDigiProcessor(TString outputFileName_):outputFileName(outputFileName_){
      //std::cout << "HcalDigiProcessor::HcalDigiProcessor" << std::endl;
    };

    int getLayer( SimCalorimeterHit* hcalHit ){
      
      for( int iLayer = 0 ; iLayer < numHcalLayers ; iLayer++ ){
	if( hcalHit->getPosition()[2] > hcalLayers[iLayer].first &&
	    hcalHit->getPosition()[2] < hcalLayers[iLayer].second )
	  return iLayer;
      }
      return -1;
    }

    void initialize(){

      //std::cout << "HcalDigiProcessor::initialize" << std::endl;
      
       /////////////////////////////////////////////////////////////////////
       // - - - - - - - - - - Layer numbering scheme - - - - - - - - - -  //
       /////////////////////////////////////////////////////////////////////
      for( int iLayer = 0 ; iLayer < numHcalLayers ; iLayer++ ){
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

    };

    void execute(){
      
      //std::cout << "HcalDigiProcessor::execute" << std::endl;
      
      int numHCalSimHits = getEvent()->getCollection(event::HCAL_SIM_HITS)->GetEntries();

      std::map<int,int> hcalLayerNum,hcalLayerPEs,hcalDetId;
      std::map<int,float> hcalZpos;
      std::map<int,float> hcalLayerEdep,hcalLayerTime;

      for( int iHit = 0 ; iHit < numHCalSimHits ; iHit++ ){
	SimCalorimeterHit* hcalHit = (SimCalorimeterHit*) getEvent()->getCollection(event::HCAL_SIM_HITS)->At(iHit);
	int detID = hcalHit->getID();
	if( hcalLayerEdep.find(hcalHit->getID()) == hcalLayerEdep.end() ){
	  hcalLayerEdep[detID]=hcalHit->getEdep();	
	  hcalLayerTime[detID]=hcalHit->getTime()*hcalHit->getEdep();	
	  hcalDetId[detID]=detID;
	  hcalZpos[detID]=hcalHit->getPosition()[2];
	  hcalLayerNum[detID]=getLayer(hcalHit);
	}else{
	  hcalLayerEdep[detID]+=hcalHit->getEdep();	
	  hcalLayerTime[detID]+=hcalHit->getTime()*hcalHit->getEdep();	
	}
	//hcalHit->Print();
	//std::cout << "Edep: " << hcalHit->getEdep() << std::endl;
      }


      float MeVperMIP = 1.40;
      float PEperMIP = 13.5*6./4.;
      float meanNoise = 2.;
      float depEnergy;
      float meanPE;

      hcalDetId_->assign(numHcalLayers,-1);
      hcalLayerNum_->assign(numHcalLayers,-1);
      hcalLayerPEs_->assign(numHcalLayers,-1);
      hcalLayerEdep_->assign(numHcalLayers,-1.);
      hcalLayerTime_->assign(numHcalLayers,-1);
      hcalLayerZpos_->assign(numHcalLayers,-1);

      for( std::map<int,float>::iterator it = hcalLayerEdep.begin() ; 
	   it != hcalLayerEdep.end() ; ++it ){

	int detID = it->first;
	depEnergy = hcalLayerEdep[detID];
	hcalLayerTime[detID] = hcalLayerTime[detID]/hcalLayerEdep[detID];
	meanPE = depEnergy/MeVperMIP*PEperMIP+meanNoise;

	std::default_random_engine generator;
	std::poisson_distribution<int> distribution(meanPE);

	hcalLayerPEs[detID] = distribution(generator);

	if( hcalLayerNum[detID] < 0 || hcalLayerNum[detID] >= numHcalLayers ){
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
	/*
	  std::cout << "detID: " << detID << std::endl;
	  std::cout << "Layer: " << hcalLayerNum[detID] << std::endl;
	  std::cout << "Edep: " << hcalLayerEdep[detID] << std::endl;
	  std::cout << "numPEs: " << hcalLayerPEs[detID] << std::endl;
	  std::cout << "time: " << hcalLayerTime[detID] << std::endl;
	  std::cout << "z: " << hcalZpos[detID] << std::endl;
	*/
      }
      outputTree->Fill();
    };

    void finish(){
      outputFile->cd();
      outputTree->Write();
      outputFile->Close();

      delete hcalLayerNum_;
      delete hcalDetId_;
      delete hcalLayerPEs_;
      delete hcalLayerEdep_;
      delete hcalLayerTime_;
      delete hcalLayerZpos_;

    };

  private:
    TFile* outputFile;
    TTree* outputTree;
    TString outputFileName;
    std::vector<int> *hcalDetId_,*hcalLayerNum_,*hcalLayerPEs_;
    std::vector<float> *hcalLayerEdep_,*hcalLayerTime_,*hcalLayerZpos_;

    int nProcessed_{0};
  };

}


int main(int argc, const char* argv[])  {

    if (argc < 3) {
      std::cerr << "Wrong number of inputs - (1) input file (2) output file " << std::endl;
      exit(1);
    }

    std::list<std::string> fileList;

    std::cout << "Adding file " << argv[1] << std::endl;
    fileList.push_back(argv[1]);
    
    RootEventSource* src = new RootEventSource(fileList, new SimEvent());
    EventLoop* loop = new EventLoop();
    loop->setEventSource(src);
    loop->addEventProcessor(new eventproc::HcalDigiProcessor(argv[2]));
    loop->initialize();
    loop->run(-1);
    loop->finish();
}
