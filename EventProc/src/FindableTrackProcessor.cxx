/**
 * @file FindableTrackProcessor.cxx
 * @brief Processor used to find all particles that pass through the recoil
 *        tracker and leave hits consistent with a findable track.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/FindableTrackProcessor.h"

namespace ldmx { 

    FindableTrackProcessor::FindableTrackProcessor(const std::string &name, const Process &process) :
        Producer(name, process) { 
    }

    FindableTrackProcessor::~FindableTrackProcessor() { 
    }

    void FindableTrackProcessor::configure(const ParameterSet &pset) { 
   
        // Instantiate the container that will hold the results
        findableTrackResults_ = new TClonesArray("ldmx::FindableTrackResult", 10000);
    }

    void FindableTrackProcessor::produce(Event &event) {

        std::cout << "event: " << std::endl; 

        // Get the collection of sim particles from the event 
        const TClonesArray *simParticles = event.getCollection("SimParticles");
        if (simParticles->GetEntriesFast() == 0) return; 

        // Get the collection of Recoil sim hits from the event
        const TClonesArray *recoilSimHits = event.getCollection("RecoilSimHits");  

        // Create the hit map
        this->createHitMap(recoilSimHits); 
        
        // Loop through all sim particles and check which are findable 
        int resultCount = 0;
        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); ++particleCount) { 
            
            // Get the ith particle from the collection of sim particles
            SimParticle* simParticle = static_cast<SimParticle*>(simParticles->At(particleCount));

            // If the sim particle is neutral, skip it.
            if (abs(simParticle->getCharge()) != 1) continue;
    
            // Check if the track is findable 
            if (hitMap_.count(simParticle) == 1) {
                
                FindableTrackResult* findableTrackResult 
                    = (FindableTrackResult*) findableTrackResults_->ConstructedAt(resultCount); 
                findableTrackResult->setResult(simParticle, this->isFindable(hitMap_[simParticle]));
                resultCount++;
            }      
        }

        //Add the result to the collection
        event.add("FindableTracks", findableTrackResults_);
    }

    void FindableTrackProcessor::createHitMap(const TClonesArray* recoilSimHits) { 
       
        // Clear the hit map to remove any previous relations 
        hitMap_.clear();

        // Loop over all recoil tracker sim hits and check which layers, if any,
        // the sim particle deposited energy in.
        std::vector<int> recoilHitCount(10, 0);
        for (int hitCount = 0; hitCount < recoilSimHits->GetEntriesFast(); ++hitCount) {

            // Get the SimTrackerHit from the collection of recoil sim hits.
            SimTrackerHit* recoilSimHit 
                = static_cast<SimTrackerHit*>(recoilSimHits->At(hitCount));
            
            // Get the MC particle associated with this hit
            SimParticle* simParticle = recoilSimHit->getSimParticle(); 

            // If the particle ins't in the hit map, add it.
            if (hitMap_.count(simParticle) == 0) {
                hitMap_[simParticle] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            }
            
            // Increment the hit count for the layer
            hitMap_[simParticle][recoilSimHit->getLayerID() - 1]++;
        }
    }

    bool FindableTrackProcessor::isFindable(std::vector<int> hitCount) { 
        
        // Count how many 3D stereo hits are created by this particle
        double hit3dCount{0};
        for (int layerN = 0; layerN  < 8; layerN += 2) { 
            if (hitCount[layerN]*hitCount[layerN+1] != 0) hit3dCount++; 
        } 

        // A track is considered findable if 
        // 1) The first four stereo layers are hit
        // 2) Three of the first four layers are hit and an axial layer is hit
        // 3) Two of the first four layers are hit and both axial layers are hit
        if (hit3dCount > 3) return true;
        else if (hit3dCount == 3 && (hitCount[8] > 0 || hitCount[9] > 0)) return true;
        else if (hit3dCount == 2 && (hitCount[8] > 0 && hitCount[9] > 0)) return true;
        
        return false; 
    }

}

DECLARE_PRODUCER_NS(ldmx, FindableTrackProcessor) 
