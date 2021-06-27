/**
 * @file EcalElectronicsID.h
 * @brief Class that identifies a location in the Ecal readout chain
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef DETDESCR_ECALELECTRONICSID_H_
#define DETDESCR_ECALELECTRONICSID_H_

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/PackedIndex.h"

namespace ldmx {
    
/**
 * @class EcalElectronicsID
 * @brief Identifies a location in the Ecal readout chain
 * 
 *    -- fiber : optical fiber number (backend number), range assumed O(0-96)
 *    -- elink : electronic link number, range assumed O(0-47)
 *    -- channel : channel-on-elink, range O(0-37)
 *
 * For transient use only.
 */
class EcalElectronicsID : public DetectorID {
 public:

  static const RawValue INDEX_MASK{0xFFFFFF};
  typedef PackedIndex<38,48> Index;
  
  /**
   * Empty ECAL id (but not null!)
   */
  EcalElectronicsID() : DetectorID(EID_ECAL, 0) {}

  /**
   * Create from raw number
   */
  EcalElectronicsID(RawValue rawid) : DetectorID(rawid) {
    SUBDETECTORID_TEST("EcalElectronicsID", EID_ECAL);
  }

  /**
   * Create from a DetectorID, but check
   */
  EcalElectronicsID(const DetectorID id) : DetectorID(id) {
    SUBDETECTORID_TEST("EcalElectronicsID", EID_ECAL);
  }

  /**
   * Create from pieces
   */
  EcalElectronicsID(unsigned int fiber, unsigned int elink, unsigned int channel)
      : DetectorID(EID_ECAL, 0) {
    Index index(channel,elink,fiber);
    id_ |= index.value();
  }

  /**
   * Get the value of the fiber from the ID.
   * @return The value of the fiber field.
   */
  int fiber() const { return Index(id_&INDEX_MASK).field2(); }
  /**
   * Get the value of the fiber from the ID.
   * @return The value of the fiber field.
   */
  int elink() const { return Index(id_&INDEX_MASK).field1(); }
  /**
   * Get the value of the fiber from the ID.
   * @return The value of the fiber field.
   */
  int channel() const { return Index(id_&INDEX_MASK).field0(); }
  
};

  
}

std::ostream& operator<<(std::ostream& s, const ldmx::EcalElectronicsID& id);


#endif // DETDESCR_ECALELECTRONICSID_H_
