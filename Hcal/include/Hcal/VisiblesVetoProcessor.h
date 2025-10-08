/*
 * @file VisiblesVetoProcessor.h
 * @brief Class that determines if activity in the Hcal looks like a LLP
 * @author Tyler Horoho, University of Virginia
 */

#ifndef EVENTPROC_VISIBLESVETOPROCESSOR_H_
#define EVENTPROC_VISIBLESVETOPROCESSOR_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/VisiblesVetoResult.h"
#include "Tools/ONNXRuntime.h"

// recoil tracking
#include "Tracking/Event/Track.h"

namespace hcal {

class VisiblesVetoProcessor : public framework::Producer {
 public:
  VisiblesVetoProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~VisiblesVetoProcessor() {}

  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

 private:
  void clearProcessor();

  void buildBDTFeatureVector(const ldmx::VisiblesVetoResult& result);

  /* a function for finding track IDs for truth-level tracking */
  bool inList(std::vector<int> parents, int track_id);

  int n_layers_hit_{0};
  double x_std_{0};
  double y_std_{0};
  double z_std_{0};
  double x_mean_{0};
  double y_mean_{0};
  double r_mean_{0};
  int iso_hits_{0};
  double iso_energy_{0};
  int n_readout_hits_{0};
  double summed_det_{0};
  double r_mean_from_photon_track_{0};

  double bdt_cut_val_{0};

  double beam_energy_mev_{0};

  std::vector<float> bdt_features_;
  std::string feature_list_name_;

  // Pass and collection names
  std::string rec_pass_name_;
  std::string rec_coll_name_;
  bool recoil_from_tracking_;
  std::string track_pass_name_;
  std::string track_collection_;
  std::string sp_collection_;
  std::string sp_pass_name_;
  std::string sim_particles_pass_name_;

  std::string collection_name_{"VisiblesVeto"};

  std::unique_ptr<ldmx::ort::ONNXRuntime> rt_;
};

}  // namespace hcal

#endif
