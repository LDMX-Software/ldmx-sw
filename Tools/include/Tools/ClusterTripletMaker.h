/**
 * @file ClusterTripletMaker.h
 * @brief Writes cluster combinations to text file
 * @author Lucia Kvarnstrom, Lund University
 */

#ifndef TRIGSCINT_CLUSTERTRIPLETMAKER_H
#define TRIGSCINT_CLUSTERTRIPLETMAKER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

// TrigScint
#include <fstream>
#include <string>
#include <vector>

#include "TrigScint/Event/TrigScintCluster.h"

namespace trigscint {

/**
 * @class ClusterTripletMaker
 * @brief Write trigger scintillator cluster triplets to a text file
 */

class ClusterTripletMaker : public framework::Analyzer {
 public:
  ClusterTripletMaker(const std::string& name, framework::Process& process);

  virtual ~ClusterTripletMaker() = default;

  void configure(framework::config::Parameters& ps) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  // verbosity
  int verbose_{0};

  // specific pass name
  std::string pass_name_{""};

  // input cluster collections
  std::vector<std::string> cluster_input_collections_;

  // output text file
  std::string output_collection_{"clusters.txt"};

  // output stream
  std::ofstream output_stream_;
};

}  // namespace trigscint

#endif  // TRIGSCINT_CLUSTERTRIPLETMAKER_H
