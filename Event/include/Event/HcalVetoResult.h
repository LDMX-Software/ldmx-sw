/**
 * @file HcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        HcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_HCALVETORESULT_H_
#define EVENT_HCALVETORESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

namespace ldmx { 
    
    class HcalVetoResult : public TObject { 
        
        public: 

            /** Constructor */
            HcalVetoResult(); 

            /** Destructor */
            ~HcalVetoResult(); 

            /**
             * Set the flag indicating whether it passed the veto.
             */
            void setResult(bool passesVeto) { passesVeto_ = passesVeto; };

            /** Copy the object */
            void Copy(TObject& object) const; 

            /** Reset the object. */
            void Clear(Option_t *option = ""); 
            
            /** Print out the object */
            void Print(Option_t *option = "");

            /** Checks if the event passes the Hcal veto. */
            bool passesVeto() { return passesVeto_; };

        private:
           
            /** Flag indicating whether the event passes the Hcal veto. */
            bool passesVeto_{false};

        ClassDef(HcalVetoResult, 1); 

    }; // HcalVetoResult
}


#endif // EVENT_HCALVETORESULT_H_
