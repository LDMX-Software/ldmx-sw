#ifndef DETDESCR_TRIGSCINTID_H
#define DETDESCR_TRIGSCINTID_H

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/DetectorID.h"

namespace ldmx {

    /**
     * Class that defines the detector ID of the trigger scintillator. 
     */
    class TrigScintID : public DetectorID {
    public:
	static const RawValue MODULE_MASK{0xFF};
	static const RawValue MODULE_SHIFT{8};
	static const RawValue BAR_MASK{0xFF};
	static const RawValue BAR_SHIFT{0};

	/// Constructor
	TrigScintID();
	
	/// Constructor with raw id
	TrigScintID(unsigned int rawid) : DetectorID(rawid) {
	    assert(null() || subdet()==SD_TRIGGER_SCINT);
	}
	
     	/**
	 * Create from a DetectorID, but check
	 */
	TrigScintID(const DetectorID id) : DetectorID(id) {
	    assert(id.null() || id.subdet()==SD_TRIGGER_SCINT); // can be replaced with a throw in the the future	    
	}
	
        /**
	 * Create from pieces
	 */
	TrigScintID(unsigned int module, unsigned int bar) : DetectorID(SD_TRIGGER_SCINT,0) {
	    id_|=(module&MODULE_MASK)<<MODULE_SHIFT;
	    id_|=(bar&BAR_MASK)<<BAR_SHIFT;
	}
	
	/// Destructor 
	~TrigScintID() { }

           
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
	int getModule() const {
	    return (id_>>MODULE_SHIFT) & MODULE_MASK;
	}
	
	/**
	 * Get the value of the bar field from the ID.
	 * @return The value of the bar field.
	 */
	int bar() const {
	    return (id_>>BAR_SHIFT) & BAR_MASK;
	}
	/**
	 * Get the bar ID.
	 *
	 * @return The bar ID. 
	 */
	int getBarID() const { return (id_>>BAR_SHIFT) & BAR_MASK; }
	
	static void createInterpreters();
    }; // TrigScintID

} // ldmx

std::ostream& operator<<(std::ostream&, const ldmx::TrigScintID&);

#endif // DETDESCR_TRIGSCINTID_H
