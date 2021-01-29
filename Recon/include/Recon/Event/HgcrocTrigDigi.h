#ifndef RECON_EVENT_HGCROCTRIGDIGI_H_
#define RECON_EVENT_HGCROCTRIGDIGI_H_

// ldmx-sw
#include <stdint.h>  //uint{32,8}_t

// ROOT
#include "TObject.h"  //For ClassDef

namespace ldmx {

// Forward declaration needed by typedef
class HgcrocTrigDigi;

/**
 * Define the type of collection for trig digis
 */
typedef std::vector<HgcrocTrigDigi> HgcrocTrigDigiCollection;

/**
 * @class HgcrocTrigDigi
 * @brief Contains the trigger output for a single trigger hgcroc channel
 */
class HgcrocTrigDigi {
 public:
  /**
   * Default Constructor
   *
   * Needed for ROOT dictionary definition,
   * suggested to not use this constructor.
   */
  HgcrocTrigDigi() = default;

  /**
   * Preferred Constructor
   *
   * Defines the trigger group ID and
   * the trigger primitive value.
   *
   * @param[in] tid raw trigger group ID
   * @param[in] tp trigger primitive value
   */
  HgcrocTrigDigi(uint32_t tid, uint8_t tp = 0);

  /**
   * Destructor
   *
   * Needs to be defined for ROOT
   * dictionary definition, does
   * nothing right now.
   */
  virtual ~HgcrocTrigDigi() = default;

  /**
   * Sort the collection to trig digis
   * by the raw ID.
   *
   * @param[in] d another digi to compare against
   * @returns true if this ID is less than the other ID
   */
  bool operator<(const HgcrocTrigDigi &digi) { return tid_ < digi.tid_; }

  /**
   * Get the id of the digi
   * @returns raw trigger ID
   */
  uint32_t getId() const { return tid_; }

  /**
   * Set the trigger primitive (7 bits) for the given link on a channel
   * @params[in] tp the value of the trigger primitive
   */
  void setPrimitive(uint8_t tp) { tp_ = tp; }

  /**
   * Get the trigger primitive (7 bits) for the given link on a channel
   * @returns the value of the trigger primitive
   */
  uint8_t getPrimitive() const { return tp_; }

  /**
   * Get the linearized value of the trigger primitive
   * @returns the linearized (unpacked) trigger primitive value
   */
  uint32_t linearPrimitive() const { return compressed2Linear(getPrimitive()); }

  /**
   * Static conversion from 18b linear -> compressed
   *
   * @param[in] lin linearized 18bit ADC value
   * @returns equivalent compressed 7bit ADC value
   */
  static uint8_t linear2Compressed(uint32_t lin);

  /**
   * Static conversion from compressed -> linear 18b
   *
   * @param[in] comp compressed 7bit ADC value
   * @returns equivalent linearized 18bit ADC value
   */
  static uint32_t compressed2Linear(uint8_t comp);

  /**
   * Print a description of this object.
   */
  void Print() const;

  /**
   * Stream the input digi
   *
   * In one line, prints out the ID (in hex),
   * the primitive (in hex), and the linearized
   * primitive (in dec).
   *
   * @param[in] o ostream to write to
   * @param[in] d digi to write out
   * @returns modified ostream
   */
  friend std::ostream &operator<<(std::ostream &o, const HgcrocTrigDigi &d);

  /**
   * Stream the input digi collection
   *
   * Prints out each digi member of the collection
   * on a new line.
   *
   * @param[in] o ostream to write to
   * @param[in] c collection to write out
   * @returns modified ostream
   */
  friend std::ostream &operator<<(std::ostream &o,
                                  const HgcrocTrigDigiCollection &c);

 private:
  /// the raw ID for this trigger channel
  uint32_t tid_{0};
  /// the compressed 7bit trigger primitive value for this channel
  uint8_t tp_{0};
  /// ROOT Dictionary class definition macro
  ClassDef(HgcrocTrigDigi, 1);
};
}  // namespace ldmx

#endif  // RECON_EVENT_HGCROCTRIGDIGI_H_
