#ifndef _DQM_TRIGSCINTTRACK_DQM_H_
#define _DQM_TRIGSCINTTRACK_DQM_H_

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
#include "TrigScint/Event/TrigScintTrack.h"

namespace dqm {

class TrigScintTrackDQM : public framework::Analyzer {
 public:
  /** Constructor */
  TrigScintTrackDQM(const std::string &name, framework::Process &process);

  /** Destructor */
  ~TrigScintTrackDQM();

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
  /** Name of trigger pad track  collection. */
  std::string trackCollectionName_{"TriggerPadTracks"};
  std::string passName_{""};
};

}  // namespace dqm

#endif  // _DQM_TRIGSCINTTRACK_DQM_H_
