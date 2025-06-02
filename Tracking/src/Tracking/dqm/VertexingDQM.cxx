#include "Tracking/dqm/VertexingDQM.h"

#include <algorithm>
#include <iostream>

#include "Tracking/Sim/TrackingUtils.h"

namespace tracking::dqm {
  
  void VertexingDQM::configure(framework::config::Parameters& parameters) {
    vertexCollection_ = parameters.getParameter<std::string>("vertex_collection");
    truthCollection_ = parameters.getParameter<std::string>("truth_collection");
    trackCollection_ = parameters.getParameter<std::string>("track_collection");
    
    target_sp_events_passname_ =
      parameters.getParameter<std::string>("target_sp_events_passname");
    target_sp_passname_ =
      parameters.getParameter<std::string>("target_sp_passname");
    truth_passname_ = parameters.getParameter<std::string>("truth_passname");
    truth_events_passname_ =
      parameters.getParameter<std::string>("truth_events_passname");
    track_passname_ = parameters.getParameter<std::string>("track_passname");
    track_collection_events_passname_ =
      parameters.getParameter<std::string>("track_collection_events_passname");
    
    title_ = parameters.getParameter<std::string>("title", "recoil_vertex_");
    trackProb_cut_ = parameters.getParameter<double>("trackProb_cut", 0.5);
    subdetector_ = parameters.getParameter<std::string>("subdetector", "Recoil");
    
    ldmx_log(info) << "Vertex Collection " << vertexCollection_ << std::endl;
    ldmx_log(info) << "Track Collection " << trackCollection_ << std::endl;
    ldmx_log(info) << "Truth Collection " << truthCollection_ << std::endl;
    
    pidmap[-321] = PIDBins::kminus;
    pidmap[321] = PIDBins::kplus;
    pidmap[-211] = PIDBins::piminus;
    pidmap[211] = PIDBins::piplus;
    pidmap[11] = PIDBins::electron;
    pidmap[-11] = PIDBins::positron;
    pidmap[2212] = PIDBins::proton;
    pidmap[-2212] = PIDBins::antiproton;
  }
  
  void VertexingDQM::analyze(const framework::Event& event) {
    ldmx_log(debug) << "DQM Reading in::" << trackCollection_ << std::endl;
    
    if (!event.exists(vertexCollection_, vertex_collection_events_passname_)) {
      ldmx_log(error) << "ERROR:: vertexCollection " << vertexCollection_
		      << " not in event" << std::endl;
      return;
    }
    
    auto vertices{
      event.getCollection<ldmx::Vertex>(vertexCollection_, vertex_passname_)};
    
    if (!event.exists(trackCollection_, track_collection_events_passname_)) {
      ldmx_log(error) << "ERROR:: trackCollection " << trackCollection_
		      << " not in event" << std::endl;
    return;
    }
    auto tracks{ event.getCollection<ldmx::Track>(trackCollection_, track_passname_)};
      // The truth track collection
    if (event.exists(truthCollection_, truth_events_passname_)) {
      truthTrackCollection_ = std::make_shared<ldmx::Tracks>(event.getCollection<ldmx::Track>(truthCollection_, truth_passname_));
      doTruthComparison = true;
    }
    
    if (event.exists("TargetScoringPlaneHits", target_sp_events_passname_)) {
      target_scoring_hits_ =
	std::make_shared<std::vector<ldmx::SimTrackerHit>>(event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits",target_sp_passname_));
    }

    //  ldmx_log(debug) << "Do truth comparison::" << doTruthComparison << std::endl;
    //might do something with this later. 
    //  if (doTruthComparison) {
    //  sortTracks(tracks, uniqueTracks_, duplicateTracks_, fakeTracks_);
    //} else {
    //  uniqueTracks_ = tracks;
    // }
    
    ldmx_log(info) << "Filling histograms " << std::endl;

    // General Plots
    //    histograms_.fill(title_ + "N_tracks", tracks.size());
    //histograms_.fill(title_ + "N_vertex", vertices.size());
    
    histograms_.fill("N_tracks", tracks.size());
    histograms_.fill("N_vertex", vertices.size());

    for(int iV=0; iV<vertices.size(); iV++){
      auto vert = vertices.at(iV); 
      histograms_.fill("vertex_x",vert.position()[0]);
      histograms_.fill("vertex_y",vert.position()[1]);
      histograms_.fill("vertex_z",vert.position()[2]);


      //      histograms_.fill(title_+"vertex_chi2",vert.fitQuality().first);
      // histograms_.fill(title_+"vertex_ndf",vert.fitQuality().second);
      //histograms_.fill(title_+"vertex_chi2_over_ndf",vert.fitQuality().first/vert.fitQuality().second);
    }

    
    //    ldmx_log(debug) << "Track Monitoring on Unique Tracks" << std::endl;
    
    //    TrackMonitoring(uniqueTracks_, measurements, title_, true, true);
    
    //  ldmx_log(debug) << "Track Monitoring on duplicates and fakes" << std::endl;
    // Fakes and duplicates
    //TrackMonitoring(duplicateTracks_, measurements, title_ + "dup_", false,
    //                false);
    ///TrackMonitoring(fakeTracks_, measurements, title_ + "fake_", false, false);
    
    
    // Tagger Recoil Matching
    
    // Clear the vectors
    //  uniqueTracks_.clear();
    //duplicateTracks_.clear();
    //fakeTracks_.clear();
  }
  
  void VertexingDQM::onProcessEnd() {
    // Produce the efficiency plots. (TODO::Switch to TEfficiency instead)
  }
  
}// namespace tracking::dqm

DECLARE_ANALYZER(tracking::dqm::VertexingDQM)
