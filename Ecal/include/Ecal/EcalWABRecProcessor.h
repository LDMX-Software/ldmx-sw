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
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalWABResult.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/StraightTrack.h"

/*~~~~~~~~~~~*/
/*   Tools   */
/*~~~~~~~~~~~*/
#include "Eigen/Dense"

// C++
#include <stdlib.h>

#include <iomanip>
#include <map>
#include <memory>
#include <numbers>  // For std::numbers::pi
#include <numeric>

namespace ecal {

class EcalWABRecProcessor : public framework::Producer {
 public:
  EcalWABRecProcessor(const std::string& name, framework::Process& process)
      : Producer(name, process) {}

  virtual ~EcalWABRecProcessor() = default;

  void onProcessEnd() override;

  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

 private:
  std::string sp_pass_name_;
  std::string rec_pass_name_;
  std::string rec_coll_name_;
  std::string track_pass_name_;
  std::string track_coll_name_;
  int nevents_{0};
  float processing_time_{0.};

  std::tuple<Eigen::VectorXd, float, int, Eigen::MatrixXd, int>
  fit2DTracksConstrained(const std::vector<float>& x1,
                         const std::vector<float>& y1,
                         const std::vector<float>& s1,
                         const std::vector<float>& x2,
                         const std::vector<float>& y2,
                         const std::vector<float>& s2,
                         const std::vector<double>& guess, int maxIter,
                         int verbosity, float dchisq, float abs_lim);

  std::pair<Eigen::VectorXd, Eigen::VectorXd> polyfitXYvsZ(
      const std::vector<float>& x_, const std::vector<float>& y_,
      const std::vector<float>& z_, int degree);

  /** Name of the collection which will contain the results. */
  std::string collection_name_{"EcalWABRec"};

};  // EcalWABRecProcessor

}  // namespace ecal

#endif