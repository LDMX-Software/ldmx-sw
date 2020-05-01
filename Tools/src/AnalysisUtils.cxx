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

        const SimParticle* getPNGamma(const std::map<int,SimParticle> &particleMap) {
            
            for ( const auto &sp : particleMap ) {
                
               if ( (sp.second.getPdgID() == 0 ) //particle is a photon
                    and (sp.second.getDaughters().size() > 0) //has offspring
                  ) {

                   std::vector<int> daughterTrackIDs = sp.second.getDaughters();
                   for ( const int &trackID : daughterTrackIDs ) {
                        
                        if ( particleMap.count(trackID) > 0 and //daughter exists in particleMap
                             particleMap.at(trackID).getProcessType() == SimParticle::ProcessType::photonNuclear //daughter came from PN
                           ) { return &(sp.second); }

                   } //loop through daughter track IDs

               } //current particle is a photon and has offspring
            } //loop through particle map

            //no particles satisfy
            //  1) Is a photon
            //  2) Has a daughter with origin process PN
            return nullptr;
        }

        const SimParticle* getRecoilPNGamma(const std::map<int,SimParticle> &particleMap) {

            // Search for and rretrieve the recoil electron 
            auto [trackID, recoilElectron] = Analysis::getRecoil(particleMap);

            // If the recoil electron was found, continue to search for the 
            // PN gamma. If not, return null. 
            if ( recoilElectron ) {
            
               auto daughterTrackIDs{recoilElectron->getDaughters()};

               auto pit = std::find_if(daughterTrackIDs.begin(), daughterTrackIDs.end(), [particleMap] (const int& id) {
                       
                            // If the particle doesn't have any daughters, return false
                            if (particleMap.at(id).getDaughters().size() == 0) return false;

                            // If the particle has daughters that were a result
                            // of a photo-nuclear reaction, then tag it as the 
                            // PN gamma. 
                            return (particleMap.at(particleMap.at(id).getDaughters().front()).getProcessType() 
                                    == SimParticle::ProcessType::photonNuclear); 
                            });
             
                // If a PN daughter was found, return it.   
                if (pit != daughterTrackIDs.end()) return &particleMap.at(*pit); 

            } 
            
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
