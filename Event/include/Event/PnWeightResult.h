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
           void setResult(double ke, double theta, double w, double fitW, double weight, int pdgID,
                          double ke_hard,  double p_hard,  double pz_hard,  double w_hard,  double theta_hard, int A_hard,
                          double ke_heavy, double p_heavy, double pz_heavy, double w_heavy, double theta_heavy, int A_heavy,
                          double ke_dau,   double p_dau,   double pz_dau,   double w_dau,   double theta_dau, int pdg_dau
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
            int pdgID_{-1};


            // hardest nucleus
            double ke_hard_{0.};
            double p_hard_{0.};
            double pz_hard_{0.};
            double w_hard_{0.};
            double theta_hard_{0.};
            int A_hard_{0};

            // heaviest nucleus
            double ke_heavy_{0.};
            double p_heavy_{0.};
            double pz_heavy_{0.};
            double w_heavy_{0.};
            double theta_heavy_{0.};
            int A_heavy_{0};

            // hardest of any daughter except nucleus
            double ke_dau_{0.};
            double p_dau_{0.};
            double pz_dau_{0.};
            double w_dau_{0.};
            double theta_dau_{0.};
            int pdg_dau_{0};

            ClassDef(PnWeightResult, 3);
    };
}

#endif
