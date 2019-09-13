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
#include "Event/Event.h"
#include "Event/EcalVetoResult.h"
#include "Event/FindableTrackResult.h"
#include "Event/HcalVetoResult.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "Event/TrackerVetoResult.h"
#include "Framework/HistogramPool.h"
#include "Tools/AnalysisUtils.h"

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

    void RecoilTrackerDQM::configure(const ParameterSet& ps) {
        ecalVetoCollectionName_ = ps.getString("ecal_veto_collection");
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
        // Check if the TrackerVeto result exists
        if (event.exists("TrackerVeto")) { 

            // Get the collection of trackerVeto results
            const TClonesArray* trackerVeto = event.getCollection("TrackerVeto");

            TrackerVetoResult* veto = static_cast<TrackerVetoResult*>(trackerVeto->At(0)); 
            // Check if the event passes the tracker veto
            if (veto->passesVeto()) { 
                passesTrackVeto = true; 
            }
        }


        if (passesTrackVeto) { 
            histograms_->get("tp_track_veto")->Fill(p);
            histograms_->get("tpt_track_veto")->Fill(pt); 
            histograms_->get("tpx_track_veto")->Fill(px); 
            histograms_->get("tpy_track_veto")->Fill(py); 
            histograms_->get("tpz_track_veto")->Fill(pz); 
        }

        // Get the collection of ECal veto results if it exist
        float bdtProb{-1}; 
        bool passesBDT{false};  
        if (event.exists(ecalVetoCollectionName_)) {
            const EcalVetoResult* veto 
                = static_cast<const EcalVetoResult*>(
                        event.getCollection(ecalVetoCollectionName_)->At(0));
       
            // Get the BDT probability  
            bdtProb = veto->getDisc();
    
            // Fill the histograms if the event passes the ECal veto
            if (bdtProb >= .99) {
        
                histograms_->get("tp_bdt")->Fill(p);
                histograms_->get("tpt_bdt")->Fill(pt); 
                histograms_->get("tpx_bdt")->Fill(px); 
                histograms_->get("tpy_bdt")->Fill(py); 
                histograms_->get("tpz_bdt")->Fill(pz); 
                passesBDT = true; 
            }
        }

        if (passesTrackVeto && passesBDT) { 
            histograms_->get("tp_track_bdt")->Fill(p);
            histograms_->get("tpt_track_bdt")->Fill(pt); 
            histograms_->get("tpx_track_bdt")->Fill(px); 
            histograms_->get("tpy_track_bdt")->Fill(py); 
            histograms_->get("tpz_track_bdt")->Fill(pz); 
        }

        bool passesHcalVeto{false}; 
        // Check if the HcalVeto result exists
        if (event.exists("HcalVeto")) {
        
            // Get the collection of HCalDQM digitized hits if the exists 
            const TClonesArray* hcalVeto = event.getCollection("HcalVeto");

            HcalVetoResult* veto = static_cast<HcalVetoResult*>(hcalVeto->At(0));

            if (veto->passesVeto()) {
                
                histograms_->get("tp_hcal")->Fill(p);
                histograms_->get("tpt_hcal")->Fill(pt); 
                histograms_->get("tpx_hcal")->Fill(px); 
                histograms_->get("tpy_hcal")->Fill(py); 
                histograms_->get("tpz_hcal")->Fill(pz); 
                passesHcalVeto = veto->passesVeto();  
            }
        }



        if (passesTrackVeto && passesBDT && passesHcalVeto) { 
            histograms_->get("tp_vetoes")->Fill(p);
            histograms_->get("tpt_vetoes")->Fill(pt); 
            histograms_->get("tpx_vetoes")->Fill(px); 
            histograms_->get("tpy_vetoes")->Fill(py); 
            histograms_->get("tpz_vetoes")->Fill(pz); 
        }
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, RecoilTrackerDQM)
