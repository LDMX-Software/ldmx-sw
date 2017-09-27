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
           
            /** Get the kinetic energy of the hardest nucleon. */
            double getHardestNucleonKe() const { return hardestNucleonKe_; }

            /** Get the polar angle of the hardest nucleon. */
            double getHardestNucleonTheta() const { return hardestNucleonTheta_; }

            /** Get the W of the hardest nucleon. */
            double getHardestNucleonW() const { return hardestNucleonW_; }

            /** Return the calcualted PN weight associated with this event. */
            double getWeight() const { return weight_; }

            /** Set the kinetic energy of the hardest nucleon. */
            void setHardestNucleonKe(const double hardestNucleonKe) { hardestNucleonKe_ = hardestNucleonKe; } 
            
            /** Set the polar angle of the hardest nucleon. */
            void setHardestNucleonTheta(const double hardestNucleonTheta) { hardestNucleonTheta_ = hardestNucleonTheta; }

            /** Set the W of the hardest nucleon. */
            void setHardestNucleonW(const double hardestNucleonW) { hardestNucleonW_ = hardestNucleonW; }

            /** Set the PN weight associated with this event. */
            void setWeight(const double weight) { weight_ = weight; }

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

            /** Kinetic energy of the hardest nucleon. */
            double hardestNucleonKe_{-9999};
           
            /** Polar angle of the hardest nucleon. */
            double hardestNucleonTheta_{-9999}; 

            /** W of the hardest nucleon. */
            double hardestNucleonW_{-9999}; 
            
            /** Calculated PN weight */
            double weight_{1.0};

            /*
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
            */

            ClassDef(PnWeightResult, 1);
    };
}

#endif
