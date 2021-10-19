#ifndef _DQM_HCALPEDESTALS_H_
#define _DQM_HCALPEDESTALS_H_

//----------//
//   STL    //
//----------//
#include <algorithm>

//----------//
//   ROOT   //
//----------//
#include "TVector3.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

#include "Tools/AnalysisUtils.h"

namespace dqm {

class HCalPedestals : public framework::Analyzer {
 public:
  /** Constructor */
  HCalPedestals(const std::string& name, framework::Process& process) : framework::Analyzer(name, process) {}

  /** Destructor */
  ~HCalPedestals() {}

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

 private:
  std::string input_name_;
  std::string input_pass_;
};

}  // namespace dqm

#endif  // _DQM_HCALPedestals_H_
