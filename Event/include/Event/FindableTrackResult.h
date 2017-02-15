/**
 * @file FindableTrackResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        FindableTrackProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_FINDABLETRACKRESULT_H_
#define EVENT_FINDABLETRACKRESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>

//----------//
//   LDMX   //
//----------//
#include "Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include <TObject.h>
#include <TRef.h>

namespace ldmx { 
    
    class FindableTrackResult : public TObject { 
        
        public: 

            /** Constructor */
            FindableTrackResult(); 

            /** Destructor */
            ~FindableTrackResult(); 

            /** 
             * Checks if the sim particle can be found by the recoil 
             * tracker.
             */
            bool isFindable() { return isFindable_; };

            /**
             * Get the sim particle associated with this result.
             */
            SimParticle* getSimParticle() { return (SimParticle*) simParticle_.GetObject(); };
            
            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setResult(SimParticle* simParticle, bool isFindable); 

            /** Print out the object */
            void Print(Option_t *option = "");

        private:

            /** Refence to the sim particle. */
            TRef simParticle_;

            /** Flag indicating whether a particle is findable. */
            bool isFindable_; 


        ClassDef(FindableTrackResult, 1); 

    }; // FindableTrackResult
}


#endif // EVENT_FINDABLETRACKRESULT_H_
