#ifndef _DQM_HCALCALIBRATION_H_
#define _DQM_HCALCALIBRATION_H_

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

class HcalCalibration : public framework::Analyzer {
 public:
  /** Constructor */
  HcalCalibration(const std::string& name, framework::Process& process) : framework::Analyzer(name, process) {}

  /** Destructor */
  ~HcalCalibration() {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  void onProcessStart() final override;

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

#endif  // _DQM_HcalCalibration_H_
