/**
 * @file RecoilTrackerDQM.cxx
 * @brief Analyzer used for DQM of the Recoil tracker. 
 * @author Omar Moreno, SLAC National Accelerator
 */

#include "DQM/RecoilTrackerDQM.h" 

namespace ldmx { 

    RecoilTrackerDQM::RecoilTrackerDQM(const std::string &name, Process &process) : 
        Analyzer(name, process) { }

    RecoilTrackerDQM::~RecoilTrackerDQM() { }

    void RecoilTrackerDQM::onProcessStart() {
        
        // Get an instance of the histogram pool  
        histograms_ = HistogramPool::getInstance();    

        // Open the file and move into the histogram directory
        getHistoDirectory();

    }

    void RecoilTrackerDQM::configure(Parameters& parameters) {
    }

    void RecoilTrackerDQM::analyze(const Event & event) { 
   
        // If the collection of findable tracks doesn't exist, stop processing
        // the event.
        if (!event.exists("FindableTracks")) return;

        // Get the collection of simulated particles from the event
        const std::vector<FindableTrackResult> tracks 
            = event.getCollection<FindableTrackResult>("FindableTracks");

        TrackMaps map = Analysis::getFindableTrackMaps(tracks);
      
        histograms_->get("track_count")->Fill(map.findable.size());  
        histograms_->get("loose_track_count")->Fill(map.loose.size());  
        histograms_->get("axial_track_count")->Fill(map.axial.size());  

        // Get the collection of simulated particles from the event
        auto particleMap{event.getMap<int, SimParticle>("SimParticles")}; 

        // Search for the recoil electron 
        auto [recoilTrackID, recoil] = Analysis::getRecoil(particleMap);

        auto it = map.findable.find(recoilTrackID);
        bool recoilIsFindable = ( it != map.findable.end() );

        // Fill the recoil vertex position histograms
        std::vector<double> recoilVertex = recoil->getVertex();
        histograms_->get("recoil_vx")->Fill(recoilVertex[0]);  
        histograms_->get("recoil_vy")->Fill(recoilVertex[1]);  
        histograms_->get("recoil_vz")->Fill(recoilVertex[2]);  

        double p{-1}, pt{-1}, px{-9999}, py{-9999}, pz{-9999}; 
        const SimTrackerHit* spHit{nullptr}; 
        if (event.exists("TargetScoringPlaneHits")) { 
            
            // Get the collection of simulated particles from the event
            const std::vector<SimTrackerHit> spHits = event.getCollection<SimTrackerHit>("TargetScoringPlaneHits");
            
            for (const SimTrackerHit &hit : spHits ) {
                if ( (hit.getTrackID() == recoilTrackID) /*hit caused by recoil*/ and
                     (hit.getLayerID() == 2) /*hit on downstream side of target*/ and
                     (hit.getMomentum()[2] > 0) /*hit momentum leaving target*/
                   ) {
                    spHit = &hit;
                    break; 
                }
            }

            if ( spHit ) {

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
            const TrackerVetoResult trackerVeto = event.getObject<TrackerVetoResult>("TrackerVeto");

            // Check if the event passes the tracker veto
            if (trackerVeto.passesVeto()) { 
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
    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, RecoilTrackerDQM)
