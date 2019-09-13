/**
 * @file TrackerVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        TrackerVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _EVENT_TRACKER_VETO_RESULT_H_
#define _EVENT_TRACKER_VETO_RESULT_H_

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

namespace ldmx { 

    class TrackerVetoResult : public TObject { 
        
        public: 

            /** Constructor */
            TrackerVetoResult(); 

            /** Destructor */
            ~TrackerVetoResult(); 

            /** Copy the object */
            void Copy(TObject& object) const; 

            /** Reset the object. */
            void Clear(Option_t *option = ""); 
            
            /** Print out the object */
            void Print(Option_t *option = "");

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
