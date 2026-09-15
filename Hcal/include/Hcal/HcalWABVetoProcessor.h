/**
 * @file HcalWABVetoProcessor.h
 * @brief Processor that determines if an event is vetoed by the Hcal.
 * @author Sophie Middleton
 */

#ifndef __HCAL_HCAL_VETO_PROCESSOR_H__
#define __HCAL_HCAL_VETO_PROCESSOR_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <string>

//----------//
//   LDMX   //
//----------//
#include "Ecal/Event/EcalHit.h"
#include "Event/HcalCluster.h"
#include "Event/HcalHit.h"
#include "Event/HcalVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace hcal {

class HcalWABVetoProcessor : public framework::Producer {
 public:
  /** Constructor */
  HcalWABVetoProcessor(const std::string& name, framework::Process& process);

  /** Destructor */
  virtual ~HcalWABVetoProcessor() = default;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * Run the processor and create a collection of results which
   * indicate if the event passes/fails the Hcal veto.
   *
   * @param event The event to process.
   */
  void produce(framework::Event& event) override;

 private:
  // Maximum sum of total ECAL and HCAL energy
  double maxtotal_energy_compare_{1000.};
  // Minimum sum of total ECAL and HCAL energy
  double mintotal_energy_compare_{0.};
  // Maximum number of clusters in an event
  double maxn_clusters_{0.};
  // Maximum allowed mean average number of hits in the event's clusters
  double max_mean_hits_per_cluster_{0.};
  // Maimum allowed mean average energy in event's clusters
  double max_mean_energy_per_cluster_{0.};
  std::string output_coll_name_;
  std::string input_hcal_cluster_coll_name_;
  std::string input_hcal_hit_coll_name_;
  std::string input_ecal_hit_coll_name_;
  std::string hcal_hit_passname_;
  std::string ecal_hit_passname_;
  std::string hcal_cluster_passname_;

};  // HcalWABVetoProcessor
}  // namespace hcal

#endif  // HCAL_HcalWABVetoProcessor_H_
