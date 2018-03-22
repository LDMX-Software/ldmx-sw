/**
 * @file RawHit.h
 * @brief Class representing a raw detector hit from a real detector.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_RAWHIT_H_
#define EVENT_RAWHIT_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

namespace ldmx { 

    class RawHit : public TObject { 
    
        public:

            /** Destructor */
            virtual ~RawHit() {}; 

            /** 
             * Get the ADC values associated with this hit.  This can be a 
             * single value or multiple values depending on the readout being 
             * used.
             */
            std::vector<short> getADCValues() const { return adcValues_; }

            /** 
             * Get the time at which the hit occurred e.g. this could be the 
             * time at which the readout of the ADC values started (t0). 
             */
            float getTime() const { return time_; }

        protected: 

            /** ADC values associated with this hit. */
            std::vector<short> adcValues_; 

            /** The hit time. */
            float time_{-9999}; 
            
            /** Class declaration */ 
            ClassDef(RawHit, 1);

    }; // RawHit
}

#endif // EVENT_RAWHIT_H_
