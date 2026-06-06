/**
 * @file PatternLUTMaker.h
 * @brief Writes LUT based on frequency of track propagation patterns for LUT-based TS tracking.
 * @author Lucia Kvarnstrom, Lund University
 */

#ifndef TRIGSCINT_PATTERNLUTMAKER_H
#define TRIGSCINT_PATTERNLUTMAKER_H
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace trigscint {

class PatternLUTMaker : public framework::Analyzer {
 public:
  struct Line {
    int event;
    float p1, p2, p3;
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
  std::string input_collection_;
  
  //output LUT file name
  std::string output_collection_;
  
  //minimum frequency of a specific pattern to be written to the LUT
  double lut_threshold_{0.0008};

  std::ifstream infile;
  std::ofstream outfile;

  // to group cluster combinations by track pattern
  std::map<std::pair<float, float>, std::vector<Line>> groups;

  int totalLines{0};
};

}  // namespace trigscint

#endif /* TRIGSCINT_PATTERNLUTMAKER_H */
