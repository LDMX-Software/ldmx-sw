
/**
 * @file EcalVetoResult.cxx
 * @brief Class used to encapsulate the results obtained from 
 *        EcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/EcalVetoResult.h"

ClassImp(ldmx::EcalVetoResult)

namespace ldmx {
            
    EcalVetoResult::EcalVetoResult() :
        TObject() {  
    }

    EcalVetoResult::~EcalVetoResult() {
        Clear(); 
    }




    void EcalVetoResult::Clear(Option_t *option) {
        TObject::Clear();  
        passesVeto_ 	= false;
        nReadoutHits_ 	= 0;
        centroidCell_	= 0;
        nIsoHits_ 		= 0;
        nMipTracks_		= 0;
        longestMipTrack_	= 0;
        mipTrackDep_	= 0;
        summedDet_      = 0;
        summedOuter_	= 0;
        summedIso_      = 0;  
        backEcalSummedDet_  = 0;
        maxIsoHit_		= 0;
        digiECALVec_.clear();

    }

    void EcalVetoResult::Copy(TObject& object) const {
        
        EcalVetoResult& result = (EcalVetoResult&) object;
        result.passesVeto_	    	= passesVeto_;
        result.nReadoutHits_    	= nReadoutHits_;
        result.nIsoHits_    		= nIsoHits_;
        result.nMipTracks_    		= nMipTracks_;
        result.longestMipTrack_    	= longestMipTrack_;
        result.mipTrackDep_    		= mipTrackDep_;
        result.summedDet_     		= summedDet_;
        result.summedOuter_     	= summedOuter_;
        result.summedIso_     		= summedIso_;
        result.backEcalSummedDet_	= backEcalSummedDet_;
        result.maxIsoHit_ 			= maxIsoHit_;
        result.digiECALVec_ 		= digiECALVec_;
        result.centroidCell_		= centroidCell_;
    }

    void EcalVetoResult::setVariables( int centroidCell, int nReadoutHits, int nIsoHits, int nMipTracks,
    		float mipTrackDep, int longestMipTrack, float summedDet, float summedOuter, float summedIso,
    		float backEcalSummedDet, float maxIsoHit, std::vector<float> digiECALVec){
         nReadoutHits_    	= nReadoutHits;
         nIsoHits_    		= nIsoHits;
         nMipTracks_    	= nMipTracks;
         mipTrackDep_    	= mipTrackDep;
         longestMipTrack_   = longestMipTrack;
         summedDet_     	= summedDet;
         summedOuter_     	= summedOuter;
         summedIso_     	= summedIso;
         backEcalSummedDet_ = backEcalSummedDet;
         maxIsoHit_ 		= maxIsoHit;
         digiECALVec_ 		= digiECALVec;
         centroidCell_      = centroidCell;
    }

    void EcalVetoResult::Print(Option_t *option) const { 
        std::cout << "[ EcalVetoResult ]:\n" 
          << "\t Passes veto : " << passesVeto_ << "\n"
          << "\t summedDep: " << summedDet_ << "\n"
          << "\t summedIso: " << summedIso_ << "\n"
          << "\t backSummedDep: " << backEcalSummedDet_ << "\n"
          << std::endl;
    }
}
