/**
 * @file EcalID.h
 * @brief Class that defines an ECal detector ID with a cell number
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_ECALDETECTORID_H_
#define DETDESCR_ECALDETECTORID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

    /**
     * @class EcalID
     * @brief Extension of DetectorID providing access to ECal layers and cell numbers in a hex grid
     */
    class EcalID : public DetectorID {

    public:
	
	static const RawValue LAYER_MASK{0x3F}; // space for up to 64 layers
	static const RawValue LAYER_SHIFT{17};
	static const RawValue MODULE_MASK{0x1F}; // space for up to 32 modules/layer
	static const RawValue MODULE_SHIFT{12};
	static const RawValue CELL_MASK{0xFFF}; // space for 4096 cells/module (!)
	static const RawValue CELL_SHIFT{0};
	
	/**
	 * Empty ECAL id (but not null!)
	 */
	EcalID() : DetectorID(SD_ECAL,0) { }

	/**
	 * Create from raw number
	 */
	EcalID(RawValue rawid) : DetectorID(rawid) {
	    SUBDETECTORID_TEST("EcalID", SD_ECAL);
	}

      	/**
	 * Create from a DetectorID, but check
	 */
	EcalID(const DetectorID id) : DetectorID(id) {
	    SUBDETECTORID_TEST("EcalID", SD_ECAL);
	}


        /**
	 * Create from pieces
	 */
	EcalID(unsigned int layer, unsigned int module, unsigned int cell) : DetectorID(SD_ECAL,0) {
	    id_|=(layer&LAYER_MASK)<<LAYER_SHIFT;
	    id_|=(module&MODULE_MASK)<<MODULE_SHIFT;
	    id_|=(cell&CELL_MASK)<<CELL_SHIFT;
	}
      
	/**
	 * Get the value of the module field from the ID.
	 * @return The value of the module field.
	 */
	int module() const {
	    return (id_>>MODULE_SHIFT) & MODULE_MASK;
	}


	/**
	 * Get the value of the module field from the ID.
	 * @return The value of the module field.
	 */
	int getModuleID() const {
	    return (id_>>MODULE_SHIFT) & MODULE_MASK;
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
	 * Get the value of the cell field from the ID.
	 * @return The value of the cell field.
	 */
	int cell() const {
	    return (id_>>CELL_SHIFT)&CELL_MASK;
	}

	/**
	 * Get the value of the cell field from the ID.
	 * @return The value of the cell field.
	 */
	int getCellID() const {
	    return (id_>>CELL_SHIFT)&CELL_MASK;
	}

	static void createInterpreters();
    };

}

std::ostream& operator<<(std::ostream&, const ldmx::EcalID&);

#endif
