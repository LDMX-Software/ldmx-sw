
#ifndef TRIGSCINT_TRIGSCINTTRACKPRODUCER_H
#define TRIGSCINT_TRIGSCINTTRACKPRODUCER_H

// LDMX Framework
#include <unordered_set>

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
  TrigScintTrackProducer(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

  void onProcessStart() override;
  void onProcessEnd() override;

 private:
  // collection of produced tracks
  std::vector<ldmx::TrigScintTrack> tracks_;

  // add a cluster to a track
  ldmx::TrigScintTrack makeTrack(std::vector<ldmx::TrigScintCluster> clusters);

  // match x, y tracks and set their x,y spatial coordinates
  void matchXYTracks(std::vector<ldmx::TrigScintTrack>& tracks);
  // std::vector<ldmx::TrigScintTrack> matchXYTracks(
  // std::vector<ldmx::TrigScintTrack> &tracks);

  // maximum difference (in channel number space) between track seed and cluster
  // in the next pad tolerated to form a track
  double max_delta_{0.};

  double max_delta_vert_{0.};
  double bar_length_y_{30.};

  // producer specific verbosity
  int verbose_{0};

  // collection used to seed the tracks
  std::string seeding_collection_;

  // other cluster collections used in track making
  std::vector<std::string> input_collections_;

  // output collection (tracks)
  std::string output_collection_;

  // specific pass name to use for track making
  std::string pass_name_{""};

  // allow forming tracks without match in the last collection
  bool skip_last_{false};

  // do tracking using LUT method instead of with max_delta
  bool lut_tracking_{false};

  // vertical bar start index
  int vert_bar_start_idx_{52};

  // number of horizontal bars (one layer) in geometry
  int n_bars_y_{16};

  // number of vertical bars (one row) in geometry
  int n_bars_x_{8};

  // track centroid in units of channel nb (will not be content weighted)
  // float centroid_{0.};

  // track horizontal centroid in units of channel nb (will not be content
  // weighted)
  // float centroidX_{-1};

  // track vertical centroid in units of channel nb (will not be content
  // weighted)
  // float centroidY_{-1};

  // track residual in units of channel nb (will not be content weighted)
  // float residual_{0.};

  struct LUTKey {
    float p1_, p2_, p3_;

    bool operator==(const LUTKey &other) const {
      return p1_ == other.p1_ && p2_ == other.p2_ && p3_ == other.p3_;
    }
  };

  struct LUTKeyHash {
    size_t operator()(const LUTKey &k) const {
      return std::hash<float>()(k.p1_) ^ (std::hash<float>()(k.p2_) << 1) ^
             (std::hash<float>()(k.p3_) << 2);
    }
  };

  std::unordered_set<LUTKey, LUTKeyHash> lut_;

  float bar_width_y_{3.};  // mm
  float bar_gap_y_{2.1};   // mm
  float bar_width_x_{3.};  // mm
  float bar_gap_x_{0.1};   // mm

  float x_conv_factor_;  // geometry conversion factors
  float x_start_;
  float y_conv_factor_;
  float y_start_;
};

}  // namespace trigscint

#endif  // TRIGSCINT_TRIGSCINTTRACKPRODUCER_H
