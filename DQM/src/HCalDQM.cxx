/**
 * @file HCalDQM.cxx
 * @brief Analyzer used for HCal DQM. 
 * @author Omar Moreno, SLAC National Accelerator
 */

#include "DQM/HCalDQM.h" 

#include <algorithm>

//----------//
//   ROOT   //
//----------//
#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"

//----------//
//   LDMX   //
//----------//
#include "DQM/Histogram1DBuilder.h"
#include "Event/Event.h"
#include "Event/FindableTrackResult.h"
#include "Event/HcalHit.h"
#include "Event/HcalVetoResult.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "Event/EcalVetoResult.h"
#include "Framework/HistogramPool.h"
#include "Tools/AnalysisUtils.h"

namespace ldmx { 

    HCalDQM::HCalDQM(const std::string &name, Process &process) : 
        Analyzer(name, process) { }

    HCalDQM::~HCalDQM() {}

    void HCalDQM::onProcessStart() {
       
        // Get an instance of the histogram pool  
        histograms_ = HistogramPool::getInstance();    

        // Move into the HCal directory
        getHistoDirectory();

        histograms_->create<TH2F>("bdt_n_hits", "BDT discriminant", 200, 0, 1, 
                                  "HCal hit multiplicity", 300, 0, 300);

        histograms_->create<TH2F>("max_pe:time", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);
        histograms_->create<TH2F>("max_pe:time_track_veto", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);
        histograms_->create<TH2F>("max_pe:time_bdt", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);
        histograms_->create<TH2F>("max_pe:time_hcal_veto", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);
        histograms_->create<TH2F>("max_pe:time_track_bdt", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);
        histograms_->create<TH2F>("max_pe:time_vetoes", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);

        histograms_->create<TH2F>("min_time_hit_above_thresh:pe", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
        histograms_->create<TH2F>("min_time_hit_above_thresh:pe_track_veto", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
        histograms_->create<TH2F>("min_time_hit_above_thresh:pe_bdt", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
        histograms_->create<TH2F>("min_time_hit_above_thresh:pe_track_bdt", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
        histograms_->create<TH2F>("min_time_hit_above_thresh:pe_vetoes", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
         
        histograms_->create<TH2F>("bdt_max_pe", 
                            "Max PE", 500, 0, 500, 
                            "BDT Prob", 200, 0.9, 1.0);
        histograms_->create<TH2F>("bdt_max_pe_vetoes", 
                            "Max PE", 500, 0, 500, 
                            "BDT Prob", 200, 0.9, 1.0);
    
    }

    void HCalDQM::analyze(const Event & event) { 

        // Check if the collection of digitized HCal hits exist. If it doesn't 
        // don't continue processing.
        if (!event.exists("hcalDigis")) return; 

        // Get the collection of HCalDQM digitized hits if the exists 
        const TClonesArray* hcalHits = event.getCollection("hcalDigis");
     
        // Get the total hit count
        int hitCount = hcalHits->GetEntriesFast();  
        histograms_->get("n_hits")->Fill(hitCount); 

        // Vector containing all HCal hits.  This will be used for sorting.
        std::vector<HcalHit*> hits; 

        double totalPE{0};  
        // Loop through all HCal hits in the event
        for (size_t ihit{0}; ihit < hitCount; ++ihit) {
            HcalHit* hit = static_cast<HcalHit*>(hcalHits->At(ihit)); 
            histograms_->get("pe")->Fill(hit->getPE());
            histograms_->get("hit_time")->Fill(hit->getTime());
           
            totalPE += hit->getPE();

            // Create the list that will be used to find the earliest hit time
            if (hit->getTime() != -999) hits.push_back(hit);  
        }
        
        histograms_->get("total_pe")->Fill(totalPE); 

        // Sort the array by hit time
        std::sort (hits.begin(), hits.end(), [ ](const auto& lhs, const auto& rhs) 
        {
            return lhs->getTime() < rhs->getTime(); 
        });
        
        double minTime{-1}; 
        double minTimePE{-1}; 
        for (const auto& hit : hits) { 
            if (hit->getPE() < maxPEThreshold_) continue; 
            minTime = hit->getTime(); 
            minTimePE = hit->getPE(); 
            break;
        } 

        histograms_->get("min_time_hit_above_thresh")->Fill(minTime); 
        histograms_->get("min_time_hit_above_thresh:pe")->Fill(minTimePE, minTime);  

        float maxPE{-1};
        float maxPETime{-1};
        bool passesHcalVeto{false}; 
        // Check if the HcalVeto result exists
        if (event.exists("HcalVeto")) {
        
            // Get the collection of HCalDQM digitized hits if the exists 
            const TClonesArray* hcalVeto = event.getCollection("HcalVeto");

            HcalVetoResult* veto = static_cast<HcalVetoResult*>(hcalVeto->At(0));
            HcalHit* maxPEHit = veto->getMaxPEHit();  


            // Get the max PE and it's time
            maxPE = maxPEHit->getPE();
            maxPETime = maxPEHit->getTime();
            
            histograms_->get("max_pe")->Fill(maxPE);
            histograms_->get("hit_time_max_pe")->Fill(maxPETime); 
            histograms_->get("max_pe:time")->Fill(maxPE, maxPETime);
            histograms_->get("veto")->Fill(veto->passesVeto());   

            if (veto->passesVeto()) {
                histograms_->get("max_pe_hcal_veto")->Fill(maxPE);
                histograms_->get("hit_time_max_pe_hcal_veto")->Fill(maxPETime); 
                histograms_->get("max_pe:time_hcal_veto")->Fill(maxPE, maxPETime);
                histograms_->get("total_pe_hcal_veto")->Fill(totalPE); 
                histograms_->get("n_hits_hcal_veto")->Fill(hitCount); 
                passesHcalVeto = veto->passesVeto();  
            }
        }

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

        // Get the collection of ECal veto results if it exist
        float bdtProb{-1};
        bool passesBDT{false};  
        if (event.exists("EcalVeto")) {
            const EcalVetoResult* veto 
                = static_cast<const EcalVetoResult*>(event.getCollection("EcalVeto")->At(0));
       
            // Get the BDT probability  
            bdtProb = veto->getDisc();
            histograms_->get("bdt_n_hits")->Fill(bdtProb, hitCount); 
            
            // Fill the histograms if the event passes the ECal veto
            if (bdtProb >= .98) {
                histograms_->get("max_pe_bdt")->Fill(maxPE);
                histograms_->get("total_pe_bdt")->Fill(totalPE); 
                histograms_->get("n_hits_bdt")->Fill(hitCount);
                histograms_->get("hit_time_max_pe_bdt")->Fill(maxPETime);  
                histograms_->get("min_time_hit_above_thresh_bdt")->Fill(minTime); 
                histograms_->get("max_pe:time_bdt")->Fill(maxPE, maxPETime);  
                histograms_->get("min_time_hit_above_thresh:pe_bdt")->Fill(minTimePE, minTime); 
                passesBDT = true;  
            }
        }

        if (passesTrackVeto) {
            histograms_->get("max_pe_track_veto")->Fill(maxPE);
            histograms_->get("total_pe_track_veto")->Fill(totalPE); 
            histograms_->get("n_hits_track_veto")->Fill(hitCount);
            histograms_->get("hit_time_max_pe_track_veto")->Fill(maxPETime);  
            histograms_->get("min_time_hit_above_thresh_track_veto")->Fill(minTime); 
            histograms_->get("max_pe:time_track_veto")->Fill(maxPE, maxPETime);  
            histograms_->get("min_time_hit_above_thresh:pe_track_veto")->Fill(minTimePE, minTime);  
            histograms_->get("bdt_max_pe")->Fill(maxPE, bdtProb); 
        }

        if (passesTrackVeto && passesBDT) { 
            histograms_->get("max_pe_track_bdt")->Fill(maxPE);
            histograms_->get("total_pe_track_bdt")->Fill(totalPE); 
            histograms_->get("n_hits_track_bdt")->Fill(hitCount);
            histograms_->get("hit_time_max_pe_track_bdt")->Fill(maxPETime);  
            histograms_->get("max_pe:time_track_bdt")->Fill(maxPE, maxPETime);  
            histograms_->get("min_time_hit_above_thresh_track_bdt")->Fill(minTime); 
            histograms_->get("min_time_hit_above_thresh:pe_track_bdt")->Fill(minTimePE, minTime);  
        }

        if (passesTrackVeto && passesHcalVeto && passesBDT) {
        
            histograms_->get("max_pe_vetoes")->Fill(maxPE);
            histograms_->get("total_pe_vetoes")->Fill(totalPE); 
            histograms_->get("n_hits_vetoes")->Fill(hitCount);
            histograms_->get("hit_time_max_pe_vetoes")->Fill(maxPETime);  
            histograms_->get("max_pe:time_vetoes")->Fill(maxPE, maxPETime);  
            histograms_->get("min_time_hit_above_thresh_vetoes")->Fill(minTime); 
            histograms_->get("bdt_max_pe_vetoes")->Fill(maxPE, bdtProb); 
        } 
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, HCalDQM)
