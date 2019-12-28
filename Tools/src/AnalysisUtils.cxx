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

        const SimParticle* getRecoil(const std::map<int,SimParticle> &particleMap) {
            
            for ( const auto &sp : particleMap ) {
                if (
                    (sp.second.getPdgID() == 11) //check that the particle is an electron
                    and (sp.second.getGenStatus() == 1) //check that it was generated "originally" (is a primary according to Geant4)
                   ) { return &(sp.second); }
            }//loop through particle map

            return nullptr;
        }

        const SimParticle* getPNGamma(const std::map<int,SimParticle> &particleMap) {
            
            for ( const auto &sp : particleMap ) {
                
               if ( (sp.second.getPdgID() == 0 ) //particle is a photon
                    and (sp.second.getDaughterCount() > 0) //has offspring
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

            const SimParticle *recoilElectron = Analysis::getRecoil( particleMap );

            if ( recoilElectron ) {
            
               std::vector<int> daughterTrackIDs = recoilElectron->getDaughters();
               for ( const int &dID : daughterTrackIDs ) {
                    
                    if ( particleMap.count(dID) > 0 /*daughter exists in particleMap*/) { 
                        std::vector<int> grandDaughterTrackIDs = particleMap.at(dID).getDaughters();
                        for ( const int &gdID : grandDaughterTrackIDs ) {
                            if ( particleMap.count(gdID) > 0 /*granddaughter exists in particleMap*/ and
                                 particleMap.at(gdID).getProcessType() == SimParticle::ProcessType::photonNuclear /*granddaughter came from PN */
                               ) { return &(particleMap.at(gdID)); }
                        } //loop through grand daughter track IDs 
                    } //daughter exists in particle map

               } //loop through daughter track IDs

            } //found recoil electron
            
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
