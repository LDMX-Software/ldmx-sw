/**
 * @file EcalMipTrackingProcessor.h
 * @brief Class that determines MIP tracking information using ECAL hit
 * information
 * @author Owen Colegrove, Danyi Zhang, Jihoon Yoo, Tamas Vami (UCSB)
 */

#ifndef EVENTPROC_ECALMIPTRACKINGPROCESSOR_H_
#define EVENTPROC_ECALMIPTRACKINGPROCESSOR_H_

// LDMX
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalTrajectoryInfo.h"
// #include "Ecal/Event/EcalVetoResult.h"
#include "Ecal/Event/EcalMipResult.h"
#include "DetDescr/SimSpecialID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/EventConstants.h"
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
#include <iostream>
#include <map>
#include <memory>
#include <utility>

// ROOT (MIP tracking)
#include "TDecompSVD.h"
#include "TMatrixD.h"
#include "TObject.h"
#include "TVector3.h"

namespace ecal {
class EcalMipTrackingProcessor : public framework::Producer {
 public:
  typedef std::pair<ldmx::EcalID, float> CellEnergyPair;

  typedef std::pair<float, float> XYCoords;

  EcalMipTrackingProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~EcalMipTrackingProcessor() = default;

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

  // MIP tracking:  Class for storing hit information for tracking in a
  // convenient way
  struct HitData {
    int layer;
    TVector3 pos;
  };

 private:
  void clearProcessor();

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

 private:
  int nevents_{0};
  double processing_time_{0.};

  std::map<std::string, double> profiling_map_;

  double linreg_radius_{0};
  bool verbose_{false};
  int nEcalLayers_{0};
  int nReadoutHits_{0};

  // MIP tracking
  /// Number of "straight" tracks found in the event
  int nStraightTracks_{0};
  /// Number of "linreg" tracks found in the event
  int nLinregTracks_{0};
  /// First ECal layer in which a hit is found near the photon
  int firstNearPhLayer_{0};
  /// Number of hits near the photon trajectory
  int nNearPhHits_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// (currently unused)
  float epAng_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as at Target
  float epAngAtTarget_{0};
  /// Distance between the projected photon and electron trajectories at the
  /// ECal face
  float epSep_{0};
  /// Dot product of the photon and electron momenta unit vectors
  float epDot_{0};
  /// Dot product of the photon and electron momenta unit vectors at Target
  float epDotAtTarget_{0};
  /// Number of hits in the photon territory
  int photonTerritoryHits_{0};

  std::string mip_collection_name_{"EcalTrajectoryInfo"};
  std::string mip_pass_name_{""};
  std::string mip_result_name_{"EcalMipResult"};

  /// handle to current geometry (to share with member functions)
  const ldmx::EcalGeometry* geometry_;
};

}  // namespace ecal

#endif