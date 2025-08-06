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
#include "Ecal/EcalHelper.h"
#include "Ecal/Event/EcalMipResult.h"
#include "Ecal/Event/EcalTrajectoryInfo.h"
#include "Ecal/Event/EcalVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

// C++ Standard Library
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// ROOT
#include "Math/Vector3D.h"
#include "TDecompSVD.h"
#include "TMatrixD.h"

namespace ecal {
class EcalMipTrackingProcessor : public framework::Producer {
 public:
  typedef std::pair<ldmx::EcalID, float> CellEnergyPair;

  using XYCoords = ldmx::XYCoords;

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

  using HitData = ldmx::HitData;

 private:
  void clearProcessor();

 private:
  int nevents_{0};
  double processing_time_{0.};

  std::map<std::string, double> profiling_map_;

  double linreg_radius_{0};

  int n_ecal_layers_{0};
  int n_readout_hits_{0};

  // MIP tracking
  /// Number of "straight" tracks found in the event
  int n_straight_tracks_{0};
  /// Number of "linreg" tracks found in the event
  int n_linreg_tracks_{0};
  /// Earliest ECal layer_ in which a hit is found near the projected photon
  /// trajectory
  int first_near_ph_layer_{0};
  /// Number of hits near the photon trajectory
  int n_near_ph_hits_{0};
  /// Number of hits in the photon territory
  int photon_territory_hits_{0};

  std::string ecal_collection_name_{"EcalVeto"};
  std::string ecal_pass_name_{""};
  std::string mip_collection_name_{"EcalTrajectoryInfo"};
  std::string mip_pass_name_{""};
  std::string mip_result_name_{"EcalMipResult"};

  /// handle to current geometry (to share with member functions)
  const ldmx::EcalGeometry* geometry_;
};

}  // namespace ecal

#endif
