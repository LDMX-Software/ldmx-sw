
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

    double rmol = 10.00;    // Moliere radius of detector, roughly. In mm TODO
    double dzchar = 100.0;  // lateral shower development in mm TODO

    double aE = a.centroid().E();
    double aX = a.centroid().Px();
    double aY = a.centroid().Py();
    double aZ = a.centroid().Pz();

    double bE = b.centroid().E();
    double bX = b.centroid().Px();
    double bY = b.centroid().Py();
    double bZ = b.centroid().Pz();

    double dijz;
    if (aE >= bE) {
      // differences in Z
      dijz = bZ - aZ;
    } else {
      dijz = aZ - bZ;
    }

    // Transverse Difference
    double dijT = pow(pow(aX - bX, 2) + pow(aY - bY, 2), 0.5);
    // Trans --> massive
    double weightT = exp(pow(dijT / rmol, 2)) - 1;
    // Long
    double weightZ = (exp(abs(dijz) / dzchar) - 1);

    // Return the highest of the two weights
    if (weightT <= weightZ) {
      return weightZ;
    } else {
      return weightT;
    }
  }
};
}  // namespace hcal

#endif
