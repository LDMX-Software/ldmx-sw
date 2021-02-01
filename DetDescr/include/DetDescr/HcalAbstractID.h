/**
 * @file HcalAbstractID.h
 * @brief Class that serves as a parent for HCal detector IDs of various types
 * @author Jeremiah Mans, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#ifndef DETDESCR_HCALABSTRACTID_H_
#define DETDESCR_HCALABSTRACTID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

    /**
     * @class HcalAbstractID
     * @brief Parent of HcalIDs
     */
    class HcalAbstractID : public DetectorID {

    public:

        /** 
	 * Encodes whether the side of the strips of the HCal is saved
	 */
        enum HcalEndType {
	  Global = 0,   
	  Digi = 1,
	  Trigger = 2,
	  Special = 7,
	};
        static const RawValue END_TYPE_MASK{0x7}; // space for up to two end types
        static const RawValue END_TYPE_SHIFT{23};
        static const RawValue HCAL_PAYLOAD_MASK{0x007FFFFF};
      
	/**
	 * Empty HCAL id (but not null!)
	 */
	HcalAbstractID() : DetectorID(SD_HCAL,0) { }

	/**
	 * Create from raw number
	 */
	HcalAbstractID(RawValue rawid) : DetectorID(rawid) {
	    SUBDETECTORID_TEST("HcalAbstractID", SD_HCAL);
	}

      	/**
	 * Create from a DetectorID, but check
	 */
	HcalAbstractID(const DetectorID id) : DetectorID(id) {
	    SUBDETECTORID_TEST("HcalAbstractID", SD_HCAL);
	}

        /**
	 * Create from pieces
	 */
        HcalAbstractID(unsigned int end_type, unsigned int payload) : DetectorID(SD_HCAL,0) {
	  id_|=(end_type&END_TYPE_MASK)<<END_TYPE_SHIFT;
	  id_|=(payload&HCAL_PAYLOAD_MASK);
	}

        /**
         * Get the value of the end field from the ID.
         * @return The value of the end field.
         */
        int end_type() const {
            return (id_>>END_TYPE_SHIFT)&END_TYPE_MASK;
        }
      
    };

}

#endif
