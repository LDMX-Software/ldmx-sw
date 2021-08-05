
#ifndef TRIGSCINT_TRIGSCINTTRACKPRODUCER_H
#define TRIGSCINT_TRIGSCINTTRACKPRODUCER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TrigScintCluster.h"
#include "TrigScint/Event/TrigScintTrack.h"

namespace trigscint {

/**
 * @class TrigScintTrackProducer
 * @brief making tracks from trigger scintillator clusters
 */
class TrigScintTrackProducer : public framework::Producer {
 public:
  TrigScintTrackProducer(const std::string &name, framework::Process &process)
      : Producer(name, process) {}

  virtual void configure(framework::config::Parameters &ps);

  virtual void produce(framework::Event &event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

 private:
  // collection of produced tracks
  std::vector<ldmx::TrigScintTrack> tracks_;

  // add a cluster to a track
  ldmx::TrigScintTrack makeTrack(std::vector<ldmx::TrigScintCluster> clusters);

  // match x, y tracks and set their x,y spatial coordinates 
  void matchXYTracks( std::vector<ldmx::TrigScintTrack> &tracks);
  //std::vector<ldmx::TrigScintTrack> matchXYTracks( std::vector<ldmx::TrigScintTrack> &tracks);

  // maximum difference (in channel number space) between track seed and cluster
  // in the next pad tolerated to form a track
  double maxDelta_{0.};

  // producer specific verbosity
  int verbose_{0};

  // collection used to seed the tracks
  std::string seeding_collection_;

  // other cluster collections used in track making
  std::vector<std::string> input_collections_;

  // output collection (tracks)
  std::string output_collection_;

  // specific pass name to use for track making
  std::string passName_{""};


  // allow forming tracks without match in the last collection
  bool skipLast_{false};
  
  // vertical bar start index 
  int vertBarStartIdx_{52};

  // number of horizontal bars (one layer) in geometry
  int nBarsY_{16};

  // number of vertical bars (one row) in geometry
  int nBarsX_{8};

  // track centroid in units of channel nb (will not be content weighted)
  float centroid_{0.};

  // track horizontal centroid in units of channel nb (will not be content weighted)
  float centroidX_{-1};

  // track vertical centroid in units of channel nb (will not be content weighted)
  float centroidY_{-1};
  
  // track residual in units of channel nb (will not be content weighted)
  float residual_{0.};


  float barWidth_y_{3.}; //mm
  float barGap_y_{2.1}; //mm
  float barWidth_x_{3.}; //mm
  float barGap_x_{0.1}; //mm

  float xConvFactor_;  //geometry conversion factors
  float xStart_;
  float yConvFactor_;
  float yStart_;
};


}  // namespace trigscint

#endif // TRIGSCINT_TRIGSCINTTRACKPRODUCER_H 
