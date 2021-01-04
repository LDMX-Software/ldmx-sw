/**
 * @file TrigScintDQM.h
 * @brief Analyzer used for TrigScint DQM.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef _DQM_TRIGSCINT_DQM_H_
#define _DQM_TRIGSCINT_DQM_H_

//----------//
//   STL    //
//----------//
#include <algorithm>

//----------//
//   ROOT   //
//----------//
#include "TVector3.h"

//----------//
//   LDMX   //
//----------//
#include "DetDescr/TrigScintID.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Tools/AnalysisUtils.h"

namespace ldmx {

class TrigScintDQM : public Analyzer {
 public:
  /** Constructor */
  TrigScintDQM(const std::string &name, Process &process);

  /** Destructor */
  ~TrigScintDQM();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param pSet Set of parameters used to configure this processor.
   */
  void configure(Parameters &pSet);

  /**
   * Process the event and make histograms ro summaries.
   *
   * @param event The event to analyze.
   */
  void analyze(const Event &event);

  /** Method executed before processing of events begins. */
  void onProcessStart();

 private:
  /// Name of trigger pad hit  collection.
  std::string hitCollectionName_{"TriggerPadUpSimHits"};

  /// Name of Pad
  std::string padName_{"_up"};
};

}  // namespace ldmx

#endif  // _DQM_TRIGSCINT_DQM_H_
