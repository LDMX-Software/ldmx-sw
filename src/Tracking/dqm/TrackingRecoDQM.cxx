#include "Tracking/dqm/TrackingRecoDQM.h"
#include "Tracking/Sim/TrackingUtils.h"

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

    std::vector<double> trk_mom = track.getMomentum();

    //The transverse momentum in the bending plane
    double pt_bending = std::sqrt(trk_mom[0]*trk_mom[0] + trk_mom[1]*trk_mom[1]);

    //The momentum in the plane transverse wrt the beam axis
    double pt_beam    = std::sqrt(trk_mom[1]*trk_mom[1] + trk_mom[2]*trk_mom[2]);
    
    histograms_.fill(title+"d0",trk_d0);
    histograms_.fill(title+"z0",trk_z0);
    histograms_.fill(title+"qOp",trk_qOp);
    histograms_.fill(title+"phi",trk_phi);
    histograms_.fill(title+"theta",trk_theta);

    histograms_.fill(title+"p",  std::abs(1./trk_qOp));
    histograms_.fill(title+"px", trk_mom[0]);
    histograms_.fill(title+"py", trk_mom[1]);
    histograms_.fill(title+"pz", trk_mom[2]);

    histograms_.fill(title+"pt_bending", pt_bending);
    histograms_.fill(title+"pt_beam", pt_beam);
    

    histograms_.fill(title+"nHits", track.getNhits());
   
    //Covariance matrix
    Acts::BoundSymMatrix cov = tracking::sim::utils::unpackCov(track.getPerigeeCov());

    double sigmad0    = sqrt(cov(Acts::BoundIndices::eBoundLoc0,Acts::BoundIndices::eBoundLoc0));
    double sigmaz0    = sqrt(cov(Acts::BoundIndices::eBoundLoc1,Acts::BoundIndices::eBoundLoc1));
    double sigmaphi   = sqrt(cov(Acts::BoundIndices::eBoundPhi ,Acts::BoundIndices::eBoundPhi));
    double sigmatheta = sqrt(cov(Acts::BoundIndices::eBoundTheta,Acts::BoundIndices::eBoundTheta));
    double sigmaqop   = sqrt(cov(Acts::BoundIndices::eBoundQOverP,Acts::BoundIndices::eBoundQOverP));
    
    histograms_.fill(title+"d0_err",   sigmad0); 
    histograms_.fill(title+"z0_err",   sigmaz0); 
    histograms_.fill(title+"phi_err",  sigmaphi); 
    histograms_.fill(title+"theta_err",sigmatheta); 
    histograms_.fill(title+"qop_err",  sigmaqop); 
                                        
    

    double sigmap = (1./trk_qOp)*(1./trk_qOp)*sigmaqop;
    histograms_.fill(title+"p_err",  sigmap); 

    
    
    //histograms_.fill(title+"Chi2", track.getChi2());
    //histograms_.fill(title+"Chi2/ndf", track.getChi2() / track.getNdf());

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
