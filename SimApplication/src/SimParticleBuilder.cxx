#include "SimApplication/SimParticleBuilder.h"

// LDMX
#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/G4TrackerHit.h"
#include "SimApplication/UserTrackingAction.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4VTrajectoryPoint.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"

namespace ldmx {

    SimParticleBuilder::SimParticleBuilder() :
            currentEvent_(nullptr) {
        trackMap_ = UserTrackingAction::getUserTrackingAction()->getTrackMap();
        outputParticleColl_ = new TClonesArray(EventConstants::SIM_PARTICLE.c_str(), 50);
    }

    SimParticleBuilder::~SimParticleBuilder() {
        delete outputParticleColl_;
    }

    void SimParticleBuilder::buildSimParticles(ldmx::Event* outputEvent) {

        // Clear the output particle collection.
        outputParticleColl_->Clear("C");
	
	// Get the trajectory container for the event.
        TrajectoryContainer* trajectories = (TrajectoryContainer*) (const_cast<G4Event*>(currentEvent_))->GetTrajectoryContainer();

        // Create empty SimParticle objects and create the map of track ID to particles.
        buildParticleMap(trajectories, outputParticleColl_);

        // Fill information into the particles.
        if (trajectories) {
	   for (auto trajectory : *trajectories->GetVector()) {
              buildSimParticle(static_cast<Trajectory*>(trajectory));
           }
	}

	// Start by saving the generated particles into SimParticles
	saveGeneratedParticle(outputParticleColl_);

std::cout<<outputParticleColl_->GetEntries()<<std::endl;

        // Add the collection data to the output event.
        outputEvent->add("SimParticles", outputParticleColl_);
    }

    void SimParticleBuilder::buildSimParticle(Trajectory* traj) {

        SimParticle* simParticle = particleMap_[traj->GetTrackID()];

        if (!simParticle) {
            std::cerr << "[ SimParticleBuilder ] : SimParticle not found for Trajectory with track ID " << traj->GetTrackID() << std::endl;
            G4Exception("SimParticleBuilder::buildSimParticle", "", FatalException, "SimParticle not found for Trajectory.");
        }

        simParticle->setGenStatus(traj->getGenStatus());
        simParticle->setPdgID(traj->GetPDGEncoding());
        simParticle->setCharge(traj->GetCharge());
        simParticle->setMass(traj->getMass());
        simParticle->setEnergy(traj->getEnergy());
        simParticle->setTime(traj->getGlobalTime());
        simParticle->setProcessType(traj->getProcessType());

        const G4ThreeVector& vertex = traj->getVertexPosition();
        simParticle->setVertex(vertex[0], vertex[1], vertex[2]);

        const G4ThreeVector& momentum = traj->GetInitialMomentum();
        simParticle->setMomentum(momentum[0], momentum[1], momentum[2]);

        const G4ThreeVector& endpMomentum = traj->getEndPointMomentum();
        simParticle->setEndPointMomentum(endpMomentum[0], endpMomentum[1], endpMomentum[2]);

        G4ThreeVector endpoint = traj->getEndPoint();
        simParticle->setEndPoint(endpoint[0], endpoint[1], endpoint[2]);

        if (traj->GetParentID() > 0) {
            SimParticle* parent = findSimParticle(traj->GetParentID());
            if (parent != nullptr) {
                simParticle->addParent(parent);
                parent->addDaughter(simParticle);
            } else {
                // If the parent particle can not be found by its track ID, this is a fatal error!
                std::cerr << "[ SimParticleBuilder ] : ERROR - SimParticle with parent ID " << traj->GetParentID() << " not found for track ID " << traj->GetTrackID() << std::endl;
                G4Exception("SimParticleBuilder::buildSimParticle", "", FatalException, "SimParticle not found from parent track ID.");
            }
        }
    }
    
    void SimParticleBuilder::saveGeneratedParticle(TClonesArray* simParticleColl) {
        
        for (int iSim = 0; iSim < simParticleColl->GetEntries(); iSim++)
	{ 
            SimParticle* sim = (SimParticle*) simParticleColl->At(iSim);
	    if (sim->getGenStatus()==1) return;	
	}    
	
	for (int iv=0; iv<currentEvent_->GetNumberOfPrimaryVertex();++iv) { 	

	   G4PrimaryVertex* primaryVertex = currentEvent_->GetPrimaryVertex(iv);
	   for (int i=0; i<primaryVertex->GetNumberOfParticle();++i) { 	

	      G4PrimaryParticle* primaryParticle = primaryVertex->GetPrimary(i);       

	      SimParticle* simParticle = (SimParticle*) simParticleColl->ConstructedAt(simParticleColl->GetEntries());
	      simParticle->setGenStatus(1);
	      simParticle->setPdgID(primaryParticle->GetPDGcode());
              simParticle->setCharge(primaryParticle->GetCharge());
              simParticle->setMass(primaryParticle->GetMass());
              simParticle->setEnergy(primaryParticle->GetTotalEnergy());
	      simParticle->setTime(primaryVertex->GetT0());
              simParticle->setProcessType(0); //set process to unknown for now
              simParticle->setVertex(primaryVertex->GetX0(), primaryVertex->GetY0(), primaryVertex->GetZ0());
              simParticle->setMomentum(primaryParticle->GetPx(),primaryParticle->GetPy(),primaryParticle->GetPz());
              //not sure what to set those to
              simParticle->setEndPointMomentum(0,0,0); 
              simParticle->setEndPoint(0,0,0);           
	   }
	}
    }

    void SimParticleBuilder::buildParticleMap(TrajectoryContainer* trajectories, TClonesArray* simParticleColl) {
        particleMap_.clear();
        if (trajectories==nullptr) return;
	for (auto trajectory : *trajectories->GetVector()) {
            particleMap_[trajectory->GetTrackID()] = (SimParticle*) simParticleColl->ConstructedAt(simParticleColl->GetEntries());
        }
    }

    SimParticle* SimParticleBuilder::findSimParticle(G4int trackID) {
        G4VTrajectory* traj = trackMap_->findTrajectory(trackID);
        if (traj != nullptr) {
            return particleMap_[traj->GetTrackID()];
        } else {
            return nullptr;
        }
    }

}
