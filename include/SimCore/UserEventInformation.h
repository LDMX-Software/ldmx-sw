#ifndef SIMCORE_USEREVENTINFORMATION_H
#define SIMCORE_USEREVENTINFORMATION_H 

#include "G4VUserEventInformation.hh"

namespace ldmx { 

    /**
     * Encapsulates user defined information associated with a Geant4 event.
     */
    class UserEventInformation : public G4VUserEventInformation { 
        
        public: 

            /// Constructor 
            UserEventInformation(); 

            /// Destructor 
            ~UserEventInformation(); 

            /// Print the information associated with the track
            void Print() const final override; 

            /// Increment the number of brem candidates in an event.
            void incBremCandidateCount() { bremCandidateCount_ += 1; } 

            /// Decrease the number of brem candidates in an event.
            void decBremCandidateCount() { bremCandidateCount_ -= 1; } 

            /**
             * Set the event weight.
             *
             * @param[in] weight the event weight
             */
            void setWeight(double weight) { weight_ = weight; }

            /**
             * @return The event weight
             */
            double getWeight() { return weight_; }

            /**
             * Increment the event weight by the input weight
             * for an individual step.
             *
             * @param[in] step_weight weight of an individual step
             */
            void incWeight(double step_weight) { weight_ *= step_weight; }

            /**
             * @return The total number of brem candidates that this event 
             *      contains.
             */
            int bremCandidateCount() { return bremCandidateCount_; }

        private: 

            /// Total number of brem candidates in the event
            int bremCandidateCount_{0};

            /** 
             * The event weight
             *
             * @note The action WeightByStep relies on the assumption
             * that this weight *starts at 1*, so the value of this
             * member variable should always be 1.
             */
            double weight_{1.}; 
    };
}

#endif // SIMCORE_USEREVENTINFORMATION_H 
