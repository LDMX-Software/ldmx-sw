/**
 * @file EcalWABRecProcessor.h
 * @brief Class that reconstructs important kinematic variables for WAB studies
 * @author Sanjit Masanam, UCSB
 */

#ifndef EVENTPROC_ECALWABROCESSOR_H_
#define EVENTPROC_ECALWABROCESSOR_H_

// LDMX
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalWABResult.h"
#include "Eigen/Dense"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/ONNXRuntime.h"

// ROOT (MIP tracking)
#include "TVector3.h"

// C++
#include <map>
#include <memory>

namespace ecal {

class EcalWABRecProcessor : public framework::Producer {
 public:
  EcalWABRecProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~EcalWABRecProcessor() = default;

  void onProcessStart() override;

  void onProcessEnd() override;

  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

 private:
  std::string rec_pass_name_;
  std::string rec_coll_name_;
  std::string track_pass_name_;
  std::string track_coll_name_;
  int nevents_{0};
  double processing_time_{0.};

  std::tuple<Eigen::VectorXd, double, int, Eigen::MatrixXd, int>
  fit2DTracksConstrained(const std::vector<double>& x1,
                         const std::vector<double>& y1,
                         const std::vector<double>& s1,
                         const std::vector<double>& x2,
                         const std::vector<double>& y2,
                         const std::vector<double>& s2,
                         const std::vector<double>& guess, int maxIter,
                         int verbosity, double dchisq, double abs_lim);

  std::pair<Eigen::VectorXd, Eigen::VectorXd> polyfitXYvsZ(
      const std::vector<double>& x, const std::vector<double>& y,
      const std::vector<double>& z, int degree);

  /** Name of the collection which will contain the results. */
  std::string collection_name_{"EcalWABRec"};

  /// handle to current geometry (to share with member functions)
  const ldmx::EcalGeometry* geometry_;

};  // EcalWABRecProcessor

}  // namespace ecal

#endif