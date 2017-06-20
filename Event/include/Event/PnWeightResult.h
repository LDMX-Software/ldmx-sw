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
            double getFitWp() { return fitWp_; }

            /** 
             * Return the kinetic energy of the nucleon used to calculate 
             * W_p.
             */ 
            double getKineticEnergy() { return ke_; }

            /** Return the measured W_p. */
            double getMeasuredWp() { return wp_; }
           
            /** Return the polar angle of the nucleon used to calculate W_p. */
            double getTheta() { return theta_; }

            /** Return the calcualted PN weight associated with this event. */
            double getWeight() { return weight_; }

            /** Set the event weight and measured/fit wp. */
            void setResult(
                           double fitWp, double ke, double wp, double theta, double weight, 
                           double fitWp_max, double ke_max, double wp_max, double theta_max, double weight_max, 
                           double fitWp_p, double ke_p, double wp_p, double theta_p, double weight_p
                          );

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
 
            /** Fit Wp */
            double fitWp_{0.0};

            /** Kinetic energy of nucleon used to calculate Wp */
            double ke_{0.0};
            
            /** Measured Wp */
            double wp_{0.0}; 

            /** Polar angle of nucleon used to calculate Wp */
            double theta_{0.0}; 
            
            /** Calculated PN weight */
            double weight_{0.0};

            double fitWp_max_{0.0};
            double ke_max_{0.};
            double wp_max_{0.};
            double theta_max_{0.};
            double weight_max_{0.0};

            double fitWp_p_{0.0};
            double ke_p_{0.};
            double wp_p_{0.};
            double theta_p_{0.};
            double weight_p_{0.0};

            ClassDef(PnWeightResult, 1);
    };
}

#endif
