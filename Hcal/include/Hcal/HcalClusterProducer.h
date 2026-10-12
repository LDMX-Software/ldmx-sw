/**
 * @file HcalClusterProducer.h
 * @brief Class that performs clustering of HCal hits_
 * @author Sophie Middleton, Caltech
 */

#ifndef HCAL_HCALCLUSTERPRODUCER_H_
#define HCAL_HCALCLUSTERPRODUCER_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace hcal {

/**
 * @class HcalClusterProducer
 * @brief Make clusters from hits in the HCAL
 */
class HcalClusterProducer : public framework::Producer {
 public:
  HcalClusterProducer(const std::string& name, framework::Process& process);

  virtual ~HcalClusterProducer() { ; }

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

 private:
  // double     EminSeed_{0.};
  double enoise_cut_{0.};
  double delta_time_{0};
  double delta_r_{0};
  double emin_cluster_{0.};
  double cut_off_{0.};
  std::string cluster_coll_name_;
  std::string hcal_hits_pass_name_;
};

}  // namespace hcal

#endif
