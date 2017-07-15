/*
 * @file Top.h
 * @brief Top DetectorElement in the hierarchy providing access to the major subsystems
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TOP_H_
#define DETDESCR_TOP_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    /**
     * @class Top
     * @brief Top of DetectorElement hierarchy
     */
    class Top : public DetectorElementImpl {

        public:

            Top();

            ~Top();

        private:
            DE_INIT(Top)
    };
}


#endif /* DETDESCR_TOPDETECTORELEMENT_H_ */
