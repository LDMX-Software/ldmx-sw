#ifndef _DQM_HCAL_DQM_H_
#define _DQM_HCAL_DQM_H_

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
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"
#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalVetoResult.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Tools/AnalysisUtils.h"
#include <map>
namespace dqm {

class HCalDQM : public framework::Analyzer {
public:
  /** Constructor */
  HCalDQM(const std::string &name, framework::Process &process);

  /** Destructor */
  ~HCalDQM() {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Process the event and make histograms ro summaries.
   *
   * @param event The event to analyze.
   */
  void analyze(const framework::Event &event) override;

  bool skipHit(const ldmx::HcalID &id) {
    const auto section{id.section()};
    return (section != section_ && section_ != -1);
  }
  void analyzeRecHits(const std::vector<ldmx::HcalHit> &hits);
  void analyzeSimHits(const std::vector<ldmx::SimCalorimeterHit> &hits);

  bool hitPassesVeto(const ldmx::HcalHit &hit, int section) {
    if (hit.getPE() < pe_veto_threshold || hit.getTime() > max_hit_time_) {
      return true;
    }
    if (section == ldmx::HcalID::HcalSection::BACK && hit.getMinPE() < 1) {
      return true;
    }
    return false;
  }

private:
  /// Hcal Sim Hits collection name
  std::string sim_coll_name_;

  /// Hcal Sim Hits pass name
  std::string sim_pass_name_;
  /// Hcal Rec Hits collection name
  std::string rec_coll_name_;

  /// Hcal Rec Hits pass name
  std::string rec_pass_name_;

  /// Hcal Veto name
  std::string veto_name_;

  /// Hcal Veto pass name
  std::string veto_pass_;

  // Veto threshold for photo-electrons
  float pe_veto_threshold;
  int section_;
  double max_hit_time_;
};

} // namespace dqm

#endif // _DQM_HCAL_DQM_H_
