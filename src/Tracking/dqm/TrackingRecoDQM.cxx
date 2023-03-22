#include "Tracking/dqm/TrackingRecoDQM.h"

namespace tracking::dqm {

void TrackingRecoDQM::analyze(const framework::Event& event) {
  
  std::string trackCollection = "TaggerTracks";
  if (!event.exists(trackCollection)) return;
  auto tagger_tracks{event.getCollection<ldmx::Track>(trackCollection)};
  
  TrackMonitoring(tagger_tracks,"tagger_trk_");
  
}



void TrackingRecoDQM::TrackMonitoring(const std::vector<ldmx::Track>& tracks,
                                      const std::string title) {
  

  for (auto &track : tracks) {

    //Perigee track parameters

    double trk_d0     = track.getD0();
    double trk_z0     = track.getZ0();
    double trk_qOp    = track.getQoP();
    double trk_theta  = track.getTheta();
    double trk_phi    = track.getPhi();

    //Acts::BoundTrackParameters targetState = track.getState("Target");
    
    //Acts::Vector3 tgt_mom = targetState.momentum();
    
    histograms_.fill(title+"d0",trk_d0);
    histograms_.fill(title+"z0",trk_z0);
    histograms_.fill(title+"qOp",trk_qOp);
    histograms_.fill(title+"phi",trk_phi);
    histograms_.fill(title+"theta",trk_theta);
    histograms_.fill(title+"p", std::abs(1./trk_qOp));

    //Target surface track parameters
    
    //histogram_fill(name+"tgt_loc0", tgt_loc0);
    //histogram_fill(name+"tgt_loc1", tgt_loc1);
    //histogram_fill(name+"tgt_phi",  tgt_phi);
    //histogram_fill(name+"tgt_theta",tgt_theta);
    //histogram_fill(name+"tgt_px",tgt_mom(0));
    //histogram_fill(name+"tgt_py",tgt_mom(1));
    //histogram_fill(name+"tgt_pz",tgt_mom(2));
    
        
  }
        
  
}
} //tracking::dqm

DECLARE_ANALYZER_NS(tracking::dqm, TrackingRecoDQM)
