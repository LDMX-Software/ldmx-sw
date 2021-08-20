#ifndef RECON_EVENT_CALOTRIGPRIM_H_
#define RECON_EVENT_CALOTRIGPRIM_H_

// ldmx-sw
#include <stdint.h>  //uint32_t

// ROOT
#include "TObject.h"  //For ClassDef

namespace ldmx {

// Forward declaration needed by typedef
class CaloTrigPrim;

/**
 * Define the type of collection for trig primitives
 */
typedef std::vector<CaloTrigPrim> CaloTrigPrimCollection;

/**
 * @class CaloTrigPrim
 * @brief Contains the trigger output for generic calo objects
 */
class CaloTrigPrim {
 public:
  /**
   * Default Constructor
   *
   * Needed for ROOT dictionary definition,
   * suggested to not use this constructor.
   */
  CaloTrigPrim() = default;

  /**
   * Preferred Constructor
   *
   * Defines the trigger group ID and
   * the trigger primitive value.
   *
   * @param[in] tid raw trigger group ID
   * @param[in] tp trigger primitive value
   */
  CaloTrigPrim(uint32_t tid, uint32_t tp = 0);

  /**
   * Destructor
   *
   * Needs to be defined for ROOT
   * dictionary definition, does
   * nothing right now.
   */
  virtual ~CaloTrigPrim() = default;

  /**
   * Sort the collection of CaloTPs
   * by the raw ID.
   *
   * @param[in] c another CaloTP to compare against
   * @returns true if this ID is less than the other ID
   */
  bool operator<(const CaloTrigPrim &c) { return tid_ < c.tid_; }

  /**
   * Get the id of the CaloTP
   * @returns raw trigger ID
   */
  uint32_t getId() const { return tid_; }

  /**
   * Set the trigger primitive value for the given channel
   * @params[in] tp the value of the trigger primitive
   */
  void setPrimitive(uint32_t tp) { tp_ = tp; }

  /**
   * Get the trigger primitive value for the given channel
   * @returns the value of the trigger primitive
   */
  uint32_t getPrimitive() const { return tp_; }

  /**
   * Print a description of this object.
   */
  void Print() const;

  /**
   * Stream the input CaloTP
   *
   * In one line, prints out the ID (in hex),
   * the primitive (in hex).
   *
   * @param[in] s ostream to write to
   * @param[in] d tp to write out
   * @returns modified ostream
   */
  friend std::ostream &operator<<(std::ostream &s, const CaloTrigPrim &c);

  /**
   * Stream the input tp collection
   *
   * Prints out each tp member of the collection
   * on a new line.
   *
   * @param[in] s ostream to write to
   * @param[in] c collection to write out
   * @returns modified ostream
   */
  friend std::ostream &operator<<(std::ostream &s,
                                  const CaloTrigPrimCollection &c);

 private:
  /// the raw ID for this trigger channel
  uint32_t tid_{0};
  /// the integer trigger primitive value for this channel
  uint32_t tp_{0};
  /// ROOT Dictionary class definition macro
  ClassDef(CaloTrigPrim, 1);
};
}  // namespace ldmx

#endif  // RECON_EVENT_CALOTRIGPRIM_H_
