#ifndef SIMAPPLICATION_HCALSD_H
#define SIMAPPLICATION_HCALSD_H

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "SimApplication/CalorimeterSD.h"


// Forward declarations
class G4Step;
class G4TouchableHistory; 

namespace ldmx {

    
    /**
     * Class defining a sensitive detector of type HCal. 
     */
    class HcalSD : public CalorimeterSD {

        public:


            /**
             * Class constructor.
             *
             * @param[in] name The namme of the sensitive detector.
             * @param[in] collectionName Name of the colleciton of hits.
             * #param[in] subDetID The subdetectorID
             */
            HcalSD(G4String name, G4String collectionName, int subDetID);
            
            /// Destructor 
            ~HcalSD();

            /**
             * Create a hit out of the energy deposition deposited during a
             * step. 
             *
             * @param[in] step The current step. 
             * @param[in] history The readout history.
             */
            G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
            
        private:

            // TODO: document!
            double birksc1_;
            
            // TODO: document!
            double birksc2_;    
    
    }; // HcalSD

} // ldmx

#endif // SIMAPPLICATION_HCALSD_H
