/**
 * @file TrackerVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        TrackerVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _EVENT_TRACKER_VETO_RESULT_H_
#define _EVENT_TRACKER_VETO_RESULT_H_

#include <ostream>

//----------//
//   ROOT   //
//----------//
#include "TObject.h" //ClassDef

namespace ldmx { 

    class TrackerVetoResult { 
        
        public: 

            /** Constructor */
            TrackerVetoResult(); 

            /** Destructor */
            ~TrackerVetoResult(); 

            /** Reset the object. */
            void Clear(); 
            
            /** Print out the object */
            void Print(std::ostream& o) const;

            /** Checks if the event passes the Hcal veto. */
            bool passesVeto() const { return passesVeto_; };

            /**
             * Sets whether the Hcal veto was passed or not.
             *
             * @param passesVeto Veto result. 
             */
            inline void setVetoResult(const bool& passesVeto = true) { 
                passesVeto_ = passesVeto; 
            } 

        private:
            
            /** Flag indicating whether the event passes the Hcal veto. */
            bool passesVeto_{false};

            ClassDef(TrackerVetoResult, 1); 

    }; // TrackerVetoResult
}


#endif // _EVENT_TRACKER_VETO_RESULT_H_
