/**
 * @file RecoilTrackerDQM.h
 * @brief Analyzer used for DQM of the Recoil tracker.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _DQM_RECOIL_TRACKER_DQM_H_
#define _DQM_RECOIL_TRACKER_DQM_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <unordered_map>
#include <utility>

//----------//
//   LDMX   //
//----------//
#include "Tools/AnalysisUtils.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

namespace dqm {

class RecoilTrackerDQM : public framework::Analyzer {
 public:
  /** Constructor */
  RecoilTrackerDQM(const std::string& name, framework::Process& process);

  /** Destructor */
  ~RecoilTrackerDQM();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  /**
   * Process the event and make histograms ro summaries.
   *
   * @param event The event to analyze.
   */
  void analyze(const framework::Event& event);

  /** Method executed before processing of events begins. */
  void onProcessStart();

};  // RecoilTrackerDQM

}  // namespace dqm

#endif  // _DQM_RECOIL_TRACKER_DQM_H_
