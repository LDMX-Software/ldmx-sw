#ifndef EVENT_ECALHIT_H_
#define EVENT_ECALHIT_H_

// ROOT
#include "Event/CalorimeterHit.h"

namespace event {

    /** 
     * @class EcalHit
     * @brief Stores reconstructed hit information from the ECAL
     *
     * @note This class representes the reconstructed hit information
     * from the ECAL, providing particular information for the ECAL,
     * above and beyond what is available in the CalorimeterHit.
     */
    class EcalHit : public CalorimeterHit {

    public:

        /**
         * Class constructor.
         */
        EcalHit() {;}

        /**
         * Class destructor.
         */
         virtual ~EcalHit() {;}

        /**
         * Print out the object.
         */
        void Print(Option_t *option = "") const;

    private:

    /**
     * The ROOT class definition.
     */
    ClassDef(EcalHit, 1);
};
    
}


#endif /* INCLUDE_EVENT_ECALHIT_H_ */
