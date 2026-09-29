/**
 * @file PileupFinder.h
 * @brief Simple PFlow algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef PARTICLEFLOW_H
#define PARTICLEFLOW_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace recon {

/**
 * @class PileupFinder
 * @brief
 */
class PileupFinder : public framework::Producer {
 public:
  PileupFinder(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onProcessEnd();

 private:
  // name of collections for PF input object to be passed
  std::string rec_hit_coll_name_;
  std::string rec_hit_pass_name_;
  std::string pf_cand_coll_name_;
  std::string pf_cand_pass_name_;
  std::string cluster_coll_name_;
  std::string cluster_pass_name_;
  // name of collection for pileup-free output hit coll
  std::string output_rec_hit_coll_name_;
  // configuration

  double min_mom_{0.};  // MeV
};
}  // namespace recon

#endif /* PILEUPFINDER_H */
