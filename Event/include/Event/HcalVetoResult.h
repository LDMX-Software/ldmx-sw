/**
 * @file HcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        HcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_HCALVETORESULT_H_
#define EVENT_HCALVETORESULT_H_

//----------//
//   ROOT   //
//----------//
#include "TObject.h" //For ClassDef

//----------//
//   LDMX   //
//----------//
#include "Event/HcalHit.h"

namespace ldmx { 

    class HcalVetoResult { 
        
        public: 

            /** Constructor */
            HcalVetoResult(); 

            /** Destructor */
            ~HcalVetoResult(); 

            /** Reset the object. */
            void Clear(); 
            
            /** Print out the object */
            void Print(std::ostream& o) const;

            /** Checks if the event passes the Hcal veto. */
            bool passesVeto() const { return passesVeto_; };

            /** @return The maximum PE HcalHit. */
            inline HcalHit getMaxPEHit() const { return maxPEHit_; } 

            /**
             * Sets whether the Hcal veto was passed or not.
             *
             * @param passesVeto Veto result. 
             */
            inline void setVetoResult(const bool& passesVeto = true) { passesVeto_ = passesVeto; } 

            /**
             * Set the maximum PE hit.
             *
             * @param maxPEHit The maximum PE HcalHit
             */
            inline void setMaxPEHit(const HcalHit maxPEHit) { maxPEHit_ = maxPEHit; } 

        private:
            
            /** Reference to max PE hit. */
            HcalHit maxPEHit_; 

            /** Flag indicating whether the event passes the Hcal veto. */
            bool passesVeto_{false};

        ClassDef(HcalVetoResult, 2); 

    }; // HcalVetoResult
}


#endif // EVENT_HCALVETORESULT_H_
