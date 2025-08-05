
#ifndef ECAL_MYCLUSTERWEIGHT_H_
#define ECAL_MYCLUSTERWEIGHT_H_

#include <iostream>

#include "Ecal/IntermediateCluster.h"

namespace ecal {

class MyClusterWeight {
 public:
  double operator()(const IntermediateCluster& a,
                    const IntermediateCluster&
                        b) {  // returns weighting function, where smallest
                              // weights will be combined first

    double rmol = 10.00;    // Moliere radius of detector, roughly. In mm
    double dzchar = 100.0;  // Characteristic cluster longitudinal variable TO
                            // BE DETERMINED! in mm

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
      dijz = b_z - a_z;
    } else {
      dijz = a_z - b_z;
    }

    double dij_t = pow(pow(a_x - b_x, 2) + pow(a_y - b_y, 2), 0.5);

    double weight_t = exp(pow(dij_t / rmol, 2)) - 1;
    double weight_z = (exp(std::abs(dijz) / dzchar) - 1);

    // Return the highest of the two weights
    if (weight_t <= weight_z) {
      return weight_z;
    } else {
      return weight_t;
    }
  }
};
}  // namespace ecal

#endif
