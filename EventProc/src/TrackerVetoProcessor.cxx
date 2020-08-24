/**
 * @file TrackerVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Recoil Tracker. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/TrackerVetoProcessor.h"

//----------//
//   ROOT   //
//----------//
#include "TVector3.h"

//----------//
//   LDMX   //
//----------//
#include "Event/FindableTrackResult.h"
#include "SimCore/SimParticle.h" 
#include "SimCore/SimTrackerHit.h"
#include "Event/TrackerVetoResult.h"
#include "Tools/AnalysisUtils.h"

namespace ldmx {

    TrackerVetoProcessor::TrackerVetoProcessor(const std::string &name, Process &process) : 
            Producer(name, process) { }
    
    TrackerVetoProcessor::~TrackerVetoProcessor() { }

    void TrackerVetoProcessor::produce(Event& event) {

        // Get the collection of simulated particles from the event
        auto particleMap{event.getMap< int, SimParticle >("SimParticles")};

        // Search for the recoil electron 
        auto [recoilTrackID, recoilElectron] = Analysis::getRecoil(particleMap);
      
        // Find the target scoring plane hit associated with the recoil
        // electron and extract the momentum
        double p{-1}, pt{-1}, px{-9999}, py{-9999}, pz{-9999}; 
        const SimTrackerHit *recoilTrackerHit = nullptr;
        if (event.exists("TargetScoringPlaneHits")) { 
            
            // Get the collection of simulated particles from the event
            const std::vector<SimTrackerHit> spHits = event.getCollection<SimTrackerHit>("TargetScoringPlaneHits");
            
            for (const SimTrackerHit &hit : spHits ) {
                if (
                    (hit.getTrackID() == recoilTrackID) /*hit blamed on recoil*/ and
                    (hit.getLayerID() == 2) /*hit in outgoing layer*/ and
                    (hit.getMomentum()[2] > 0) /*particle leaving target*/
                   ) {
                    recoilTrackerHit = &hit;
                    break; 
                }
            }//check all target scoring plane hits

            if ( recoilTrackerHit ) {
                TVector3 recoilP(recoilTrackerHit->getMomentum().data()); 
        
                p  = recoilP.Mag(); 
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
            const std::vector<FindableTrackResult> tracks = event.getCollection<FindableTrackResult>("FindableTracks");

            map = Analysis::getFindableTrackMaps(tracks);
            
            auto it = map.findable.find(recoilTrackID);
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

        event.add("TrackerVeto", result);
    }
}

DECLARE_PRODUCER_NS(ldmx, TrackerVetoProcessor);
