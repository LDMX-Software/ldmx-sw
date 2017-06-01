/**
 * @file PnWeightResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        PnWeightProcessor.
 * @author Alex Patterson, UCSB
 */


#ifndef EVENT_PNWEIGHTRESULT_H_
#define EVENT_PNWEIGHTRESULT_H_

#include <iostream>

#include "Event/SimParticle.h"

#include <TObject.h>


namespace ldmx {

    class PnWeightResult : public TObject {

        public:

            /** Constructor */
            PnWeightResult();

            /** Destructor */
            ~PnWeightResult();

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

            double getWeight() {
                return weight_;
            }

            void setWeight(double inWeight) {
                weight_ = inWeight;
            }


        private:

            /** Calculated PN weight */
            double weight_{1.};

            ClassDef(PnWeightResult, 2);
    };
}

#endif
