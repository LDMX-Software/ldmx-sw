/**
 * @file EcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        EcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_ECALVETORESULT_H_
#define EVENT_ECALVETORESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>

//----------//
//   LDMX   //
//----------//
#include "Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

namespace ldmx { 
    
    class EcalVetoResult : public TObject { 
        
        public: 

            /** Constructor */
            EcalVetoResult(); 

            /** Destructor */
            ~EcalVetoResult(); 

            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setVariables(int centroidCell, int nReadoutHits, int nIsoHits, int nMipTracks,
            		float mipTrackDep, int longestMipTrack, float summedDet, float summedOuter, float summedIso,
            		float backEcalSummedDet, float maxIsoHit, std::vector<float> digiECALVec);

            /** Reset the object. */
            void Clear(Option_t *option = ""); 
            
            /**
             * Copy this object. 
             *
             * @param object The target object. 
             */
            void Copy(TObject& object) const;

            /** Print the object */
            void Print(Option_t *option = "") const;

            /** Checks if the event passes the Ecal veto. */
            bool passesVeto() { return passesVeto_; };

            int getNReadoutHits() { return nReadoutHits_; };

            int getNIsoHits() { return nIsoHits_; };

            int getNMipTracks() { return nMipTracks_; };

            int getLongestMipTrack() { return longestMipTrack_; };


            float getSummedDet() { return summedDet_; };

            float getSummedOuter() { return summedOuter_; };

            float getSummedIso() { return summedIso_; };

            float getBackSummedDep() { return backEcalSummedDet_; };

            float getMaxIsoHit() { return maxIsoHit_; };

            float getMipTrackDep() { return mipTrackDep_; };

            void setVetoResult(double passesVeto) {    	 passesVeto_ 		= passesVeto;}


            std::vector<float> digiECALVec() { return digiECALVec_; };


        private:
           
            /** Flag indicating whether the event is vetoed by the Ecal. */
            bool passesVeto_{false};
            
            int   centroidCell_{0};
            int   nReadoutHits_{0};
            int   nIsoHits_{0};
            int   nMipTracks_{0};
            int   longestMipTrack_{0};
            float summedDet_{0};
            float summedIso_{0};
            float summedOuter_{0};
            float maxIsoHit_{0};
            float backEcalSummedDet_{0};
            float mipTrackDep_{0};

            std::vector<float> digiECALVec_;
            
        ClassDef(EcalVetoResult, 1); 

    }; // EcalVetoResult
}


#endif // EVENT_ECALVETORESULT_H_
