#ifndef _DQM_TRIGSCINTCLUSTER_DQM_H_
#define _DQM_TRIGSCINTCLUSTER_DQM_H_

//----------//
//   STL    //
//----------//
#include <algorithm>

//----------//
//   LDMX   //
//----------//
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Tools/AnalysisUtils.h"
#include "TrigScint/Event/TrigScintCluster.h"

namespace dqm {

class TrigScintClusterDQM : public framework::Analyzer {
 public:
  /** Constructor */
  TrigScintClusterDQM(const std::string &name, framework::Process &process);

  /** Destructor */
  ~TrigScintClusterDQM();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param pSet Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &pSet);

  /**
   * Process the event and make histograms ro summaries.
   *
   * @param event The event to analyze.
   */
  void analyze(const framework::Event &event);

  /** Method executed before processing of events begins. */
  void onProcessStart();

 private:
  /** Name of trigger pad cluster  collection. */
  std::string clusterCollectionName_{"TriggerPadUpClusters"};
  std::string padName_{"_up"};
  std::string passName_{""};
};

}  // namespace dqm

#endif  // _DQM_TRIGSCINTCLUSTER_DQM_H_
