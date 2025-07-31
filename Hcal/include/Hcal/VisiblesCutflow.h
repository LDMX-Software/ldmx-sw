#ifndef HCAL_VISIBLESCUTFLOW_H_
#define HCAL_VISIBLESCUTFLOW_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/ONNXRuntime.h"

namespace hcal {

class VisiblesCutflow : public framework::Analyzer {
 public:
  VisiblesCutflow(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}

  ~VisiblesCutflow() override = default;

  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

 private:
  std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;
  double bdtCutVal_{0.};
  std::string featureListName_;

  double beamEnergyMeV_{0.};

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
  double ecalbdtCutVal_{0.};

  bool in_list(std::vector<int> parents, int a);
};

}  // namespace hcal

#endif
