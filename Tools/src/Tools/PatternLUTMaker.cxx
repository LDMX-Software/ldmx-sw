// Analyzer file which writes a LUT text file from input cluster text file (made
// in ClusterTripletMaker) containing only those cluster combination entries
// which occur with a frequency above the lut_threshold parameter

#include "Tools/PatternLUTMaker.h"

namespace tools {

PatternLUTMaker::PatternLUTMaker(const std::string& name,
                                 framework::Process& process)
    : Analyzer(name, process) {}

void PatternLUTMaker::configure(framework::config::Parameters& ps) {
  input_file_ = ps.get<std::string>("input_file");
  output_file_ = ps.get<std::string>("output_file");
  lut_threshold_ = ps.get<double>("lut_threshold");
  verbose_ = ps.get<int>("verbosity");

  // The LUT threshold is the minimum percentage of times that a track
  // must appear in a pool of events to be written to the LUT.
  // Example --- horizontal tracks whose pad 1-2 and pad 2-3 delta values
  //             are both 0 make up ~80% of tracks out of 10.000 events,
  //             while a track with deltas (+22,-22), meaning pad 1 cluster in
  //             e.g. bar 4, pad 2 cluster in bar 26, and pad 3 cluster in bar 4
  //             only appears once in 10.000 events (0.01%). The straight
  //             tracks are written to the LUT and the single "anomaly" is not.
  // Primary motivation here is for use in the case of an unknown TS
  // misalignment.

  ldmx_log(info) << "In PatternLUTMaker: configure done!" << std::endl;
  ldmx_log(info) << "Got parameters: \nInput file:   " << input_file_
                   << "\nOutput file:     " << output_file_
                   << "\nLUT threshold:     " << lut_threshold_
                   << "\nVerbosity:      " << verbose_;

  return;
}

void PatternLUTMaker::onProcessStart() {
  infile_.open(input_file_);
  outfile_.open(output_file_);

  if (total_lines_ > 0) return;
  int ev;
  float p1, p2, p3;
  while (infile_ >> ev >> p1 >> p2 >> p3) {
    float p12 = p2 - p1; //for each cluster combination, calculate vertical propagation
    float p23 = p3 - p2; //between pads 1 and 2 (p12) and between pads 2 and 3 (p23);
    groups_[{p12, p23}].push_back({ev, p1, p2, p3}); //and group by these values
    total_lines_++;                                 //(the "propagation patterns").
  }
  return;
}

void PatternLUTMaker::analyze(const framework::Event& event) {
}

void PatternLUTMaker::onProcessEnd() {
  ldmx_log(info) << "total_lines = " << total_lines_;
  ldmx_log(info) << "groups = " << groups_.size();
  int combs = 0; //combs will be the total number of patterns written to the LUT
  int tracks = 0; //and tracks the number of tracks contained within those pattern groups

  if (verbose_) {
    ldmx_log(info) << "Total number of cluster combinations: " << total_lines_ << "\n"
                   << "Number of different propagation patterns: " << groups_.size()
                   << "\n" << "LUT Threshold: " << lut_threshold_ * 100 << "%\n";
  }

  for (auto& g : groups_) {
    int count = g.second.size();
    double frac = static_cast<double>(count) / total_lines_;

    if (verbose_) {
      ldmx_log(debug) << "(" << g.first.first << "," << g.first.second << ") appears "
                      << count << " times, (" << frac * 100 << " %)" << "\n";
    }

    if (frac > lut_threshold_) {  // write to outfile_ (LUT file) if over threshold
        combs++;
        for (auto& line : g.second) {
          tracks++;
          outfile_ << line.p1_ << " " << line.p2_ << " " << line.p3_ << "\n";
        }
    }

  }

  if (verbose_) {
    ldmx_log(info) << "\nLUT textfile written." << "\n" << combs
                   << " combinations (" << tracks << " tracks) written to LUT.\n";
  }
  return;
}
}  // namespace tools

DECLARE_ANALYZER(tools::PatternLUTMaker)
