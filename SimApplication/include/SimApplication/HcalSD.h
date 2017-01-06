/**
 * @file HcalSD.h
 * @brief Class defining an HCal sensitive detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_HCALSD_H_
#define SIMAPPLICATION_HCALSD_H_

// LDMX
#include "SimApplication/CalorimeterSD.h"

namespace sim {

/**
 * @class HcalSD
 * @brief HCal sensitive detector
 *
 * @note
 * This class basically doesn't do anything right now.
 *
 * @todo Add actual custom hit processing for HCal detector.
 */
class HcalSD : public CalorimeterSD {

    public:

        HcalSD(G4String name,
                G4String theCollectionName,
                int subdet,
                DetectorID* detID);

        virtual ~HcalSD();
};

}

#endif
