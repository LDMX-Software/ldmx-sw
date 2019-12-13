#include "SimApplication/SimParticleBuilder.h"

#include <string>

// LDMX
#include "Exception/Exception.h"
#include "Framework/Event.h"
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

namespace ldmx {

    SimParticleBuilder::SimParticleBuilder() :
            currentEvent_(nullptr) {
        trackMap_ = UserTrackingAction::getUserTrackingAction()->getTrackMap();
    }

    SimParticleBuilder::~SimParticleBuilder() { }

    void SimParticleBuilder::buildSimParticles(ldmx::Event* outputEvent) {

        // Get the trajectory container for the event.
        TrajectoryContainer* trajectories = (TrajectoryContainer*) (const_cast<G4Event*>(currentEvent_))->GetTrajectoryContainer();
        std::cout << "buildParticleMap" << std::endl;
        // Create empty SimParticle objects and create the map of track ID to particles.
        std::vector<SimParticle> outputParticleColl;
        buildParticleMap(trajectories, outputParticleColl );

        std::cout << "buildSimParticle" << std::endl;
        // Fill information into the particles.
        for (auto trajectory : *trajectories->GetVector()) {
            buildSimParticle(static_cast<Trajectory*>(trajectory));
        }

        std::cout << "event->add" << std::endl;
        // Add the collection data to the output event.
        outputEvent->add("SimParticles", outputParticleColl );
    }

    void SimParticleBuilder::buildSimParticle(Trajectory* traj) {
         
        std::cout << "particleMap_[trackID] " << traj << "\t";
        SimParticle* simParticle = particleMap_[traj->GetTrackID()];

//        if (!simParticle) {
//            EXCEPTION_RAISE( "MissingInfo" , "SimParticle not found for Trajectory with track ID '" 
//                    + std::to_string(traj->GetTrackID()) + "'." );
//        }

        std::cout << "simParticle->set(\t";
        simParticle->setGenStatus(traj->getGenStatus());
        simParticle->setTrackID(traj->GetTrackID());
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
            std::cout << "findSimParticle\t";
            SimParticle* parent = findSimParticle(traj->GetParentID());
            std::cout << "addParent\t";
            simParticle->addParent( traj->GetParentID() );
            auto parentParticle = particleMap_.find( traj->GetParentID() );
            if ( parentParticle != particleMap_.end() ) {
                //this parent has been found in particleMap_
                std::cout << "addDaughter\t";
                particleMap_[ traj->GetParentID() ]->addDaughter( simParticle->getTrackID() );
            }//check if parent exists
        }//check if particle has a parent
        std::cout << std::endl;
    }

    void SimParticleBuilder::buildParticleMap(TrajectoryContainer* trajectories, std::vector<SimParticle> &simParticleColl) {
        particleMap_.clear();
        for (auto trajectory : *trajectories->GetVector()) {
            simParticleColl.emplace_back();
            particleMap_[trajectory->GetTrackID()] = &(simParticleColl.back());
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
