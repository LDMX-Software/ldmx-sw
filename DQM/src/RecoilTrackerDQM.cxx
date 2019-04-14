/**
 * @file RecoilTrackerDQM.cxx
 * @brief Analyzer used for DQM of the Recoil tracker. 
 * @author Omar Moreno, SLAC National Accelerator
 */

#include "DQM/RecoilTrackerDQM.h" 

//----------------//
//   C++ StdLib   //
//----------------//
#include <utility>

//----------//
//   ROOT   //
//----------//
#include "TH1.h"
#include "TVector3.h"

//----------//
//   LDMX   //
//----------//
#include "DQM/AnalysisUtils.h"
#include "Event/Event.h"
#include "Event/EcalVetoResult.h"
#include "Event/FindableTrackResult.h"
#include "Event/HcalHit.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "Framework/HistogramPool.h"

namespace ldmx { 

    RecoilTrackerDQM::RecoilTrackerDQM(const std::string &name, Process &process) : 
        Analyzer(name, process) { }

    RecoilTrackerDQM::~RecoilTrackerDQM() {}

    void RecoilTrackerDQM::onProcessStart() {
        
        // Get an instance of the histogram pool  
        histograms_ = HistogramPool::getInstance();    

        // Open the file and move into the histogram directory
        getHistoDirectory();

    }

    void RecoilTrackerDQM::analyze(const Event & event) { 
   
        // If the collection of findable tracks doesn't exist, stop processing
        // the event.
        if (!event.exists("FindableTracks")) return;

        // Get the collection of simulated particles from the event
        const TClonesArray* tracks 
            = event.getCollection("FindableTracks");

        TrackMaps map = Analysis::getFindableTrackMaps(tracks);
      
        histograms_->get("track_count")->Fill(map.findable.size());  
        histograms_->get("loose_track_count")->Fill(map.loose.size());  
        histograms_->get("axial_track_count")->Fill(map.axial.size());  

        // Get the collection of simulated particles from the event
        const TClonesArray* particles = event.getCollection("SimParticles");
      
        // Search for the recoil electron 
        const SimParticle* recoil = Analysis::searchForRecoil(particles);


        bool recoilIsFindable{false}; 
        auto it = map.findable.find(recoil);
        if ( it != map.findable.end()) recoilIsFindable = true; 

        // Fill the recoil vertex position histograms
        std::vector<double> recoilVertex = recoil->getVertex();
        histograms_->get("recoil_vx")->Fill(recoilVertex[0]);  
        histograms_->get("recoil_vy")->Fill(recoilVertex[1]);  
        histograms_->get("recoil_vz")->Fill(recoilVertex[2]);  

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
            
        histograms_->get("tp")->Fill(p);
        histograms_->get("tpt")->Fill(pt); 
        histograms_->get("tpx")->Fill(px); 
        histograms_->get("tpy")->Fill(py); 
        histograms_->get("tpz")->Fill(pz); 
   
        bool passesTrackVeto{false}; 
        if ((map.findable.size() == 1) && recoilIsFindable && (p < 1200)) passesTrackVeto = true; 

        if (passesTrackVeto) { 
            histograms_->get("tp_track_veto")->Fill(p);
            histograms_->get("tpt_track_veto")->Fill(pt); 
            histograms_->get("tpx_track_veto")->Fill(px); 
            histograms_->get("tpy_track_veto")->Fill(py); 
            histograms_->get("tpz_track_veto")->Fill(pz); 
        }

        // Get the collection of ECal veto results if it exist
        float bdtProb{-1}; 
        if (event.exists("EcalVeto")) {
            const EcalVetoResult* veto 
                = static_cast<const EcalVetoResult*>(event.getCollection("EcalVeto")->At(0));
       
            // Get the BDT probability  
            bdtProb = veto->getDisc();
    
            // Fill the histograms if the event passes the ECal veto
            if (bdtProb >= .98) {
        
                histograms_->get("tp_bdt")->Fill(p);
                histograms_->get("tpt_bdt")->Fill(pt); 
                histograms_->get("tpx_bdt")->Fill(px); 
                histograms_->get("tpy_bdt")->Fill(py); 
                histograms_->get("tpz_bdt")->Fill(pz); 
            }
        }

        if (passesTrackVeto && (bdtProb >= .98)) { 
            histograms_->get("tp_track_bdt")->Fill(p);
            histograms_->get("tpt_track_bdt")->Fill(pt); 
            histograms_->get("tpx_track_bdt")->Fill(px); 
            histograms_->get("tpy_track_bdt")->Fill(py); 
            histograms_->get("tpz_track_bdt")->Fill(pz); 
        } 

        double minTimePE{-1}; 
        if (event.exists("hcalDigis")) { 
        
            // Get the collection of HCalDQM digitized hits if the exists 
            const TClonesArray* hcalHits = event.getCollection("hcalDigis");
        
            // Vector containing all HCal hits.  This will be used for sorting.
            std::vector<HcalHit*> hits; 

            // Find the maximum PE in the event 
            for (size_t ihit{0}; ihit < hcalHits->GetEntriesFast(); ++ihit) {
                HcalHit* hit = static_cast<HcalHit*>(hcalHits->At(ihit)); 
           
                if (hit->getTime() != -999) hits.push_back(hit);  
            }
        
            // Sort the array by hit time
            std::sort (hits.begin(), hits.end(), [ ](const auto& lhs, const auto& rhs) 
            {
                return lhs->getTime() < rhs->getTime(); 
            });
        
            for (const auto& hit : hits) { 
                if (hit->getPE() < 3) continue; 
                minTimePE = hit->getPE(); 
                break;
            } 
       
            if (minTimePE == -1) { 
                histograms_->get("tp_max_pe")->Fill(p);
                histograms_->get("tpt_max_pe")->Fill(pt); 
                histograms_->get("tpx_max_pe")->Fill(px); 
                histograms_->get("tpy_max_pe")->Fill(py); 
                histograms_->get("tpz_max_pe")->Fill(pz); 
            }
        }

        if (passesTrackVeto && (bdtProb >= .98) && (minTimePE == -1)) { 
            histograms_->get("tp_vetoes")->Fill(p);
            histograms_->get("tpt_vetoes")->Fill(pt); 
            histograms_->get("tpx_vetoes")->Fill(px); 
            histograms_->get("tpy_vetoes")->Fill(py); 
            histograms_->get("tpz_vetoes")->Fill(pz); 
        }
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, RecoilTrackerDQM)
