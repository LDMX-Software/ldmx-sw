/**
 * @file EcalRecoilRemovalProcessor.h
 * @brief Class that discards Ecal reconstructed hits from the recoil electron
 * for WAB event processing.
 * @author Oscar Lewis (UCSB)
 */

#ifndef EVENTPROC_ECALRECOILREMOVALPROCESSOR_H_
#define EVENTPROC_ECALRECOILREMOVALPROCESSOR_H_

// LDMX
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "Ecal/EcalHelper.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalTrajectoryInfo.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tools/AnalysisUtils.h"

// C++
#include <stdlib.h>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <map>

namespace ecal {

/**
 * @class EcalRecoilRemovalProcessor
 * @brief Discards Ecal reconstructed hits from the recoil electron for WAB
 * event processing.
 */
class EcalRecoilRemovalProcessor : public framework::Producer {
 public:
  using XYCoords = ldmx::XYCoords;

  EcalRecoilRemovalProcessor(const std::string& name,
                             framework::Process& process)
      : Producer(name, process) {}

  virtual ~EcalRecoilRemovalProcessor() {}

  /**
   * onNewRun is the first function called for each processor
   * *after* the conditions are fully configured and accessible.
   * This is where you could create single-processors, multi-event
   * calculation objects.
   */
  void onNewRun(const ldmx::RunHeader& rh) override;

  /**
   *
   */
  void onProcessEnd() override;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

 private:
  std::vector<XYCoords> getTrajectory(std::array<float, 3> momentum,
                                      std::array<float, 3> position);

 private:
  int nevents_{0};
  float processing_time_{0.};
  std::map<std::string, float> profiling_map_;

  // Number of electrons in the event; TODO: replace with ElectronCounter
  int n_electrons_{1};

  std::vector<float> ecal_layer_edep_raw_;
  std::vector<float> ecal_layer_edep_readout_;
  std::vector<float> ecal_layer_time_;

  std::vector<std::vector<float>> rem_dist_values_;

  // ldmx-sw pass parameters
  float beam_energy_mev_{0.};
  int num_ecal_layers_;
  std::string rem_dist_file_name_;

  // boolean parameters
  bool recoil_from_tracking_;

  // pass and collection names
  std::string ecal_sim_pass_name_;
  std::string ecal_sp_hits_pass_name_;
  std::string rec_coll_name_;
  std::string rec_pass_name_;
  std::string track_coll_name_;
  std::string track_pass_name_;

  /** Name of the collection which will containt the results. */
  std::string collection_name_included_;
  std::string collection_name_excluded_;

  // handle to current geometry (to share with member functions)
  const ldmx::EcalGeometry* geometry_;
};

}  // namespace ecal

#endif