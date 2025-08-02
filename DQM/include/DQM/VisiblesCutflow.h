/*
 * @file VisiblesCutflow.h
 * @brief Class used to run full visibles analysis
 * @author Tyler Horoho, University of Virginia
 */

#ifndef DQM_VISIBLESCUTFLOW_H_
#define DQM_VISIBLESCUTFLOW_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/ONNXRuntime.h"

namespace dqm {

class VisiblesCutflow : public framework::Analyzer {
 public:
  VisiblesCutflow(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}

  ~VisiblesCutflow() override = default;

  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

 private:
  std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;
  double bdt_cut_val_{0.};
  std::string feature_list_name_;

  bool all_cuts_{true};
  
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
  std::string ecal_veto_collection_;
  std::string ecal_veto_pass_;
  double ecal_bdt_cut_val_{0.};

  bool inList(std::vector<int> parents, int track_id);
};

}  // namespace dqm

#endif
