
#ifndef HCAL_MYCLUSTERWEIGHT_H_
#define HCAL_MYCLUSTERWEIGHT_H_

#include <iostream>

#include "Hcal/WorkingCluster.h"

namespace hcal {

class MyClusterWeight {
 public:
  double operator()(
      const WorkingCluster& a,
      const WorkingCluster& b) {  // returns weighting function, where smallest
                                  // weights will be combined first

    double rmol = 10.00;    // Moliere radius_ of detector, roughly. In mm TODO
    double dzchar = 100.0;  // lateral shower development in mm TODO

    double a_e = a.centroid().E();
    double a_x = a.centroid().Px();
    double a_y = a.centroid().Py();
    double a_z = a.centroid().Pz();

    double b_e = b.centroid().E();
    double b_x = b.centroid().Px();
    double b_y = b.centroid().Py();
    double b_z = b.centroid().Pz();

    double dijz;
    if (a_e >= b_e) {
      // differences in Z
      dijz = b_z - a_z;
    } else {
      dijz = a_z - b_z;
    }

    // Transverse Difference
    double dij_t = pow(pow(a_x - b_x, 2) + pow(a_y - b_y, 2), 0.5);
    // Trans --> massive
    double weight_t = exp(pow(dij_t / rmol, 2)) - 1;
    // Long
    double weight_z = (exp(abs(dijz) / dzchar) - 1);

    // Return the highest of the two weights
    if (weight_t <= weight_z) {
      return weight_z;
    } else {
      return weight_t;
    }
  }
};
}  // namespace hcal

#endif
