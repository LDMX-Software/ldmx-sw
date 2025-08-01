/*
 * @file VisiblesFeatureProducer.h
 * @brief Class used to get features for visibles BDT,
 *        and can save features to a .txt file.
 * @author Tyler Horoho, University of Virginia
 */

#ifndef HCAL_VISIBLESFEATUREPRODUCER_H_
#define HCAL_VISIBLESFEATUREPRODUCER_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace hcal {

class VisiblesFeatureProducer : public framework::Analyzer {
 public:
  VisiblesFeatureProducer(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}

  ~VisiblesFeatureProducer() override = default;

  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

 private:
  bool training_{false};
  std::string training_file_;

  double beam_energy_mev_{0.};

  std::string hcal_rec_collection_;
  std::string hcal_rec_pass_name_;
  std::string ecal_rec_collection_;
  std::string ecal_rec_pass_name_;
  bool recoil_from_tracking_{false};
  std::string track_collection_;
  std::string track_pass_name_;
  std::string sp_collection_;
  std::string sp_pass_name_;
  std::string sim_particles_pass_name_;

  bool inList(std::vector<int> parents, int track_id);
};

}  // namespace hcal

#endif
