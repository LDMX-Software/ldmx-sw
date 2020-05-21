/**
 * @file Trigscinthit.h
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Andrew Whitbeck, Texas Tech University
 */

#ifndef EVENT_TRIGSCINTHIT_H
#define EVENT_TRIGSCINTHIT_H

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Event/HcalHit.h"

namespace ldmx {

    /**
     * @class Trigscinthit
     * @brief Stores reconstructed hit information from the HCAL
     *
     * @note This class represents the reconstructed hit information
     * from the trigger scintillator. 
     */
    class TrigScintHit : public HcalHit {

        public:

            /**
             * Class constructor.
             */
            TrigScintHit() { }

            /**
             * Class destructor.
             */
            ~TrigScintHit() { }

            /**
             * Clear the data in the object.
             */
            void Clear(Option_t *option = "");

            /**
             * Print out the object.
             */
            void Print(Option_t *option = "") const;

            /// Decode the section associated with the hit from the ID. 
            int getSection() const final override { return (getID() & 0x7000) >> 12; }

            /// Decode the strip associated with the hit from the ID. */
            int getStrip() const final override { return (getID() & 0x7F8000) >> 15; }

            /// Get the bar/channel associated with the hit from the ID. */
            int getBarID() const { return barID_; }

            /// Get the module/pad associated with the hit from the ID. */
            int getModule() const { return moduleID_; }

	    /**
             * Set the hit time. 
             *
             * @param time The hit time.
             */
            void setTime(const float time) { time_ = time; }

	    /** 
             * Set hit bar ID.
             *
             * @param barID The bar ID of the hit.
             */
	    void setBarID(const int barID) { barID_ = barID; };

	    /** 
             * Set hit module ID.
             *
             * @param moduleID The module ID of the hit.
             */
	    void setModuleID(const int moduleID) { moduleID_ = moduleID; };

	    /** 
             * Set beam energy fraction of hit. 
             *
             * @param beamEfrac The beam energy fraction of the hit.
             */
	    void setBeamEfrac(const float beamEfrac) { beamEfrac_ = beamEfrac; };
	  
        private:

            /// The time estimated for this hit. 
            float time_{0};
	    // bar/channel number associated with the hit 
	    int barID_{-1};
	    // module/pad number associated with the hit
	    int moduleID_{-1};
	    
	    /// The fraction of energy associated with beam electrons. 
	    float beamEfrac_{0};
	    
            ClassDef(TrigScintHit, 1);
    
    }; // TrigScintHit

} // ldmx

#endif // EVENT_TRIGSCINTHIT_H
