/**
 * @file AnalysisUtils.cxx
 * @brief Collection of utility functions useful for analysis
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Tools/AnalysisUtils.h"

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <stdexcept>

//----------//
//   ldmx   //
//----------//
#include "Event/FindableTrackResult.h"
#include "Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

namespace ldmx {

    namespace Analysis {

        const SimParticle* searchForRecoil(const TClonesArray* particles, const int index) { 

            // Check that the index is within the bounds of the array. If not, 
            // throw an exception.
            if (index == particles->GetEntriesFast()) 
                throw std::out_of_range("Index is beyond the size of the TClonesArray."); 

            const SimParticle* particle = static_cast<const SimParticle*>(particles->At(index));

            if ((particle->getPdgID() == 11) && (particle->getGenStatus() == 1)) return particle;

            return searchForRecoil(particles, index + 1); 
        }

        const SimParticle* searchForPNGamma(const SimParticle* particle, const int index) { 

            // Check that the index is within the bounds of the array. If not, 
            // throw an exception.
            if (index == particle->getDaughterCount()) 
                throw std::out_of_range("Index is beyond the size of the TClonesArray.");

            const SimParticle* daughter = particle->getDaughter(index);
            if ((daughter->getDaughterCount() > 0) 
                    && (daughter->getDaughter(0)->getProcessType() 
                        == SimParticle::ProcessType::photonNuclear)) return daughter;

            return searchForPNGamma(particle, index + 1);  
        }

        TrackMaps getFindableTrackMaps(const TClonesArray* tracks) { 
       
            TrackMaps map; 

            for (size_t itrk{0}; itrk < tracks->GetEntriesFast(); ++itrk) { 
                FindableTrackResult* trk = static_cast<FindableTrackResult*>(tracks->At(itrk)); 
                if (trk->is4sFindable() || trk->is3s1aFindable() || trk->is2s2aFindable()) { 
                    map.findable[trk->getSimParticle()] =  trk; 
                }

                if (trk->is2sFindable()) map.loose[trk->getSimParticle()] = trk;
            
                if (trk->is2aFindable()) map.axial[trk->getSimParticle()] = trk;    
            }

            return map;  
        }

    } // Analysis    

} // ldmx
