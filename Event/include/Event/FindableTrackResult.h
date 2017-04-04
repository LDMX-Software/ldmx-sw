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

            // Strategies
            enum Strategy {
                STRATEGY_NONE = 0, 
                STRATEGY_4S   = 1, 
                STRATEGY_3S1A = 2, 
                STRATEGY_2S2A = 3, 
                STRATEGY_2A   = 4,
                STRATEGY_2S   = 5,
                STRATEGY_3S   = 6, 
            };

            /** Constructor */
            FindableTrackResult(); 

            /** Destructor */
            ~FindableTrackResult(); 

            /** 
             * Checks if a sim particle is findable using the 4 stereo layers 
             * of the recoil tracker only. 
             */
            bool is4sFindable() { return is4sFindable_; };

            /** 
             * Checks if a sim particle is findable using the strategy 3 
             * stereo + 1 axial.
             *
             */
            bool is3s1aFindable() { return is3s1aFindable_; };

            /** 
             * Checks if a sim particle is findable using the strategy 2 
             * stereo + 2 axial.
             *
             */
            bool is2s2aFindable() { return is2s2aFindable_; };
           
            /** 
             * Checks if a sim particle is findable using the strategy 2 
             * axial.
             *
             */
            bool is2aFindable() { return is2aFindable_; };

            /**
             * Checks if a sim particle is findable using the 2 stereo hit 
             * strategy.
             */
            bool is2sFindable() { return is2sFindable_; };

            /**
             * Checks if a sim particle is findable using the 3 stereo hit 
             * strategy.
             */
            bool is3sFindable() { return is3sFindable_; };

            /**
             * Get the sim particle associated with this result.
             */
            SimParticle* getSimParticle() { return (SimParticle*) simParticle_.GetObject(); };
            
            /**
             * Set the sim particle associated with this result.
             */
            void setSimParticle(SimParticle* simParticle) { simParticle_ = simParticle; };

            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setResult(Strategy strategy, bool isFindable); 

            /** Reset the object. */
            void Clear(Option_t *option = ""); 
            
            /** Print out the object */
            void Print(Option_t *option = "");

        private:
            
            /** Refence to the sim particle. */
            TRef simParticle_{nullptr};

            /**
             * Flag indicating whether a particle is findable using the
             * strategy that requires 4 stereo layers.
             */
            bool is4sFindable_{false};

            /** 
             * Flag indicating whether a particle is findable using the 
             * strategy 3 stereo + 1 axial. 
             */
            bool is3s1aFindable_{false}; 

            /** 
             * Flag indicating whether a particle is findable using the 
             * strategy 2 stereo + 2 axial. 
             */
            bool is2s2aFindable_{false}; 
            
            /** 
             * Flag indicating whether a particle is findable using the 
             * strategy 2 axial.  This will be mainly used to reject
             * back scattered particles. 
             */
            bool is2aFindable_{false};

            /**
             * Flag indicating whether a particle is findable using the 2 stereo
             * strategy. 
             */
            bool is2sFindable_{false}; 
            
            /**
             * Flag indicating whether a particle is findable using the 3 stereo
             * hit strategy. 
             */
            bool is3sFindable_{false}; 

        ClassDef(FindableTrackResult, 3); 

    }; // FindableTrackResult
}


#endif // EVENT_FINDABLETRACKRESULT_H_
