#include "SimApplication/RootPrimaryGenerator.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4IonTable.hh"

// LDMX
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "Event/SimParticle.h"
#include "Event/EventConstants.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

namespace ldmx {

    RootPrimaryGenerator::RootPrimaryGenerator( G4String filename ) {
        
        filename_ = filename;
        ifile_ = new TFile( filename_ );
        itree_ = (TTree*) ifile_->Get(EventConstants::EVENT_TREE_NAME.c_str());
	eventHeader_ = 0;
        simParticles_ = new TClonesArray(EventConstants::SIM_PARTICLE.c_str());
        itree_->SetBranchAddress(EventConstants::EVENT_HEADER.c_str(), &eventHeader_);
        itree_->SetBranchAddress("SimParticles_sim", &simParticles_);
        evtCtr_ = 0;
        nEvts_ = itree_->GetEntriesFast();
    }

    RootPrimaryGenerator::~RootPrimaryGenerator() {
    }

    void RootPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        std::cout << "Reading next Root event ..." << std::endl;
        if (evtCtr_ > nEvts_) G4RunManager::GetRunManager()->AbortEvent();

        itree_->GetEntry(evtCtr_);

        // put in protection for if we run out of ROOT events
        std::vector< G4PrimaryVertex* > vertices;
        for (int iSP = 0; iSP < simParticles_->GetEntriesFast(); ++iSP) {

            // check if particle has status 1
            ldmx::SimParticle* sp = (ldmx::SimParticle*) simParticles_->At(iSP);
            if (sp->getGenStatus() != 1) continue;

            bool vertexExists = false;
            G4PrimaryVertex* curvertex = new G4PrimaryVertex();         
            for (unsigned int iV = 0; iV < vertices.size(); ++iV){
                double cur_vx = sp->getVertex()[0];
                double cur_vy = sp->getVertex()[1];
                double cur_vz = sp->getVertex()[2];
                double i_vx = vertices.at(iV)->GetX0();
                double i_vy = vertices.at(iV)->GetY0();
                double i_vz = vertices.at(iV)->GetZ0();
                if ((cur_vx == i_vx)&&(cur_vy == i_vy)&&(cur_vz == i_vz)){ 
                    vertexExists = true;
                    curvertex = vertices.at(iV);
                }
            }
            if (vertexExists == false){
                curvertex->SetPosition(sp->getVertex()[0], sp->getVertex()[1], sp->getVertex()[2]);
                curvertex->SetWeight(1.);
                anEvent->AddPrimaryVertex(curvertex);
            }

            G4PrimaryParticle* primary = new G4PrimaryParticle();
            primary->SetPDGcode( sp->getPdgID() );
            primary->SetMomentum( sp->getMomentum()[0] * MeV, sp->getMomentum()[1] * MeV, sp->getMomentum()[2] * MeV);
            primary->SetMass( sp->getMass() * MeV);

            UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
            primaryInfo->setHepEvtStatus(1.);
            primary->SetUserInformation(primaryInfo);

            curvertex->SetPrimary(primary);

        }

        std::ofstream tmpout("tmpEvent.rndm");
        std::string eventSeed = eventHeader_->getStringParameter("eventSeed");
        tmpout << eventSeed;
        tmpout.close();

        // ..... validation ..... 
        // int nPV = anEvent->GetNumberOfPrimaryVertex();
        // for (int iPV = 0; iPV < nPV; ++iPV){
        //     G4PrimaryVertex* curPV =  anEvent->GetPrimaryVertex(iPV);
        //     int nPar = curPV->GetNumberOfParticle();            
        //     for (int iPar = 0; iPar < nPar; ++iPar){
        //         std::cout << "pv/primary = " << iPV << "," << iPar << std::endl;
        //     }
        // }

        // move to the next event
        evtCtr_++;

    }

}
