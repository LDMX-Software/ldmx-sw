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
#include <ostream>

//----------//
//   LDMX   //
//----------//
#include "Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include <TObject.h> //For ClassDef

namespace ldmx {

    class PnWeightResult {

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

            /** Get the kinetic energy of the hardest nucleon. */
            double getHighestWNucleonKe() const { return highestWNucleonKe_; }

            /** Get the polar angle of the hardest nucleon. */
            double getHighestWNucleonTheta() const { return highestWNucleonTheta_; }

            /** Get the W of the hardest nucleon. */
            double getHighestWNucleonW() const { return highestWNucleonW_; }

            /** Return the calcualted PN weight associated with this event. */
            double getWeight() const { return weight_; }

            /** Return the collection of inclusive W values for the event. */
            std::vector<double> getInclusiveW() { return w_; }

            /** Add theta to the collection of theta's. */
            void addTheta(double theta) { theta_.push_back(theta); }

            /** Add W to the inclusive collection of W's. */
            void addW(double w) { w_.push_back(w); }

            /** Set the kinetic energy of the hardest nucleon. */
            void setHardestNucleonKe(const double hardestNucleonKe) { hardestNucleonKe_ = hardestNucleonKe; } 
            
            /** Set the polar angle of the hardest nucleon. */
            void setHardestNucleonTheta(const double hardestNucleonTheta) { hardestNucleonTheta_ = hardestNucleonTheta; }

            /** Set the W of the hardest nucleon. */
            void setHardestNucleonW(const double hardestNucleonW) { hardestNucleonW_ = hardestNucleonW; }

            /** Set the kinetic energy of the hardest nucleon. */
            void setHighestWNucleonKe(const double highestWNucleonKe) { highestWNucleonKe_ = highestWNucleonKe; } 
            
            /** Set the polar angle of the hardest nucleon. */
            void setHighestWNucleonTheta(const double highestWNucleonTheta) { highestWNucleonTheta_ = highestWNucleonTheta; }

            /** Set the W of the hardest nucleon. */
            void setHighestWNucleonW(const double highestWNucleonW) { highestWNucleonW_ = highestWNucleonW; }

            /** Set the PN weight associated with this event. */
            void setWeight(const double weight) { weight_ = weight; }

            /** Reset the object. */
            void Clear();

            /** Print the object */
            void Print(std::ostream& o) const;

        private:

            /** Theta of all nucleons. */
            std::vector<double> theta_; 

            /** W of all nucleons. */
            std::vector<double> w_; 

            /** Kinetic energy of the hardest nucleon. */
            double hardestNucleonKe_{-9999};
           
            /** Polar angle of the hardest nucleon. */
            double hardestNucleonTheta_{-9999}; 

            /** W of the hardest nucleon. */
            double hardestNucleonW_{-9999}; 

            /** Kinetic energy of the nucleon with the highest W. */
            double highestWNucleonKe_{-9999};
           
            /** Polar angle of the nucleon with the highest W. */
            double highestWNucleonTheta_{-9999}; 

            /** W of the nucleon with the highest W. */
            double highestWNucleonW_{-9999}; 

            /** Calculated PN weight */
            double weight_{1.0};

            ClassDef(PnWeightResult, 1);
    };
}

#endif
