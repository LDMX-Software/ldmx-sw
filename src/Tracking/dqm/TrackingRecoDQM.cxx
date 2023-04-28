#include "Tracking/dqm/TrackingRecoDQM.h"
#include "Tracking/Sim/TrackingUtils.h"

#include <algorithm>

namespace tracking::dqm {


void TrackingRecoDQM::configure(framework::config::Parameters &parameters) {

  trackCollection_ = parameters.getParameter<std::string>("track_collection","TaggerTracks");
  truthCollection_ = parameters.getParameter<std::string>("truth_collection","TaggerTruthTracks");
  title_           = parameters.getParameter<std::string>("title","tagger_trk_");
  trackProb_cut_   = parameters.getParameter<double>("trackProb_cut",0.5);
  
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


void TrackingRecoDQM::onProcessEnd() {

  //Produce the efficiency plots. (TODO::Switch to TEfficiency instead)

  //TH1* matchp = histograms_.get(title+"match_p");
} 


void TrackingRecoDQM::TrackMonitoring(const std::vector<ldmx::Track>& tracks,
                                      const std::string title) {

  //If I have truth information, sort the tracks vector according to their trackID and truthProb
  
  for (auto &track : tracks) {

    //Perigee track parameters
    
    double trk_d0     = track.getD0();
    double trk_z0     = track.getZ0();
    double trk_qop    = track.getQoP();
    double trk_theta  = track.getTheta();
    double trk_phi    = track.getPhi();
    double trk_p      = 1./abs(trk_qop);
    
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

      //Remove duplicates
      std::vector<ldmx::Track> unique_tracks;
      std::vector<ldmx::Track> duplicate_tracks;
    
      removeDuplicates(tracks,unique_tracks,duplicate_tracks);
      
      //Truth Comparison
      ldmx::TruthTrack* truth_trk = nullptr;
    
      auto it = std::find_if(truthTrackCollection_->begin(),
                             truthTrackCollection_->end(),[&](const ldmx::TruthTrack& tt) {
                               return tt.getTrackID() == track.getTrackID();
                             });
      
      double trackTruthProb = track.getTruthProb();
      
      if (it != truthTrackCollection_->end() && trackTruthProb > trackProb_cut_)
        truth_trk = &(*it);
     
      
      //Found matched track
      if (truth_trk) {

        double truth_d0    = truth_trk->getD0();
        double truth_z0    = truth_trk->getZ0();
        double truth_phi   = truth_trk->getPhi();
        double truth_theta = truth_trk->getTheta();
        double truth_qop   = truth_trk->getQoP();
        double truth_p     = 1. / abs(truth_trk->getQoP());
        
        histograms_.fill(title+"truth_d0",   truth_d0);
        histograms_.fill(title+"truth_z0",   truth_z0);
        histograms_.fill(title+"truth_phi",  truth_phi);
        histograms_.fill(title+"truth_theta",truth_theta);
        histograms_.fill(title+"truth_qop",  truth_qop);
        histograms_.fill(title+"truth_p",    truth_p);

        double res_d0    = trk_d0 - truth_d0;
        double res_z0    = trk_z0 - truth_z0;
        double res_phi   = trk_phi - truth_phi;
        double res_theta = trk_theta - truth_theta;
        double res_qop   = trk_qop - truth_qop;
        double res_p     = trk_p - truth_p;

        histograms_.fill(title+"res_d0",   res_d0);
        histograms_.fill(title+"res_z0",   res_z0);
        histograms_.fill(title+"res_phi",  res_phi);
        histograms_.fill(title+"res_theta",res_theta);
        histograms_.fill(title+"res_qop",  res_qop);
        histograms_.fill(title+"res_p",    res_p);
        
        double pull_d0    = res_d0    / sigmad0;
        double pull_z0    = res_z0    / sigmaz0;
        double pull_phi   = res_phi   / sigmaphi;
        double pull_theta = res_theta / sigmatheta;
        double pull_qop   = res_qop   / sigmaqop;
        double pull_p     = res_p     / sigmap;
        
        histograms_.fill(title+"pull_d0",   pull_d0);
        histograms_.fill(title+"pull_z0",   pull_z0);
        histograms_.fill(title+"pull_phi",  pull_phi);
        histograms_.fill(title+"pull_theta",pull_theta);
        histograms_.fill(title+"pull_qop",  pull_qop);
        histograms_.fill(title+"pull_p",    pull_p);


        //Fill reco plots for efficiencies - numerator
        
        histograms_.fill(title+"match_d0",   truth_d0);
        histograms_.fill(title+"match_z0",   truth_z0);
        histograms_.fill(title+"match_phi",  truth_phi);
        histograms_.fill(title+"match_theta",truth_theta);
        histograms_.fill(title+"match_p",    truth_p);
      
      
      } //found matched track
    }//do TruthComparison
  }//loop on tracks
  
  if (doTruthComparison) {
    std::vector<ldmx::Tracks> uniqueTracks;
    std::vector<ldmx::Tracks> duplicateTracks;
    removeDuplicates(tracks,uniqueTracks, duplicateTracks);
  }
  
    
}//Track Monitoring


void removeDuplicates(const std::vector<ldmx::Track>& tracks,
                      std::vector<ldmx::Track>& uniqueTracks,
                      std::vector<ldmx::Track>& duplicateTracks) {
  
  // Create a copy of the const vector so we can sort it
  std::vector<ldmx::Track> sortedTracks = tracks;
  
  // Sort the vector of Track objects based on their trackID member
  std::sort(sortedTracks.begin(),
            sortedTracks.end(),
            [](ldmx::Track& t1,ldmx::Track& t2){
              return t1.getTrackID() < t2.getTrackID();
            }
            );
  
  // Loop over the sorted vector of Track objects
  for (size_t i = 0; i < sortedTracks.size(); i++) {
    // If this is the first Track object with this trackID, add it to the uniqueTracks vector
    if (i == 0 || sortedTracks[i].getTrackID() != sortedTracks[i-1].getTrackID()) {
      uniqueTracks.push_back(sortedTracks[i]);
    }
    // Otherwise, add it to the duplicateTracks vector if its truthProb is lower than the existing Track object
    // Otherwise, if the truthProbability is higher than the track stored in uniqueTracks, put it in uniqueTracks and move the uniqueTracks.back to duplicateTracks.
    else if (sortedTracks[i].getTruthProb() > uniqueTracks.back().getTruthProb()) {
      duplicateTracks.push_back(uniqueTracks.back());
      uniqueTracks.back() = sortedTracks[i];
    }
    // Otherwise, add it to the duplicateTracks vector
    else {
      duplicateTracks.push_back(sortedTracks[i]);
    }
  }
  
  // The total number of elements in the uniqueTracks and duplicateTracks vectors should be equal to the number of elements in the original tracks vector
  if (uniqueTracks.size() + duplicateTracks.size() != tracks.size()) {
    std::cerr << "Error: unique and duplicate tracks vectors do not add up to original tracks vector" << std::endl;
    return;
  }
  
  // Iterate through the uniqueTracks vector and duplicateTracks vector
  std::cout << "Unique tracks:" << std::endl;
  for (const ldmx::Track& track : uniqueTracks) {
    std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl;
  }
  std::cout << "Duplicate tracks:" << std::endl;
  for (const ldmx::Track& track : duplicateTracks) {
    std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl;
  }
}
} //tracking::dqm

DECLARE_ANALYZER_NS(tracking::dqm, TrackingRecoDQM)
