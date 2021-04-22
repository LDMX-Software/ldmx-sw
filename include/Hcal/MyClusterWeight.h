
#ifndef HCAL_MYCLUSTERWEIGHT_H_ 
#define HCAL_MYCLUSTERWEIGHT_H_

#include "Hcal/WorkingCluster.h"
#include <iostream>

namespace hcal {

    class MyClusterWeight {
    
        public:
    
            double operator()(const WorkingCluster& a, const WorkingCluster& b) { // returns weighting function, where smallest weights will be combined first

                double rmol = 10.00; //Moliere radius of detector, roughly. In mm TODO
                double dzchar = 100.0; // lateral shower development in mm TODO

                double aE = a.centroid().E();
                double aX = a.centroid().Px();
                double aY = a.centroid().Py();
                double aZ = a.centroid().Pz();

                double bE = b.centroid().E();
                double bX = b.centroid().Px();
                double bY = b.centroid().Py();
                double bZ = b.centroid().Pz();

                double dijz;
                double eFrac;
                if (aE >= bE) {
                    eFrac = bE/aE;  // ratio of energies
                    dijz = bZ-aZ; // differences in Z
                } else {
                    eFrac = aE/bE;
                    dijz = aZ-bZ;
                }

                double dijT = pow(pow(aX-bX,2) + pow(aY-bY,2),0.5); //Transverse Difference

                double weightT = exp(pow(dijT/rmol,2))-1; //Trans --> massive
                double weightZ = (exp(abs(dijz)/dzchar)-1); //Long

                //Return the highest of the two weights
                if (weightT <= weightZ) {
                    return weightZ;
                } else {
                    return weightT;
                }
            }
    };
}

#endif
