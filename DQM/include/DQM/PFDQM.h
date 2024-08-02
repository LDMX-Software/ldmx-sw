#ifndef _DQM_PF_DQM_H_
#define _DQM_PF_DQM_H_
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
#include <map>

#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"
#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalVetoResult.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tools/AnalysisUtils.h"
namespace dqm {

class PFDQM : public framework::Analyzer {
 public:
  /** Constructor */
  PFDQM(const std::string &name, framework::Process &process);

  /** Destructor */
  ~PFDQM() {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) override;

  /**
   * Process the event and make histograms ro summaries.
   *
   * @param event The event to analyze.
   */
  void analyze(const framework::Event &event) override;

  // bool skipHit(const ldmx::HcalID &id) {
  //   const auto section{id.section()};
  //   return (section != section_ && section_ != -1);
  // }
  // void analyzeRecHits(const std::vector<ldmx::HcalHit> &hits);
  // void analyzeSimHits(const std::vector<ldmx::SimCalorimeterHit> &hits);

  // bool hitPassesVeto(const ldmx::HcalHit &hit, int section) {
  //   if (hit.getPE() < pe_veto_threshold || hit.getTime() > max_hit_time_) {
  //     return true;
  //   }
  //   if (section == ldmx::HcalID::HcalSection::BACK && hit.getMinPE() < 1) {
  //     return true;
  //   }
  //   return false;
  // }

 private:
  /// Rec Hits collection names
  std::string hcal_rec_coll_name_;
  std::string hcal_rec_pass_name_;
  std::string ecal_rec_coll_name_;
  std::string ecal_rec_pass_name_;
  std::string hcal_sim_coll_name_;
  std::string hcal_sim_pass_name_;
  std::string ecal_sim_coll_name_;
  std::string ecal_sim_pass_name_;
};

}  // namespace dqm

#endif  // _DQM_PF_DQM_H_
