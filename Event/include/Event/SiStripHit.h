/**
 * @file SiStripHit.h
 * @brief Class representing a Si sensor strip hit.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_SISTRIPHIT_H_
#define EVENT_SISTRIPHIT_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <string>

//----------//
//   ROOT   //
//----------//
#include "TObject.h" //ClassDef

//-------------//
//   LDMX-SW   //
//-------------//
#include "Event/RawHit.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace ldmx { 

    class SiStripHit : public RawHit { 
    
        public: 

            /** Constructor */
            SiStripHit(); 

            /** Destructor */
            virtual ~SiStripHit(); 

            /** 
             * Add a SimTrackerHit that contributes to this SiStripHit. 
             *
             * @param simTrackerHit A simulated tracker hit that contributes to
             *                      this SiStripHit.
             */
            void addSimTrackerHit(const SimTrackerHit simTrackerHit);

            /** 
             * Return a TRefArray with the SimTrackerHits associated with 
             * this SiStripHit.
             */ 
            const std::vector<SimTrackerHit> getSimTrackerHits() const { return simTrackerHits_; }

            /** Print a description of this object. */
            void Print() const; 

            /** Reset this object. */
            void Clear();
        
        private: 

            /** 
             * References to the SimTrackerHits used to create this 
             * SiStripHit.  
             */
            std::vector<SimTrackerHit> simTrackerHits_;
            
            /** Class declaration */ 
            ClassDef(SiStripHit, 1);
            
    }; // SiStripHit
}

#endif // EVENT_SISTRIPHIT_H_
