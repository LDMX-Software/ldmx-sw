/**
 * @file PnWeightResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        PnWeightProcessor.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_PNWEIGHTRESULT_H_
#define EVENT_PNWEIGHTRESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//----------//
//   LDMX   //
//----------//
#include "Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

namespace ldmx {

    class PnWeightResult : public TObject {

        public:

            /** Constructor */
            PnWeightResult();

            /** Destructor */
            ~PnWeightResult();
            
            /** Return the fit W_p. */
            double getFitW() { return fitW_; }

            /** 
             * Return the kinetic energy of the nucleon used to calculate W
             *
             */ 
            double getKineticEnergy() { return ke_; }

            /** Return the measured W */
            double getMeasuredW() { return w_; }
           
            /** Return the polar angle of the nucleon used to calculate W */
            double getTheta() { return theta_; }

            /** Return the calcualted PN weight associated with this event. */
            double getWeight() { return weight_; }

            /** Set the event weight and measured/fit w */
            void setResult(double ke, double theta, double w, double fitW, double weight);

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
        
        private:
 
            /** Fit W */
            double fitW_{0.0};

            /** Kinetic energy of nucleon used to calculate W */
            double ke_{0.0};
            
            /** Measured W */
            double w_{0.0}; 

            /** Polar angle of nucleon used to calculate W */
            double theta_{0.0}; 
            
            /** Calculated PN weight */
            double weight_{0.0};

            ClassDef(PnWeightResult, 1);
    };
}

#endif
