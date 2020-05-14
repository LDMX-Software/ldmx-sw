#ifndef DETDESCR_TRIGSCINTID_H
#define DETDESCR_TRIGSCINTID_H

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/DetectorID.h"

namespace ldmx {

    /**
     * Class that defines the detector ID of the trigger scintillator. 
     */
    class TrigScintID : public DetectorID {
    
        public: 
            
            /// Constructor
            TrigScintID();

            /// Destructor 
            ~TrigScintID();

            /**
             * Get the module ID. 
             *
             * @return The module ID. 
             */
            int getModuleID() { return getFieldValue(0); }

            /**
             * Get the bar ID.
             *
             * @return The bar ID. 
             */
            int getBarID() { return getFieldValue(1); }
            

    }; // TrigScintID

} // ldmx

#endif // DETDESCR_TRIGSCINTID_H
