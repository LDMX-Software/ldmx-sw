#pragma once

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Vertex.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Event/TruthTrack.h"

namespace tracking::dqm {

enum PIDBins {
  kminus = -4,
  antiproton,
  piminus,
  positron,
  electron,
  piplus,
  proton,
  kplus
};

class VertexingDQM : public framework::Analyzer {
 public:
  VertexingDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process){};

  /// Destructor
  ~VertexingDQM() = default;

  void analyze(const framework::Event& event) override;

  /*
  void VertexMonitoring(const std::vector<ldmx::Track>& vertices,
                       const std::vector<ldmx::Measurement>& measurements,
                       const std::string title, const bool& doDetail,
                       const bool& doTruth);
  */

  void configure(framework::config::Parameters& parameters) override;

  void onProcessEnd() override;



 private:
  std::string vertexCollection_;
  std::string trackCollection_;
  std::string truthCollection_;

  std::string target_sp_events_passname_;
  std::string target_sp_passname_;
  std::string vertex_collection_events_passname_;
  std::string vertex_passname_;
  std::string track_events_passname_;
  std::string track_passname_;
  std::string simparticle_passname_;
  std::string truth_events_passname_;
  std::string truth_passname_;

  std::string title_{"recoil_vertex_"};
  double trackProb_cut_{0.5};
  std::string subdetector_{"Recoil"};
  bool doTruthComparison{false};
  bool debug_{false};

  // Truth Track collection
  std::shared_ptr<ldmx::Tracks> truthTrackCollection_{nullptr};

  // Recon Track collection
  std::shared_ptr<ldmx::Tracks> recoTrackCollection_{nullptr};

  // Target  scoring plane hits
  std::shared_ptr<std::vector<ldmx::SimTrackerHit>> target_scoring_hits_{nullptr}; 

  // PID mapping
  std::map<int, int> pidmap;
// If I have truth information, sort the tracks vector according to their
  // trackID and truthProb
  // real tracks (truth_prob > cut), unique
  std::vector<ldmx::Track> uniqueTracks_;
  // real tracks (truth_prob > cut), duplicated
  std::vector<ldmx::Track> duplicateTracks_;
  // fake tracks (truth_prob < cut)
  std::vector<ldmx::Track> fakeTracks_;

  int pionPdgId_=211; 
  int kshortPdgId_=310;

  ldmx::Track* getTrackFromSimParticleID(std::shared_ptr<ldmx::Tracks> trk_coll, int trkID);
  
};
}  // namespace tracking::dqm
