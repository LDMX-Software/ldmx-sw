
#ifndef ECAL_ECALCLUSTERWEIGHT_H_
#define ECAL_ECALCLUSTERWEIGHT_H_

#include <iostream>
#include "Event/EcalHit.h"

namespace ecal {

class EcalClusterWeight {
 public:
  double operator()(
      const ldmx::EcalHit& a,
      const ldmx::EcalHit& b) {  // returns weighting function, where smallest
                                  // weights will be combined first

    double rmol = 10.00;    // Moliere radius of detector, roughly. In mm
    double dzchar = 100.0;  // Characteristic cluster longitudinal variable TO
                            // BE DETERMINED! in mm

    double aE = a.getEnergy();
    double aX = a.getXPos();
    double aY = a.getYPos();
    double aZ = a.getZPos();

    double bE = b.getEnergy();
    double bX = b.getXPos();
    double bY = b.getYPos();
    double bZ = b.getZPos();


    double dijz;
    double eFrac;
    if (aE >= bE) {
      eFrac = bE / aE;
      dijz = bZ - aZ;
    } else {
      eFrac = aE / bE;
      dijz = aZ - bZ;
    }

    double dijT = pow(pow(aX - bX, 2) + pow(aY - bY, 2), 0.5);

    double weightT = exp(pow(dijT / rmol, 2)) - 1;
    double weightZ = (exp(abs(dijz) / dzchar) - 1);

    // Return the highest of the two weights
    if (weightT <= weightZ) {
      return weightZ;
    } else {
      return weightT;
    }
  }
};
}  // namespace ecal

#endif
