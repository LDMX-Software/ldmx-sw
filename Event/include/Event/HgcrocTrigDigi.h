// ldmx-sw
#include "Event/EventConstants.h"
#include <stdint.h>

#ifndef EVENT_HGCROCTRIGDIGI_H_
#define EVENT_HGCROCTRIGDIGI_H_

// ROOT
#include "TObject.h" //For ClassDef


namespace ldmx {

    /**
     * @class HgcrocTrigDigi 
     * @brief Contains the trigger output for a
     * single elink from an HgcrocTrigDigi, along with indexing
     * information
     */
    class HgcrocTrigDigi {
	public:

	HgcrocTrigDigi();
	HgcrocTrigDigi(uint32_t tid);

	virtual ~HgcrocTrigDigi();


	bool operator<(const HgcrocTrigDigi &d) {
	  return tid_<d.tid_;
	}

	/**
	 * Set the trigger primitive (7 bits) for the given link on a channel
	 * @param ichan Channel id (0-3)
	 */
	void setPrimitive(int ichan, uint8_t tp) {
	    if (ichan==0) tp0_=tp&0x7f;
	    if (ichan==1) tp1_=tp&0x7f;
	    if (ichan==2) tp2_=tp&0x7f;
	    if (ichan==3) tp3_=tp&0x7f;
	}
	
	/**
	 * Get the trigger primitive (7 bits) for the given link on a channel
	 * @param ichan Channel id (0-3)
	 */
	uint8_t getPrimitive(int ichan) const {
	    if (ichan==0) return tp0_;
	    if (ichan==1) return tp1_;
	    if (ichan==2) return tp2_;
	    if (ichan==3) return tp3_;
	    return 0;
	}

	/** 
	 * Get the linearized value of the trigger primitive
	 */
	uint32_t linearPrimitive(int ichan) const {
	    return compressed2Linear(getPrimitive(ichan));
	}
	
	/** 
	 * Static conversion from 18b linear -> compressed
	 */
	static uint8_t linear2Compressed(uint32_t lin);

	/** 
	 * Static conversion from compressed -> linear 18b
	 */
	static uint32_t compressed2Linear(uint8_t comp);
	
	/**
	 * Print a description of this object.
	 */
	void Print() const;
	
	private:
         uint32_t tid_{0};
	uint8_t tp0_{0};
	uint8_t tp1_{0};
	uint8_t tp2_{0};
	uint8_t tp3_{0};

	ClassDef(HgcrocTrigDigi, 1);
    }; 

    typedef std::vector<HgcrocTrigDigi> HgcrocTrigDigiCollection;

}


#endif // EVENT_HGCROCTRIGDIGI_H_
