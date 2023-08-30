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
  auto tracks{event.getCollection<ldmx::Track>(trackCollection_)};
  
  if (event.exists(truthCollection_)) {
    truthTrackCollection_ =
        std::make_shared<std::vector<ldmx::TruthTrack>>(event.getCollection<ldmx::TruthTrack>(truthCollection_));
    doTruthComparison = true;
  }

  TrackMonitoring(tracks,title_);
}


void TrackingRecoDQM::onProcessEnd() {

  //Produce the efficiency plots. (TODO::Switch to TEfficiency instead)

  //TH1* matchp = histograms_.get(title+"match_p");
} 


void TrackingRecoDQM::TrackMonitoring(const std::vector<ldmx::Track>& tracks,
                                      const std::string title) {
  
  //If I have truth information, sort the tracks vector according to their trackID and truthProb
  std::vector<ldmx::Track> uniqueTracks;     // real tracks (truth_prob > cut), unique
  std::vector<ldmx::Track> duplicateTracks;  // real tracks (truth_prob > cut), duplicated
  std::vector<ldmx::Track> fakeTracks;       // fake tracks (truth_prob < cut)
  
  if (doTruthComparison) {
    sortTracks(tracks,uniqueTracks, duplicateTracks,fakeTracks);
  }
  else {
    uniqueTracks = tracks;
  }
  
  for (auto &track : uniqueTracks) {
    
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
    histograms_.fill(title+"pt_beam",    pt_beam);
    
    histograms_.fill(title+"nHits",    track.getNhits());
    histograms_.fill(title+"Chi2",     track.getChi2());
    histograms_.fill(title+"ndf", track.getNdf());
    histograms_.fill(title+"Chi2/ndf", track.getChi2()/track.getNdf());
    histograms_.fill(title+"nShared",  track.getNsharedHits());
    
    
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


    //Track states monitoring
    auto& trackStates = track.getTrackStates();
    
    for (auto& ts : trackStates) {}
        
    if (doTruthComparison) {

      //Truth Comparison
      ldmx::TruthTrack* truth_trk = nullptr;
      
      auto it = std::find_if(truthTrackCollection_->begin(),
                             truthTrackCollection_->end(),[&](const ldmx::TruthTrack& tt) {
                               return tt.getTrackID() == track.getTrackID();
                             });
      
      double trackTruthProb = track.getTruthProb();
      
      if (it != truthTrackCollection_->end() && trackTruthProb >= trackProb_cut_)
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
        
        histograms_.fill(title+"match_d0",   trk_d0);
        histograms_.fill(title+"match_z0",   trk_z0);
        histograms_.fill(title+"match_phi",  trk_phi);
        histograms_.fill(title+"match_theta",trk_theta);
        histograms_.fill(title+"match_p",    trk_p);

      } //found matched track
    }//do TruthComparison
  }//loop on unique tracks (or not split)
  
  
  for (auto& ftrack : fakeTracks) {
    histograms_.fill(title+"fake_d0",   ftrack.getD0());
    histograms_.fill(title+"fake_z0",   ftrack.getZ0());
    histograms_.fill(title+"fake_phi",  ftrack.getPhi());
    histograms_.fill(title+"fake_theta",ftrack.getTheta());
    histograms_.fill(title+"fake_p",    1. / abs(ftrack.getQoP()));
    histograms_.fill(title+"fake_nHits",    ftrack.getNhits());
    histograms_.fill(title+"fake_Chi2",     ftrack.getChi2());
    histograms_.fill(title+"fake_Chi2/ndf", ftrack.getChi2()/ftrack.getNdf());
    histograms_.fill(title+"fake_nShared",  ftrack.getNsharedHits());
  }
  
  for (auto& dtrack : duplicateTracks) {
    histograms_.fill(title+"dup_d0",   dtrack.getD0());
    histograms_.fill(title+"dup_z0",   dtrack.getZ0());
    histograms_.fill(title+"dup_phi",  dtrack.getPhi());
    histograms_.fill(title+"dup_theta",dtrack.getTheta());
    histograms_.fill(title+"dup_p",    1. / abs(dtrack.getQoP()));
    histograms_.fill(title+"dup_nHits",    dtrack.getNhits());
    histograms_.fill(title+"dup_Chi2",     dtrack.getChi2());
    histograms_.fill(title+"dup_Chi2/ndf", dtrack.getChi2()/dtrack.getNdf());
    histograms_.fill(title+"dup_nShared",  dtrack.getNsharedHits());
  }
      
}//Track Monitoring


void TrackingRecoDQM::sortTracks(const std::vector<ldmx::Track>& tracks,
                                 std::vector<ldmx::Track>& uniqueTracks,
                                 std::vector<ldmx::Track>& duplicateTracks,
                                 std::vector<ldmx::Track>& fakeTracks) {
  
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
    if (sortedTracks[i].getTruthProb() < trackProb_cut_) 
      fakeTracks.push_back(sortedTracks[i]);
    else { //not a fake track
      // If this is the first Track object with this trackID, add it to the uniqueTracks vector directly
      if (uniqueTracks.size() == 0 || sortedTracks[i].getTrackID() != sortedTracks[i-1].getTrackID()) {
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
    } //a real track
  } //loop on sorted tracks
  // The total number of elements in the uniqueTracks and duplicateTracks vectors should be equal to the number of elements in the original tracks vector
  if (uniqueTracks.size() + duplicateTracks.size() + fakeTracks.size() != tracks.size()) {
    std::cerr << "Error: unique and duplicate tracks vectors do not add up to original tracks vector" << std::endl;
    return;
  }
  
  if (debug_) {
    // Iterate through the uniqueTracks vector and duplicateTracks vector
    std::cout << "Unique tracks:" << std::endl;
    for (const ldmx::Track& track : uniqueTracks) {
      std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl;
    }
    std::cout << "Duplicate tracks:" << std::endl;
    for (const ldmx::Track& track : duplicateTracks) {
      std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl;
    }
    std::cout << "Fake tracks:" << std::endl;
    for (const ldmx::Track& track : fakeTracks) {
      std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl; 
    }
  }
}
} //tracking::dqm

DECLARE_ANALYZER_NS(tracking::dqm, TrackingRecoDQM)
