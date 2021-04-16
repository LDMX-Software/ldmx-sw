#ifndef _DQM_TRIGSCINTHIT_DQM_H_
#define _DQM_TRIGSCINTHIT_DQM_H_

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
#include "TrigScint/Event/TrigScintHit.h"

namespace dqm {

class TrigScintHitDQM : public framework::Analyzer {
 public:
  /** Constructor */
  TrigScintHitDQM(const std::string &name, framework::Process &process);

  /** Destructor */
  ~TrigScintHitDQM();

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
  /** Name of trigger pad hit  collection. */
  std::string hitCollectionName_{"TriggerPadUpDigiHits"};
  std::string padName_{"_up"};
};

}  // namespace dqm

#endif  // _DQM_TRIGSCINTHIT_DQM_H_
