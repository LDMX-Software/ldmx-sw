
/**
 * @file EcalVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information
 * @author Owen Colegrove, Danyi Zhang, Tamas Vami (UCSB)
 */

#ifndef EVENTPROC_ECALVETOPROCESSOR_H_
#define EVENTPROC_ECALVETOPROCESSOR_H_

// LDMX
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalTrajectoryInfo.h"
#include "Ecal/Event/EcalVetoResult.h"
#include "Ecal/TrackPropagator.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tools/AnalysisUtils.h"
#include "Tools/ONNXRuntime.h"

// C++
#include <stdlib.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>

// ROOT (for angle calculations)
#include "Math/Vector3D.h"

namespace ecal {

/**
 * @class EcalVetoProcessor
 * @brief Determines if event is vetoable using ECAL hit information
 */
class EcalVetoProcessor : public framework::Producer {
 public:
  typedef std::pair<ldmx::EcalID, float> CellEnergyPair;

  typedef std::pair<float, float> XYCoords;

  EcalVetoProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~EcalVetoProcessor() {}

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
  void clearProcessor();

  /* Function to calculate the energy weighted shower centroid */
  ldmx::EcalID GetShowerCentroidIDAndRMS(
      const std::vector<ldmx::EcalHit>& ecal_rec_hits, float& shower_rms);

  /* Function to load up empty vector of hit maps */
  void fillHitMap(const std::vector<ldmx::EcalHit>& ecal_rec_hits,
                  std::map<ldmx::EcalID, float>& cell_map_);

  /* Function to take loaded hit maps and find isolated hits in them */
  void fillIsolatedHitMap(const std::vector<ldmx::EcalHit>& ecal_rec_hits,
                          ldmx::EcalID global_centroid,
                          std::map<ldmx::EcalID, float>& cell_map,
                          std::map<ldmx::EcalID, float>& cell_map_iso,
                          bool doTight = false);

  std::vector<XYCoords> getTrajectory(std::array<float, 3> momentum,
                                      std::array<float, 3> position);

  void buildBDTFeatureVector(const ldmx::EcalVetoResult& result);

  // MIP tracking
  /**
   * Returns the distance between the lines v and w, with v defined to pass
   * through the points (v1,v2) (and similarly for w).
   *
   * @param[in] v1 An arbitrary point on line v
   * @param[in] v2 A second, distinct point on line v
   * @param[in] w1 An arbitrary point on line w
   * @param[in] w2 A second, distinct point on line w
   * @returns Closest distance of approach of lines u and v
   */
  float distTwoLines(ROOT::Math::XYZVector v1, ROOT::Math::XYZVector v2, ROOT::Math::XYZVector w1, ROOT::Math::XYZVector w2);
  /**
   * Return the minimum distance between the point h1 and the line passing
   * through points p1 and p2.
   *
   * @param[in] h1 Point to find the distance to
   * @param[in] p1 An arbitrary point on the line
   * @param[in] p2 A second, distinct point on the line
   * @returns Minimum distance between h1 and the line
   */
  float distPtToLine(ROOT::Math::XYZVector h1, ROOT::Math::XYZVector p1, ROOT::Math::XYZVector p2);

 private:
  int nevents_{0};
  float processing_time_{0.};

  std::map<std::string, float> profiling_map_;
  std::map<ldmx::EcalID, float> cell_map_;
  std::map<ldmx::EcalID, float> cell_map_tight_iso_;

  std::vector<float> ecal_layer_edep_raw_;
  std::vector<float> ecal_layer_edep_readout_;
  std::vector<float> ecal_layer_time_;

  std::vector<std::vector<float>> roc_range_values_;

  int n_ecal_layers_{0};
  int n_readout_hits_{0};
  int deepest_layer_hit_{0};

  float summed_det_{0};
  float summed_tight_iso_{0};
  float max_cell_dep_{0};
  float shower_rms_{0};
  float x_std_{0};
  float y_std_{0};
  float avg_layer_hit_{0};
  float std_layer_hit_{0};
  float ecal_back_energy_{0};

  /// Number of hits outside of the electron roc in the Ecal
  /// or if the electron trajectory is missing, all the hits in the Ecal
  int n_tracking_hits_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as projected at ECAL
  float ep_ang_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as at Target
  float ep_ang_at_target_{0};
  /// Distance between the projected photon and electron trajectories at the
  /// ECal face
  float ep_sep_{0};
  /// Dot product of the photon and electron momenta unit vectors at Ecal
  float ep_dot_{0};
  /// Dot product of the photon and electron momenta unit vectors at Target
  float ep_dot_at_target_{0};

  float bdt_cut_val_{0};

  float beam_energy_mev_{0};

  std::string bdt_file_name_;
  std::string roc_file_name_;
  std::vector<float> bdt_features_;
  std::string feature_list_name_;

  // Pass and collection names
  std::string sp_pass_name_;
  std::string rec_pass_name_;
  std::string rec_coll_name_;
  bool recoil_from_tracking_;
  std::string track_pass_name_;
  std::string track_collection_;

  std::string sim_particles_passname_;
  bool inverse_skim_{false};

  /** Name of the collection which will containt the results. */
  std::string collection_name_{"EcalVeto"};

  std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;

  /// handle to current geometry (to share with member functions)
  const ldmx::EcalGeometry* geometry_;
};

}  // namespace ecal

#endif
