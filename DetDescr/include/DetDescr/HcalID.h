/**
 * @file HcalID.h
 * @brief Class that defines an HCal sensitive detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_HCALID_H_
#define DETDESCR_HCALID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

    /**
     * Encodes the section of the HCal based on the 'section' field value.
     */
    enum HcalSection {
        BACK = 0,
        TOP = 1,
        BOTTOM = 2,
        LEFT = 4,
        RIGHT = 3
    };
    
    /**
     * @class HcalID
     * @brief Implements detector ids for HCal subdetector
     */
    class HcalID : public DetectorID {	
	public:

	static const RawValue SECTION_MASK{0x7}; // space for up to 7 sections
	static const RawValue SECTION_SHIFT{18};
	static const RawValue LAYER_MASK{0xFF}; // space for up to 255 layers
	static const RawValue LAYER_SHIFT{10};
	static const RawValue STRIP_MASK{0xFF}; // space for 255 strips/layer
	static const RawValue STRIP_SHIFT{0};

	/**
	 * Empty HCAL id (but not null!)
	 */
	HcalID() : DetectorID(SD_HCAL,0) { }

	/**
	 * Create from raw number
	 */
	HcalID(RawValue rawid) : DetectorID(rawid) {
	    assert(null() || subdet()==SD_HCAL);
	}

      	/**
	 * Create from a DetectorID, but check
	 */
	HcalID(const DetectorID id) : DetectorID(id) {
	    assert(id.null() || id.subdet()==SD_HCAL); // can be replaced with a throw in the the future	    
	}

    
        /**
	 * Create from pieces
	 */
	HcalID(unsigned int section, unsigned int layer, unsigned int strip) : DetectorID(SD_HCAL,0) {
	    id_|=(section&SECTION_MASK)<<SECTION_SHIFT;
	    id_|=(layer&LAYER_MASK)<<LAYER_SHIFT;
	    id_|=(strip&STRIP_MASK)<<STRIP_SHIFT;
	}
	
	
	/*
	 * Get the value of the 'section' field from the ID.
	 * @return The value of the 'strip' field.
	 */
	int getSection() const {
	    return (id_>>SECTION_SHIFT)&SECTION_MASK;
	}

        /*
	 * Get the value of the 'section' field from the ID.
	 * @return The value of the 'strip' field.
	 */
	int section() const {
	    return (id_>>SECTION_SHIFT)&SECTION_MASK;
	}

 	/**
	 * Get the value of the layer field from the ID.
	 * @return The value of the layer field.
	 */
	int layer() const {
	    return (id_>>LAYER_SHIFT) & LAYER_MASK;
	}

	/**
	 * Get the value of the layer field from the ID.
	 * @return The value of the layer field.
	 */
	int getLayerID() const {
	    return (id_>>LAYER_SHIFT) & LAYER_MASK;
	}

        /**
	 * Get the value of the 'strip' field from the ID.
	 * @return The value of 'strip' field.
	 */
	int getStrip() const {
	    return (id_>>STRIP_SHIFT) & STRIP_MASK;
	}

	/**
	 * Get the value of the 'strip' field from the ID.
	 * @return The value of 'strip' field.
	 */
	int strip() const {
	    return (id_>>STRIP_SHIFT) & STRIP_MASK;
	}

	static void createInterpreters();
    };
}

std::ostream& operator<<(std::ostream&, const ldmx::HcalID&);

#endif
