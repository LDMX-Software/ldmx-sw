/**
 * @file MyClusterWeight.h
 * @brief Weight function for Hcal cluster merging decisions
 */
#ifndef HCAL_MYCLUSTERWEIGHT_H_
#define HCAL_MYCLUSTERWEIGHT_H_

#include <cmath>

#include "Hcal/Event/HcalHit.h"
#include "Recon/WorkingCluster.h"

namespace hcal {

/**
 * @class MyClusterWeight
 * @brief Computes the weight (distance) between two Hcal clusters.
 *
 * The weight is used by TemplatedClusterFinder to decide which clusters
 * to merge. Smaller weights indicate clusters that should be merged first.
 * The weight is based on both transverse and longitudinal separation.
 */
class MyClusterWeight {
 public:
  using ClusterType = recon::WorkingCluster<ldmx::HcalHit>;

  /**
   * Compute the weight between two clusters.
   *
   * @param a First cluster
   * @param b Second cluster
   * @return Weight value (smaller = should merge first)
   */
  double operator()(const ClusterType& a, const ClusterType& b) {
    // Moliere radius of detector, roughly. In mm (TODO: tune for Hcal)
    double rmol = 10.00;
    // Lateral shower development in mm (TODO: tune for Hcal)
    double dzchar = 100.0;

    double a_e = a.energy();
    double a_x = a.centroidX();
    double a_y = a.centroidY();
    double a_z = a.centroidZ();

    double b_e = b.energy();
    double b_x = b.centroidX();
    double b_y = b.centroidY();
    double b_z = b.centroidZ();

    double dijz;
    if (a_e >= b_e) {
      dijz = b_z - a_z;
    } else {
      dijz = a_z - b_z;
    }

    // Transverse difference
    double dij_t = std::sqrt(std::pow(a_x - b_x, 2) + std::pow(a_y - b_y, 2));

    // Transverse weight
    double weight_t = std::exp(std::pow(dij_t / rmol, 2)) - 1;
    // Longitudinal weight
    double weight_z = std::exp(std::abs(dijz) / dzchar) - 1;

    // Return the highest of the two weights
    return std::max(weight_t, weight_z);
  }
};

}  // namespace hcal

#endif  // HCAL_MYCLUSTERWEIGHT_H_
