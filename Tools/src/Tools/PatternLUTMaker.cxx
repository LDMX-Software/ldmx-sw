//Analyzer file which writes a LUT text file from input cluster text file (made in 
//ClusterTripletMaker) containing only those cluster combination entries which 
//occur with a frequency above the lut_threshold parameter

#include "TrigScint/PatternLUTMaker.h"

namespace trigscint {

PatternLUTMaker::PatternLUTMaker(const std::string& name,
                                 framework::Process& process) 
    : Analyzer(name, process) {}

void PatternLUTMaker::configure(framework::config::Parameters& ps) {
  input_collection_ = ps.get<std::string>("input_collection");
  output_collection_ = ps.get<std::string>("output_collection");
  lut_threshold_ = ps.get<double>("lut_threshold");
  verbose_ = ps.get<int>("verbosity");
  
        //The LUT threshold is the minimum percentage of times that a track 
        //must appear in a pool of events to be written to the LUT. 
        //Example --- horizontal tracks whose pad 1-2 and pad 2-3 delta values 
        //            are both 0 make up ~80% of tracks out of 10.000 events,
        //            while a track with deltas (+22,-22), meaning pad 1 cluster in 
        //            e.g. bar 4, pad 2 cluster in bar 26, and pad 3 cluster in bar 4
        //            only appears once in 10.000 events (0.01%). The straight 
        //            tracks are written to the LUT and the single "anomaly" is not.
        //Primary motivation here is for use in the case of an unknown TS misalignment. 
        
  if (verbose_) {
    ldmx_log(info) << "In PatternLUTMaker: configure done!" << std::endl;
    ldmx_log(info) << "Got parameters: \nInput file:   " << input_collection_
                   << "\nOutput file:     " << output_collection_
                   << "\nLUT threshold:     " << lut_threshold_ 
                   << "\nVerbosity:      " << verbose_;
  }
  return;
}

void PatternLUTMaker::onProcessStart() {
  infile.open(input_collection_);
  outfile.open(output_collection_);
  return;
}

void PatternLUTMaker::analyze(const framework::Event& event) {
  if (totalLines > 0) return;

  int ev;
  float p1, p2, p3;

  while (infile >> ev >> p1 >> p2 >> p3) {
    float p12 = p2 - p1;
    float p23 = p3 - p2;

    groups[{p12, p23}].push_back({ev, p1, p2, p3});

    totalLines++;
  }

  return;
}

void PatternLUTMaker::onProcessEnd() {
  int combs = 0;
  int tracks = 0;

  if(verbose_){
    ldmx_log(info) << "Total number of track candidates: "
                   << totalLines << "\n"
                   << "Number of track candidate types: "
                   << groups.size() << "\n"
                   << "LUT Threshold: "
                   << lut_threshold_ * 100 << "%\n";
  }

  for (auto& g : groups) {
    float p12 = g.first.first;
    float p23 = g.first.second;
    int count = g.second.size();

    double frac = static_cast<double>(count) / totalLines;

    if (verbose_) {
      ldmx_log(info) << "(" << p12 << "," << p23 << ") appears " 
                     << count << " times, (" << frac * 100 << " %)" 
                     << "\n";
    }
  }

  for (auto& g : groups) {
    int count = g.second.size();

    double frac = static_cast<double>(count) / totalLines;

    if (frac > lut_threshold_) { //write to outfile if over threshold
      combs++;

      for (auto& line : g.second) {
        tracks++;

        outfile << line.p1 << " "
                << line.p2 << " "
                << line.p3 << "\n";
      }
    }
  }

  if (verbose_) {
    ldmx_log(info) << "\nLUT textfile written."
                   << "\n" << combs << " combinations ("
                   << tracks << " tracks) written to LUT.\n";
  }

  return;
}

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::PatternLUTMaker)
