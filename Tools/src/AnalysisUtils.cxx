/**
 * @file AnalysisUtils.cxx
 * @brief Collection of utility functions useful for analysis
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Tools/AnalysisUtils.h"

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <string>
#include <tuple>

//----------//
//   ldmx   //
//----------//
#include "Event/FindableTrackResult.h"
#include "Event/SimParticle.h"
#include "Exception/Exception.h"

//----------//
//   ROOT   //
//----------//
#include "TVector3.h"

namespace ldmx {

    namespace Analysis {

        std::tuple<int, const SimParticle*> getRecoil(const std::map<int,SimParticle> &particleMap) {
            
            // The recoil electron always has a track ID of 1.  
            return {1, &(particleMap.at(1))};
        }

        const SimParticle* getPNGamma(const std::map< int, SimParticle >& particleMap, 
                const SimParticle* recoil, const float& energyThreshold) {
           
            // Get all of the daughter track IDs
            auto daughterTrackIDs{recoil->getDaughters()};

            auto pit = std::find_if(daughterTrackIDs.begin(), daughterTrackIDs.end(), 
                    [energyThreshold, particleMap] (const int& id) {
                    
                // Get the SimParticle from the map
                auto daughter{particleMap.at(id)};

                // If the particle doesn't have any daughters, return false
                if (daughter.getDaughters().size() == 0) return false;

                // If the particle has daughters that were a result of a 
                // photo-nuclear reaction, and its energy is above threshold, 
                // then tag it as the PN gamma. 
                return ((particleMap.at(daughter.getDaughters().front()).getProcessType() 
                            == SimParticle::ProcessType::photonNuclear) 
                        && (daughter.getEnergy() >= energyThreshold));
            });
             
            // If a PN daughter was found, return it.   
            if (pit != daughterTrackIDs.end()) return &particleMap.at(*pit); 

            return nullptr;
        } 

        TrackMaps getFindableTrackMaps(const std::vector<FindableTrackResult> &tracks) { 
       
            TrackMaps map; 

            for (const FindableTrackResult &track : tracks ) {
                if (track.is4sFindable() || track.is3s1aFindable() || track.is2s2aFindable()) { 
                    map.findable[track.getParticleTrackID()] =  &track; 
                }

                if (track.is2sFindable()) map.loose[track.getParticleTrackID()] = &track;
            
                if (track.is2aFindable()) map.axial[track.getParticleTrackID()] = &track;    
            }

            return map;  
        }
    
    } // Analysis    

} // ldmx
