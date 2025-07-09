
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
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tools/AnalysisUtils.h"
#include "Tools/ONNXRuntime.h"
#include "Tracking/Event/Track.h"

// C++
#include <stdlib.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>

// ROOT (for anle calculations)
#include "TVector3.h"

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
      const std::vector<ldmx::EcalHit>& ecalRecHits, float& showerRMS);

  /* Function to load up empty vector of hit maps */
  void fillHitMap(const std::vector<ldmx::EcalHit>& ecalRecHits,
                  std::map<ldmx::EcalID, float>& cellMap_);

  /* Function to take loaded hit maps and find isolated hits in them */
  void fillIsolatedHitMap(const std::vector<ldmx::EcalHit>& ecalRecHits,
                          ldmx::EcalID globalCentroid,
                          std::map<ldmx::EcalID, float>& cellMap,
                          std::map<ldmx::EcalID, float>& cellMapIso,
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
  float distTwoLines(TVector3 v1, TVector3 v2, TVector3 w1, TVector3 w2);
  /**
   * Return the minimum distance between the point h1 and the line passing
   * through points p1 and p2.
   *
   * @param[in] h1 Point to find the distance to
   * @param[in] p1 An arbitrary point on the line
   * @param[in] p2 A second, distinct point on the line
   * @returns Minimum distance between h1 and the line
   */
  float distPtToLine(TVector3 h1, TVector3 p1, TVector3 p2);

  /**
   * Return a vector of parameters for a propagated recoil track
   * @param[in] tracks The track collection
   * @param[in] ts_type The track state type, i.e. tracks state at the ECAL face
   * @param[in] ts_title The track state title, most likely "ecal"
   * @returns Vector of parameters for a propagated recoil track
   */
  std::vector<float> trackProp(const ldmx::Tracks& tracks,
                               ldmx::TrackStateType ts_type,
                               const std::string& ts_title);

 private:
  int nevents_{0};
  float processing_time_{0.};

  std::map<std::string, float> profiling_map_;
  std::map<ldmx::EcalID, float> cellMap_;
  std::map<ldmx::EcalID, float> cellMapTightIso_;

  std::vector<float> ecalLayerEdepRaw_;
  std::vector<float> ecalLayerEdepReadout_;
  std::vector<float> ecalLayerTime_;

  std::vector<std::vector<float>> roc_range_values_;

  int nEcalLayers_{0};
  int nReadoutHits_{0};
  int deepestLayerHit_{0};

  float summedDet_{0};
  float summedTightIso_{0};
  float maxCellDep_{0};
  float showerRMS_{0};
  float xStd_{0};
  float yStd_{0};
  float avgLayerHit_{0};
  float stdLayerHit_{0};
  float ecalBackEnergy_{0};

  /// Number of hits outside of the electron roc in the Ecal
  /// or if the electron trajectory is missing, all the hits in the Ecal
  int n_tracking_hits_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as projected at ECAL
  float epAng_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as at Target
  float epAngAtTarget_{0};
  /// Distance between the projected photon and electron trajectories at the
  /// ECal face
  float epSep_{0};
  /// Dot product of the photon and electron momenta unit vectors at Ecal
  float epDot_{0};
  /// Dot product of the photon and electron momenta unit vectors at Target
  float epDotAtTarget_{0};

  float bdtCutVal_{0};

  float beamEnergyMeV_{0};

  std::string bdtFileName_;
  std::string rocFileName_;
  std::vector<float> bdtFeatures_;
  std::string featureListName_;

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
  std::string collectionName_{"EcalVeto"};

  std::unique_ptr<ldmx::Ort::ONNXRuntime> rt_;

  /// handle to current geometry (to share with member functions)
  const ldmx::EcalGeometry* geometry_;
};

}  // namespace ecal

#endif
