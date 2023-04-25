#include "Tracking/dqm/TrackingRecoDQM.h"
#include "Tracking/Sim/TrackingUtils.h"

namespace tracking::dqm {


void TrackingRecoDQM::configure(framework::config::Parameters &parameters) {

  trackCollection_ = parameters.getParameter<std::string>("track_collection","TaggerTracks");
  truthCollection_ = parameters.getParameter<std::string>("truth_collection","TaggerTruthTracks");
  title_           = parameters.getParameter<std::string>("title","tagger_trk_");
  
}

void TrackingRecoDQM::analyze(const framework::Event& event) {
  
  if (!event.exists(trackCollection_)) return;
  auto tagger_tracks{event.getCollection<ldmx::Track>(trackCollection_)};

  if (event.exists(truthCollection_)) {
    truthTrackCollection_ =
        std::make_shared<std::vector<ldmx::TruthTrack>>(event.getCollection<ldmx::TruthTrack>(truthCollection_));
    doTruthComparison = true;
  }
  
  TrackMonitoring(tagger_tracks,title_);
  
  
}

void TrackingRecoDQM::TrackMonitoring(const std::vector<ldmx::Track>& tracks,
                                      const std::string title) {
  
  for (auto &track : tracks) {

    //Perigee track parameters
    
    double trk_d0     = track.getD0();
    double trk_z0     = track.getZ0();
    double trk_qop    = track.getQoP();
    double trk_theta  = track.getTheta();
    double trk_phi    = track.getPhi();

    std::vector<double> trk_mom = track.getMomentum();

    //The transverse momentum in the bending plane
    double pt_bending = std::sqrt(trk_mom[0]*trk_mom[0] + trk_mom[1]*trk_mom[1]);

    //The momentum in the plane transverse wrt the beam axis
    double pt_beam    = std::sqrt(trk_mom[1]*trk_mom[1] + trk_mom[2]*trk_mom[2]);
    
    histograms_.fill(title+"d0",trk_d0);
    histograms_.fill(title+"z0",trk_z0);
    histograms_.fill(title+"qop",trk_qop);
    histograms_.fill(title+"phi",trk_phi);
    histograms_.fill(title+"theta",trk_theta);
    
    histograms_.fill(title+"p",  std::abs(1./trk_qop));
    histograms_.fill(title+"px", trk_mom[0]);
    histograms_.fill(title+"py", trk_mom[1]);
    histograms_.fill(title+"pz", trk_mom[2]);

    histograms_.fill(title+"pt_bending", pt_bending);
    histograms_.fill(title+"pt_beam", pt_beam);
    
    histograms_.fill(title+"nHits", track.getNhits());
    histograms_.fill(title+"Chi2", track.getChi2());
    histograms_.fill(title+"Chi2/ndf", track.getChi2()/track.getNdf());
    histograms_.fill(title+"nShared", track.getNsharedHits());
        
    
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
    
    double sigmap = (1./trk_qop)*(1./trk_qop)*sigmaqop;
    histograms_.fill(title+"p_err",  sigmap); 
    
    //Target surface track parameters
    
    //histogram_fill(name+"tgt_loc0", tgt_loc0);
    //histogram_fill(name+"tgt_loc1", tgt_loc1);
    //histogram_fill(name+"tgt_phi",  tgt_phi);
    //histogram_fill(name+"tgt_theta",tgt_theta);
    //histogram_fill(name+"tgt_px",tgt_mom(0));
    //histogram_fill(name+"tgt_py",tgt_mom(1));
    //histogram_fill(name+"tgt_pz",tgt_mom(2));


    

    if (doTruthComparison) {

      //Truth Comparison
      ldmx::TruthTrack* truth_trk = nullptr;
      
      auto it = std::find_if(truthTrackCollection_->begin(),
                          truthTrackCollection_->end(),[&](const ldmx::TruthTrack& tt) {
                            return tt.getTrackID() == track.getTrackID();
                          });
      
      
      if (it != truthTrackCollection_->end())
        truth_trk = &(*it);


      //Found matched track
      if (truth_trk) {

        double truth_d0    = truth_trk->getD0();
        double truth_z0    = truth_trk->getZ0();
        double truth_phi   = truth_trk->getPhi();
        double truth_theta = truth_trk->getTheta();
        double truth_qop   = truth_trk->getQoP();
        
        histograms_.fill(title+"truth_d0",   truth_d0);
        histograms_.fill(title+"truth_z0",   truth_z0);
        histograms_.fill(title+"truth_phi",  truth_phi);
        histograms_.fill(title+"truth_theta",truth_theta);
        histograms_.fill(title+"truth_qop",  truth_qop);

        double res_d0    = trk_d0 - truth_d0;
        double res_z0    = trk_z0 - truth_z0;
        double res_phi   = trk_phi - truth_phi;
        double res_theta = trk_theta - truth_theta;
        double res_qop   = trk_qop - truth_qop;

        histograms_.fill(title+"res_d0",   res_d0);
        histograms_.fill(title+"res_z0",   res_z0);
        histograms_.fill(title+"res_phi",  res_phi);
        histograms_.fill(title+"res_theta",res_theta);
        histograms_.fill(title+"res_qop",  res_qop);
        
        double pull_d0    = res_d0    / sigmad0;
        double pull_z0    = res_z0    / sigmaz0;
        double pull_phi   = res_phi   / sigmaphi;
        double pull_theta = res_theta / sigmatheta;
        double pull_qop   = res_qop   / sigmaqop;
        
        histograms_.fill(title+"pull_d0",   pull_d0);
        histograms_.fill(title+"pull_z0",   pull_z0);
        histograms_.fill(title+"pull_phi",  pull_phi);
        histograms_.fill(title+"pull_theta",pull_theta);
        histograms_.fill(title+"pull_qop",  pull_qop);
        
      } //found matched track
    }//do TruthComparison
  
  }//track loop
        
  
}
} //tracking::dqm

DECLARE_ANALYZER_NS(tracking::dqm, TrackingRecoDQM)
