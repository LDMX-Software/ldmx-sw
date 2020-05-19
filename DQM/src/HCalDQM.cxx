/**
 * @file HCalDQM.cxx
 * @brief Analyzer used for HCal DQM. 
 * @author Omar Moreno, SLAC National Accelerator
 */

#include "DQM/HCalDQM.h" 

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
        histograms_->create<TH2F>("max_pe:time_hcal_veto", 
                                  "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "HCal max PE hit time (ns)", 1500, 0, 1500);

        histograms_->create<TH2F>("min_time_hit_above_thresh:pe", 
                                  "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                                  "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500);
         
    }

    void HCalDQM::configure(Parameters& parameters) {
    }

    void HCalDQM::analyze(const Event & event) { 

        // Check if the collection of digitized HCal hits exist. If it doesn't 
        // don't continue processing.
        if (!event.exists("hcalDigis")) return; 

        // Get the collection of HCalDQM digitized hits if the exists 
        const std::vector<HcalHit> hcalHits = event.getCollection<HcalHit>("hcalDigis");
     
        // Get the total hit count
        int hitCount = hcalHits.size();  
        histograms_->get("n_hits")->Fill(hitCount); 

        double totalPE{0};  

        // Loop through all HCal hits in the event
        // Get non-noise generated hits into new vector for sorting
        std::vector<const HcalHit *> filteredHits;
        for (const HcalHit &hit : hcalHits ) {

            histograms_->get("pe")->Fill(hit.getPE());
            histograms_->get("hit_time")->Fill(hit.getTime());
           
            totalPE += hit.getPE();

            if ( hit.getTime() > -999. ) { filteredHits.push_back( &hit ); }
        }
        
        histograms_->get("total_pe")->Fill(totalPE); 

        // Sort the array by hit time
        std::sort (filteredHits.begin(), filteredHits.end(), [ ](const auto& lhs, const auto& rhs) 
        {
            return lhs->getTime() < rhs->getTime(); 
        });
        
        //get first time and PE of hit over threshold
        double minTime{-1}; 
        double minTimePE{-1}; 
        for (const auto& hit : filteredHits) { 
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
            const HcalVetoResult hcalVeto = event.getObject<HcalVetoResult>("HcalVeto");

            HcalHit maxPEHit = hcalVeto.getMaxPEHit();  

            // Get the max PE and it's time
            maxPE = maxPEHit.getPE();
            maxPETime = maxPEHit.getTime();
            
            histograms_->get("max_pe")->Fill(maxPE);
            histograms_->get("hit_time_max_pe")->Fill(maxPETime); 
            histograms_->get("max_pe:time")->Fill(maxPE, maxPETime);
            histograms_->get("veto")->Fill(hcalVeto.passesVeto());   

            if (hcalVeto.passesVeto()) {
                histograms_->get("max_pe_hcal_veto")->Fill(maxPE);
                histograms_->get("hit_time_max_pe_hcal_veto")->Fill(maxPETime); 
                histograms_->get("max_pe:time_hcal_veto")->Fill(maxPE, maxPETime);
                histograms_->get("total_pe_hcal_veto")->Fill(totalPE); 
                histograms_->get("n_hits_hcal_veto")->Fill(hitCount); 
                passesHcalVeto = true;
            }
        }
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, HCalDQM)
