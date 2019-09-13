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
#include "TVector3.h"

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
            if (index == particle->getDaughterCount()) return nullptr; 
                //throw std::out_of_range("Index is beyond the size of the TClonesArray.");

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

        void printDaughters(const SimParticle* particle, std::string prefix) {

            std::cout << prefix << ">>>>>>>>> Daughters of PDG ID: " << particle->getPdgID() 
                      << " with energy " << particle->getEnergy() << std::endl; 

            for (size_t idaughter = 0; idaughter < particle->getDaughterCount(); ++idaughter) {
            
                // Get the ith daughter
                const SimParticle* daughter = particle->getDaughter(idaughter);

                // Get the PDG ID
                int pdgID = daughter->getPdgID();

                // Calculate the kinetic energy
                double ke = daughter->getEnergy() - daughter->getMass();

                std::vector<double> vec = daughter->getMomentum(); 
                TVector3 pvec(vec[0], vec[1], vec[2]); 

                //  Calculate the polar angle
                double theta = pvec.Theta()*(180/3.14159);

                std::cout << prefix << "    PDG ID: " << pdgID << " KE: " << ke 
                          << " Theta: " << theta  
                          << " Endpoint ( " <<  daughter->getEndPoint()[0] 
                          << " , " << daughter->getEndPoint()[1] 
                          << " , " << daughter->getEndPoint()[2] << " ) " 
                          << " Time: " << daughter->getTime() <<  
                std::endl;

                if ((pdgID == 2112) & (ke > 2000)) { 
                    printDaughters(daughter, "\t"); 
                }
            
            }
            std::cout << prefix << ">>>>>>>>>>\n" << std::endl;

            return;
        }
    
    } // Analysis    

} // ldmx
