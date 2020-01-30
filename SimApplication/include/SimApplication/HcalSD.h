/**
 * @file HcalSD.h
 * @brief Class defining an HCal sensitive detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_HCALSD_H_
#define SIMAPPLICATION_HCALSD_H_

// LDMX
#include "SimApplication/CalorimeterSD.h"
#include "DetDescr/HcalID.h"

namespace ldmx {

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

            HcalSD(G4String name, G4String theCollectionName, int subdet, DetectorID* detID = new HcalID);

            virtual ~HcalSD();

            G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* /*unused*/);
            
        private:
          double birksc1_;
          double birksc2_;    
    };

}

#endif
