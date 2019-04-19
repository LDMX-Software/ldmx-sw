/**
 * @file TrackerVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Recoil Tracker. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/TrackerVetoProcessor.h"

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"
#include "TVector3.h"

//----------//
//   LDMX   //
//----------//
#include "Event/FindableTrackResult.h"
#include "Event/SimParticle.h" 
#include "Event/SimTrackerHit.h"
#include "Event/TrackerVetoResult.h"
#include "Tools/AnalysisUtils.h"

namespace ldmx {

    TrackerVetoProcessor::TrackerVetoProcessor(const std::string &name, Process &process) : 
            Producer(name, process) {
    }
    
    TrackerVetoProcessor::~TrackerVetoProcessor() { 
    }

    void TrackerVetoProcessor::configure(const ParameterSet& pSet) {
    }

    void TrackerVetoProcessor::produce(Event& event) {

        // Get the collection of simulated particles from the event
        const TClonesArray* particles = event.getCollection("SimParticles");
      
        // Search for the recoil electron 
        const SimParticle* recoil = Analysis::searchForRecoil(particles); 

        // Find the target scoring plane hit associated with the recoil
        // electron and extract the momentum
        double p{-1}, pt{-1}, px{-9999}, py{-9999}, pz{-9999}; 
        SimTrackerHit* spHit{nullptr}; 
        if (event.exists("TargetScoringPlaneHits")) { 
            
            // Get the collection of simulated particles from the event
            const TClonesArray* spHits = event.getCollection("TargetScoringPlaneHits");
            
            //
            for (size_t iHit{0}; iHit < spHits->GetEntriesFast(); ++iHit) { 
                SimTrackerHit* hit = static_cast<SimTrackerHit*>(spHits->At(iHit)); 
                if ((hit->getSimParticle() == recoil) && (hit->getLayerID() == 2)
                        && (hit->getMomentum()[2] > 0)) {
                    spHit = hit;
                    break; 
                }
            }

            if (spHit != nullptr) {
                TVector3 recoilP(spHit->getMomentum().data()); 
        
                p = recoilP.Mag(); 
                pt = recoilP.Pt(); 
                px = recoilP.Px();
                py = recoilP.Py(); 
                pz = recoilP.Pz();  
            }
        }

        bool recoilIsFindable{false};
        TrackMaps map;  
        if (event.exists("FindableTracks")) { 
            // Get the collection of simulated particles from the event
            const TClonesArray* tracks 
                = event.getCollection("FindableTracks");

            map = Analysis::getFindableTrackMaps(tracks);
            
            auto it = map.findable.find(recoil);
            if ( it != map.findable.end()) recoilIsFindable = true; 
        }

        bool passesTrackVeto{false}; 
        if ((map.findable.size() == 1) && recoilIsFindable && (p < 1200)) passesTrackVeto = true; 


        TrackerVetoResult result; 
        result.setVetoResult(passesTrackVeto);

        if (passesTrackVeto) { 
            setStorageHint(hint_shouldKeep); 
        } else { 
            setStorageHint(hint_shouldDrop); 
        } 

        event.addToCollection("TrackerVeto", result);
    }
}

DECLARE_PRODUCER_NS(ldmx, TrackerVetoProcessor);
