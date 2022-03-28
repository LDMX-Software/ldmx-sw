
#ifndef DETDESCR_HCALELECTRONICSID_H_
#define DETDESCR_HCALELECTRONICSID_H_

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/PackedIndex.h"

namespace ldmx {
    
/**
 * @class HcalElectronicsID
 * @brief Identifies a location in the Hcal readout chain
 * 
 *    -- fiber : optical fiber number (backend number), range assumed O(0-96)
 *    -- elink : electronic link number, range assumed O(0-47)
 *    -- channel : channel-on-elink, range O(0-37)
 *
 * For transient use only i.e. we use this ID to help translate the digitized
 * data coming off the detector into spatially-important HcalDigiIDs.
 *
 * TODO update the ranges in the packed index template.
 * These were originally copied from the EcalElectronicsID implementation
 * and may not align with the Hcal readout design.
 */
class HcalElectronicsID : public DetectorID {
 public:

  static const RawValue INDEX_MASK{0xFFFFFF};
  // PackedIndex for channel (field 0) and elink (field 1), fiber (field 2)
  typedef PackedIndex<38,48,97> Index;
  // Maximum value of any packed index here
  static const unsigned int MAX_INDEX{38*48*200};
  
  /**
   * Empty HCAL id (but not null!)
   */
  HcalElectronicsID() : DetectorID(EID_HCAL, 0) {}

  /**
   * Create from raw number
   */
  HcalElectronicsID(RawValue rawid) : DetectorID(rawid) {
    SUBDETECTORID_TEST("HcalElectronicsID", EID_HCAL);
  }

  /**
   * Create from a DetectorID, but check
   */
  HcalElectronicsID(const DetectorID id) : DetectorID(id) {
    SUBDETECTORID_TEST("HcalElectronicsID", EID_HCAL);
  }

  /**
   * Create from pieces
   */
  HcalElectronicsID(unsigned int fiber, unsigned int elink, unsigned int channel)
      : DetectorID(EID_HCAL, 0) {
    Index index(channel,elink,fiber);
    id_ |= index.value();
  }

  /**
   * Construct an electronics id from an index 
   *
   * This looks ugly (and it is) because we already have a constructor
   * that uses the unsigned int type. This means we need a different
   * static method for translating something we know to be an EID index
   * rather than a full DetID raw value.
   */
  static HcalElectronicsID idFromIndex(unsigned int index) {
    HcalElectronicsID eid;
    eid.id_|=index;
    return eid;
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

  /** 
   * Get the compact index value
   */
  unsigned int index() const { return id_&INDEX_MASK; }
  
};

}

std::ostream& operator<<(std::ostream& s, const ldmx::HcalElectronicsID& id);


#endif // DETDESCR_HCALELECTRONICSID_H_
