/**
 * @file EcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        EcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_ECALVETORESULT_H_
#define EVENT_ECALVETORESULT_H_

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

namespace ldmx { 
    
    class EcalVetoResult : public TObject { 
        
        public: 

            /** Constructor */
            EcalVetoResult(); 

            /** Destructor */
            ~EcalVetoResult(); 

            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setResult(bool passesVeto, float summedDep, float summedIso, float backSummedDep); 

            /** Reset the object. */
            void Clear(Option_t *option = ""); 
            
            /**
             * Copy this object. 
             *
             * @param object The target object. 
             */
            void Copy(TObject& object) const;

            /** Print the object */
            void Print(Option_t *option = "") const;

            /** Checks if the event passes the Ecal veto. */
            bool passesVeto() { return passesVeto_; };

            float getSummedDep() { return summedDep_; }; 

            float getSummedIso() { return summedIso_; };

            float getBackSummedDep() { return backSummedDep_; };

        private:
           
            /** Flag indicating whether the event is vetoed by the Ecal. */
            bool passesVeto_{false};
            
            float summedDep_{0}; 
            float summedIso_{0};
            float backSummedDep_{0};
            
        ClassDef(EcalVetoResult, 1); 

    }; // EcalVetoResult
}


#endif // EVENT_ECALVETORESULT_H_
