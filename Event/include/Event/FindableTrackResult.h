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
#include <ostream>
#include <map>

//----------//
//   ROOT   //
//----------//
#include <TObject.h> //For ClassDef
#include <TRef.h>

namespace ldmx { 
    
    class FindableTrackResult { 
        
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
            bool is4sFindable() const { return is4sFindable_; };

            /** 
             * Checks if a sim particle is findable using the strategy 3 
             * stereo + 1 axial.
             *
             */
            bool is3s1aFindable() const { return is3s1aFindable_; };

            /** 
             * Checks if a sim particle is findable using the strategy 2 
             * stereo + 2 axial.
             *
             */
            bool is2s2aFindable() const { return is2s2aFindable_; };
           
            /** 
             * Checks if a sim particle is findable using the strategy 2 
             * axial.
             *
             */
            bool is2aFindable() const { return is2aFindable_; };

            /**
             * Checks if a sim particle is findable using the 2 stereo hit 
             * strategy.
             */
            bool is2sFindable() const { return is2sFindable_; };

            /**
             * Checks if a sim particle is findable using the 3 stereo hit 
             * strategy.
             */
            bool is3sFindable() const { return is3sFindable_; };

            /**
             * Set the sim particle associated with this result.
             */
            void setParticleTrackID(int trackID) { particleTrackID_ = trackID; };

            /**
             * Get the track ID of the sim particle causing this track
             */
            int getParticleTrackID() const { return particleTrackID_; }

            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setResult(Strategy strategy, bool isFindable); 

            /** Reset the object. */
            void Clear(); 
            
            /** Print out the object */
            void Print(std::ostream& o) const;

            /** Sort by track ID of particle causing track */
            bool operator < ( const FindableTrackResult &rhs ) const {
                return this->getParticleTrackID() < rhs.getParticleTrackID();
            }

        private:
            
            /** Unique identifying number for the particle in Geant4 */
            int particleTrackID_{-1};

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
