#ifndef SIMAPPLICATION_TRIGSD_H
#define SIMAPPLICATION_TRIGSD_H

/*~~~~~~~~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~~~~~~~~*/
#include "SimCore/CalorimeterSD.h"

// Forward declarations
class G4Step;
class G4TouchableHistory; 

namespace ldmx {

    /**
     * Class defining a sensitive detector of type trigger scintillator.
     */
    class TrigScintSD : public CalorimeterSD {

        public:

            /**
             * Class constructor.
             *
             * @param[in] name The name of the sensitive detector.
             * @param[in] theCollectionName The name of the hits collection.
             * @param[in] subdet The subdetector ID.
             */
            TrigScintSD(G4String name, G4String collectionName, int subDetID); 

            /// Destructor
            ~TrigScintSD();

            /**
             * Process steps to create hits.
             *
             * @param[in] step The step information.
             * @param[in] history The readout history.
             */
            G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

    };

}

#endif
