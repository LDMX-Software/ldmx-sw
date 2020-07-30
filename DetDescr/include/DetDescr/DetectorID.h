/**
 * @file DetectorID.h
 * @brief Class that defines a 32-bit packed ID for uniquely identifying hits and detector components
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef DETDESCR_DETECTORID_H_
#define DETDESCR_DETECTORID_H_

#include <cstdint>
#include <iostream>
#include "assert.h" // a temporary solution until Exception is refactored to allow usage from DetDescr

namespace ldmx {
    
    typedef enum SubdetectorIDTypeEnum {
	SD_TRACKER_TAGGER   = 1,
	SD_TRIGGER_SCINT    = 2,
	SD_ACTVE_TARGET     = 3,
	SD_TRACKER_RECOIL   = 4,
	SD_ECAL             = 5,
	SD_HCAL             = 6,
	SD_SCORING_PLANE    = 7, 
	EID_TRACKER         = 16,
	EID_TRIGGER_SCINT   = 17,
	EID_ECAL            = 18,
	EID_HCAL            = 19
    } SubdetectorIDType;
    
    /**
     * @class DetectorID
     * @brief Defines a 32-bit packed ID for uniquely identifying hits and detector components
     *
     * @note This class has a memory footprint of a single 32-bit integer, so can be efficiently used in data structures
     */
    class DetectorID {
    public:
	typedef uint32_t RawValue;

	static const RawValue SUBDETECTORID_MASK{0x3F};
	static const RawValue SUBDETECTORID_SHIFT{26};
	static const RawValue SUBDETECTOR_PAYLOAD_MASK{0x3FFFFFFF};

		
	/**
	 * Class constructor for a null DetectorID
	 */
	DetectorID() : id_{0} {
	}
	
	
	/**
	 * Class constructor from a raw 32-bit integer
	 */
	DetectorID(RawValue rawid) : id_{rawid} {	    
	}
	
	/**
	 * Class constructor from a subdetector id and a subdetector-specific section (masked to 
	 */
	DetectorID(SubdetectorIDType sdtype, RawValue raw_subpayload) {
	    id_=((RawValue(sdtype) & DetectorID::SUBDETECTORID_MASK ) << DetectorID::SUBDETECTORID_SHIFT) | (raw_subpayload & DetectorID::SUBDETECTOR_PAYLOAD_MASK);
	}


	/**
	 * Is this an empty id?
	 */
	bool null() const { return id_==0; }
	
	/** 
	 * Get the subdetector type
	 */
	SubdetectorIDType subdet() const {
	    return SubdetectorIDType((id_>>SUBDETECTORID_SHIFT)&SUBDETECTORID_MASK);
	}


	/** get the raw value */
	RawValue raw() const {
	    return id_;
	}

	/**
	 * Set the raw value of the detector ID.
	 * @param rawValue The raw value of the ID.
	 */
	void setRawValue(RawValue rawValue) {
	    id_=rawValue;
	}

	bool operator<(const DetectorID& id) const {
	    return id_<id.id_;
	}

	bool operator==(const DetectorID& id) const {
	    return id_==id.id_;
	}

	bool operator!=(const DetectorID& id) const {
	    return id_!=id.id_;
	}

    protected:

	/**
	 * The raw, packed value of the ID.
	 */
	RawValue id_;	
    };
    
}

std::ostream& operator<<(std::ostream&, const ldmx::DetectorID&);

#endif
