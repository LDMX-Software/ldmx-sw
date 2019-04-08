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
#include "DQM/AnalysisUtils.h"
#include "DQM/Histogram1DBuilder.h"
#include "Event/Event.h"
#include "Event/HcalHit.h"
#include "Event/SimParticle.h"
#include "Event/EcalVetoResult.h"
#include "Framework/HistogramPool.h"

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

        histograms_->create<TH2F>("max_pe_time", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);
        histograms_->create<TH2F>("max_pe_time_bdt", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);
        histograms_->create<TH2F>("max_pe_time_vetoes", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);

        histograms_->create<TH2F>("min_time_pe", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
        histograms_->create<TH2F>("min_time_pe_bdt", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
        histograms_->create<TH2F>("min_time_pe_vetoes", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
    }

    void HCalDQM::analyze(const Event & event) { 

        // Check if the collection of digitized HCal hits exist. If it doesn't 
        // don't continue processing.
        if (!event.exists("hcalDigis")) return; 

        // Get the collection of HCalDQM digitized hits if the exists 
        const TClonesArray* hcalHits = event.getCollection("hcalDigis");
       
        float maxPE{-1};
        float maxTime{-1};
        double totalPE{0};  

        // Vector containing all HCal hits.  This will be used for sorting.
        std::vector<HcalHit*> hits; 

        // Find the maximum PE in the event 
        for (size_t ihit{0}; ihit < hcalHits->GetEntriesFast(); ++ihit) {
            HcalHit* hit = static_cast<HcalHit*>(hcalHits->At(ihit)); 
            if (maxPE < hit->getPE()) {
                maxPE = hit->getPE(); 
                maxTime = hit->getTime();  
            }
            histograms_->get("pe")->Fill(hit->getPE());
            histograms_->get("hit_time")->Fill(hit->getTime());
           
            totalPE += hit->getPE();

            if (hit->getTime() != -999) hits.push_back(hit);  
        }

        // Sort the array by hit time
        std::sort (hits.begin(), hits.end(), [ ](const auto& lhs, const auto& rhs) 
        {
            return lhs->getTime() < rhs->getTime(); 
        });
        /*std::cout << "[ ";
        for (const auto& hit : hits) {
            std::cout << " ( " << hit->getTime() << ", " << hit->getPE() << " ),  "; 
        }
        std::cout << "] \n";*/

        double minTime{-1}; 
        double minTimePE{-1}; 
        for (const auto& hit : hits) { 
            if (hit->getPE() < 3) continue; 
            minTime = hit->getTime(); 
            minTimePE = hit->getPE(); 
            break;
        } 
        //std::cout << "Min time: " << minTime << " PE: " << minTimePE << std::endl; 

        histograms_->get("max_pe")->Fill(maxPE);
        histograms_->get("hit_time_max_pe")->Fill(maxTime);
        histograms_->get("total_pe")->Fill(totalPE); 
        histograms_->get("n_hits")->Fill(hcalHits->GetEntriesFast());
        histograms_->get("min_hit_time_hcal_veto")->Fill(minTime); 

        histograms_->get("max_pe_time")->Fill(maxPE, maxTime);  
        histograms_->get("min_time_pe")->Fill(minTimePE, minTime);  

        // Get the collection of ECal veto results if it exist
        float bdtProb{-1}; 
        if (event.exists("EcalVeto")) {
            const EcalVetoResult* veto 
                = static_cast<const EcalVetoResult*>(event.getCollection("EcalVeto")->At(0));
       
            // Get the BDT probability  
            bdtProb = veto->getDisc();
            histograms_->get("bdt_n_hits")->Fill(bdtProb, hcalHits->GetEntriesFast()); 
            
            // Fill the histograms if the event passes the ECal veto
            if (bdtProb >= .98) {
                histograms_->get("max_pe_bdt")->Fill(maxPE);
                histograms_->get("total_pe_bdt")->Fill(totalPE); 
                histograms_->get("n_hits_bdt")->Fill(hcalHits->GetEntriesFast());
                histograms_->get("hit_time_max_pe_bdt")->Fill(maxTime);  
                histograms_->get("max_pe_time_bdt")->Fill(maxPE, maxTime);  
                histograms_->get("min_hit_time_bdt")->Fill(minTime); 
                histograms_->get("min_time_pe_bdt")->Fill(minTimePE, minTime);  
            }
        }

        if ((maxPE < 3) && (bdtProb >= .98)) {
        
            histograms_->get("max_pe_vetoes")->Fill(maxPE);
            histograms_->get("total_pe_vetoes")->Fill(totalPE); 
            histograms_->get("n_hits_vetoes")->Fill(hcalHits->GetEntriesFast());
            histograms_->get("hit_time_max_pe_vetoes")->Fill(maxTime);  
            histograms_->get("max_pe_time_vetoes")->Fill(maxPE, maxTime);  
            histograms_->get("min_hit_time_vetoes")->Fill(minTime); 
            histograms_->get("min_time_pe_vetoes")->Fill(minTimePE, minTime);  
        } 
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, HCalDQM)
