#include "SimApplication/RootPrimaryGenerator.h"

// Geant4
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4IonTable.hh"

// LDMX
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "Event/EventConstants.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

namespace ldmx {

    RootPrimaryGenerator::RootPrimaryGenerator(G4String filename) {

        filename_ = filename;
        ifile_ = new TFile(filename_);
        itree_ = (TTree*) ifile_->Get(EventConstants::EVENT_TREE_NAME.c_str());
        eventHeader_ = 0;
        simParticles_ = new TClonesArray(EventConstants::SIM_PARTICLE.c_str());
        ecalSPParticles_ = new TClonesArray(EventConstants::SIM_TRACKER_HIT.c_str());
        itree_->SetBranchAddress(EventConstants::EVENT_HEADER.c_str(), &eventHeader_);
        itree_->SetBranchAddress("SimParticles_sim", &simParticles_);
        itree_->SetBranchAddress("EcalScoringPlaneHits_sim", &ecalSPParticles_);
        evtCtr_ = 0;
        nEvts_ = itree_->GetEntriesFast();
        runMode_ = 0;
    }

    RootPrimaryGenerator::~RootPrimaryGenerator() {
    }

    void RootPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

        if (evtCtr_ >= nEvts_) {
            std::cout << "[ RootPrimaryGenerator ]: End of file reached." << std::endl;
            G4RunManager::GetRunManager()->AbortRun(true);
            anEvent->SetEventAborted();
        }

        itree_->GetEntry(evtCtr_);

        // Mode == 0; regenerate the same events (with the useSeed option toggled on)
        // Mode == 1; generate events from the ecal scoring plane hits

        int theMode = runMode_;

        if (theMode == 1){
            
            // go through ecal SP hits
            for (int iSTH = 0; iSTH < ecalSPParticles_->GetEntriesFast(); ++iSTH) {

                ldmx::SimTrackerHit* sth = (ldmx::SimTrackerHit*) ecalSPParticles_->At(iSTH);

                /*
                 std::cout << iSTH << ", " << sth->getID() << ", " << sth->getLayerID() << ", " << sth->getSimParticle()->getPdgID() << std::endl;
                 std::cout << "\t STH position: " << sth->getPosition()[0] << ", " << sth->getPosition()[1] << ", " << sth->getPosition()[2] << std::endl;
                 std::cout << "\t STH momentum: " << sth->getMomentum()[0] << ", " << sth->getMomentum()[1] << ", " << sth->getMomentum()[2] << std::endl;
                 std::cout << "\t SIM momentum: " << sth->getSimParticle()->getMomentum()[0] << ", " << sth->getSimParticle()->getMomentum()[1] << ", " << sth->getSimParticle()->getMomentum()[2] << std::endl;
                 std::cout << "\t SIM energy  : " << sth->getSimParticle()->getEnergy() << std::endl;
                */   

                /// let's go through the cases when the particle is entering the ECAL volume
                if (sth->getLayerID() == 1 and sth->getMomentum()[2] > 0) continue;
                if (sth->getLayerID() == 2 and sth->getMomentum()[2] < 0) continue; 
                if (sth->getLayerID() == 3 and sth->getMomentum()[1] < 0) continue; 
                if (sth->getLayerID() == 4 and sth->getMomentum()[1] > 0) continue; 
                if (sth->getLayerID() == 5 and sth->getMomentum()[0] > 0) continue; 
                if (sth->getLayerID() == 6 and sth->getMomentum()[0] < 0) continue; 

                G4PrimaryVertex* curvertex = new G4PrimaryVertex();
                curvertex->SetPosition(sth->getPosition()[0]*mm, sth->getPosition()[1]*mm, sth->getPosition()[2]*mm);
                curvertex->SetWeight(1.);

                G4PrimaryParticle* primary = new G4PrimaryParticle();
                primary->SetPDGcode(sth->getSimParticle()->getPdgID());
                primary->SetMomentum(sth->getMomentum()[0] * MeV, sth->getMomentum()[1] * MeV, sth->getMomentum()[2] * MeV);

                UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
                primaryInfo->setHepEvtStatus(1.);
                primary->SetUserInformation(primaryInfo);

                curvertex->SetPrimary(primary);
                anEvent->AddPrimaryVertex(curvertex);

            }   

        }
        else if (theMode == 0){

            // put in protection for if we run out of ROOT events
            std::vector<G4PrimaryVertex*> vertices;
            for (int iSP = 0; iSP < simParticles_->GetEntriesFast(); ++iSP) {

                // check if particle has status 1
                ldmx::SimParticle* sp = (ldmx::SimParticle*) simParticles_->At(iSP);
                if (sp->getGenStatus() != 1)
                    continue;

                bool vertexExists = false;
                G4PrimaryVertex* curvertex = new G4PrimaryVertex();
                for (unsigned int iV = 0; iV < vertices.size(); ++iV) {
                    double cur_vx = sp->getVertex()[0];
                    double cur_vy = sp->getVertex()[1];
                    double cur_vz = sp->getVertex()[2];
                    double i_vx = vertices.at(iV)->GetX0();
                    double i_vy = vertices.at(iV)->GetY0();
                    double i_vz = vertices.at(iV)->GetZ0();
                    if ((cur_vx == i_vx) && (cur_vy == i_vy) && (cur_vz == i_vz)) {
                        vertexExists = true;
                        curvertex = vertices.at(iV);
                    }
                }
                if (vertexExists == false) {
                    curvertex->SetPosition(sp->getVertex()[0], sp->getVertex()[1], sp->getVertex()[2]);
                    curvertex->SetWeight(1.);
                    anEvent->AddPrimaryVertex(curvertex);
                }

                G4PrimaryParticle* primary = new G4PrimaryParticle();
                primary->SetPDGcode(sp->getPdgID());
                primary->SetMomentum(sp->getMomentum()[0] * MeV, sp->getMomentum()[1] * MeV, sp->getMomentum()[2] * MeV);
                primary->SetMass(sp->getMass() * MeV);

                UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
                primaryInfo->setHepEvtStatus(1.);
                primary->SetUserInformation(primaryInfo);

                curvertex->SetPrimary(primary);

            }
        }
        else {
            std::cerr << "Mode value is invalid!" << std::endl;
        }

        std::ofstream tmpout("tmpEvent.rndm");
        std::string eventSeed = eventHeader_->getStringParameter("eventSeed");
        tmpout << eventSeed;
        tmpout.close();

        // move to the next event
        evtCtr_++;

    }

}
