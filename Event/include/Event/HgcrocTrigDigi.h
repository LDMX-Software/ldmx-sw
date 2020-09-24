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
 * @brief Contains the trigger output for a single trigger hgcroc channel
 */
class HgcrocTrigDigi {
 public:

  HgcrocTrigDigi();
  HgcrocTrigDigi(uint32_t tid, uint8_t tp=0);

  virtual ~HgcrocTrigDigi();

  bool operator<(const HgcrocTrigDigi &d) {
    return tid_<d.tid_;
  }

  /** 
   * Get the id of the digi 
   */
  uint32_t getId() const {
    return tid_;
  }
  
  /**
   * Set the trigger primitive (7 bits) for the given link on a channel
   */
  void setPrimitive(uint8_t tp) {
    tp_=tp;
  }

  /**
   * Get the trigger primitive (7 bits) for the given link on a channel
   */
  uint8_t getPrimitive() const {
    return tp_;
  }

  /** 
   * Get the linearized value of the trigger primitive
   */
  uint32_t linearPrimitive() const {
    return compressed2Linear(getPrimitive());
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
  uint8_t tp_{0};
  ClassDef(HgcrocTrigDigi, 1);
}; 

typedef std::vector<HgcrocTrigDigi> HgcrocTrigDigiCollection;

}

std::ostream& operator<<(std::ostream&, const ldmx::HgcrocTrigDigi&);
std::ostream& operator<<(std::ostream&, const ldmx::HgcrocTrigDigiCollection&);



#endif // EVENT_HGCROCTRIGDIGI_H_
