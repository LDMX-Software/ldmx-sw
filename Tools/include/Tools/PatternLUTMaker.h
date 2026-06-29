/**
 * @file PatternLUTMaker.h
 * @brief Writes LUT based on frequency of track propagation patterns for
 * LUT-based TS tracking.
 * @author Lucia Kvarnström, Lund University
 */

#ifndef TOOLS_PATTERNLUTMAKER_H
#define TOOLS_PATTERNLUTMAKER_H
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace tools {

class PatternLUTMaker : public framework::Analyzer {
 public:
  struct Line {
    int event_;
    float p1_, p2_, p3_;
  };

  PatternLUTMaker(const std::string& name, framework::Process& process);

  virtual ~PatternLUTMaker() = default;

  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  // verbosity
  int verbose_{0};

  // input text file of clusters
  std::string input_file_;

  // output LUT file name
  std::string output_file_;

  // minimum frequency of a specific pattern to be written to the LUT
  double lut_threshold_{0.0008};

  std::ifstream infile_;
  std::ofstream outfile_;

  // to group cluster combinations by track pattern
  std::map<std::pair<float, float>, std::vector<Line>> groups_;

  //total number of lines in input file
  int total_lines_{0};
};

}  // namespace tools

#endif /* TOOLS_PATTERNLUTMAKER_H */
