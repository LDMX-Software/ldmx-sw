/**
 * @file HcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        HcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_HCALVETORESULT_H_
#define EVENT_HCALVETORESULT_H_

<<<<<<< HEAD
//----------//
//   ROOT   //
//----------//
#include "TObject.h"
#include "TRef.h"

namespace ldmx { 

    // Forward declarations within the ldmx namespace
    class HcalHit; 
=======
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
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    
    class HcalVetoResult : public TObject { 
        
        public: 

            /** Constructor */
            HcalVetoResult(); 

            /** Destructor */
            ~HcalVetoResult(); 

<<<<<<< HEAD
=======
            /**
             * Set the flag indicating whether it passed the veto.
             */
            void setResult(bool passesVeto) { passesVeto_ = passesVeto; };

>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
            /** Copy the object */
            void Copy(TObject& object) const; 

            /** Reset the object. */
            void Clear(Option_t *option = ""); 
            
            /** Print out the object */
            void Print(Option_t *option = "");

            /** Checks if the event passes the Hcal veto. */
<<<<<<< HEAD
            bool passesVeto() const { return passesVeto_; };

            /** @return The maximum PE HcalHit. */
            inline HcalHit* getMaxPEHit() const { 
                return (HcalHit*) maxPEHit_.GetObject();
            } 

            /**
             * Sets whether the Hcal veto was passed or not.
             *
             * @param passesVeto Veto result. 
             */
            inline void setVetoResult(const bool& passesVeto = true) { 
                passesVeto_ = passesVeto; 
            } 

            /**
             * Set the maximum PE hit.
             *
             * @param maxPEHit The maximum PE HcalHit
             */
            inline void setMaxPEHit(HcalHit* maxPEHit) { 
               maxPEHit_ = (TObject*) maxPEHit; 
            } 

        private:
            
            /** Reference to max PE hit. */
            TRef maxPEHit_; 

            /** Flag indicating whether the event passes the Hcal veto. */
            bool passesVeto_{false};

        ClassDef(HcalVetoResult, 2); 
=======
            bool passesVeto() { return passesVeto_; };

        private:
           
            /** Flag indicating whether the event passes the Hcal veto. */
            bool passesVeto_{false};

        ClassDef(HcalVetoResult, 1); 
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

    }; // HcalVetoResult
}


#endif // EVENT_HCALVETORESULT_H_
