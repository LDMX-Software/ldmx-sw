#ifndef RECON_EVENT_TRIGENERGYSUM_H_
#define RECON_EVENT_TRIGENERGYSUM_H_

// ldmx-sw
#include <stdint.h>  //uint32_t

// ROOT
#include "TObject.h"  //For ClassDef

namespace ldmx {

// Forward declaration needed by typedef
class TrigEnergySum;

/**
 * Define the type of collection for trig digis
 */
typedef std::vector<TrigEnergySum> TrigEnergySumCollection;

/**
 * @class TrigEnergySum
 * @brief Contains the trigger output for generic calo objects
 */
class TrigEnergySum {
 public:
  /**
   * Default Constructor
   *
   * Needed for ROOT dictionary definition,
   * suggested to not use this constructor.
   */
  TrigEnergySum() = default;

  /**
   * Preferred Constructor
   *
   * Defines the trigger group ID and
   * the trigger primitive value.
   *
   * @param[in] tid raw trigger group ID
   * @param[in] tp trigger primitive value
   */
  TrigEnergySum(int layer, int hwEnergy = 0);

  /**
   * Destructor
   *
   * Needs to be defined for ROOT
   * dictionary definition, does
   * nothing right now.
   */
  virtual ~TrigEnergySum() = default;

  /**
   * Sort the collection to trig digis
   * by the raw ID.
   *
   * @param[in] d another digi to compare against
   * @returns true if this ID is less than the other ID
   */
  bool operator<(const TrigEnergySum &sum) { return hwEnergy_ < sum.hwEnergy_; }

  void Clear() { hwEnergy_ = 0; layer_=0; }
  
  void setLayer(int layer) { layer_ = layer; }
  int layer() const { return layer_; }

  void setHwEnergy(int hwEnergy) { hwEnergy_ = hwEnergy; }
  int hwEnergy() const { return hwEnergy_; }
  // inline float pe() const { return hwEnergy_*pe_per_adc_; }
  // inline float hadEnergy() const { return hwEnergy_*pe_per_adc_*mev_per_pe_*had_sample_frac_; }

  
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
  friend std::ostream &operator<<(std::ostream &o, const TrigEnergySum &d);

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
                                  const TrigEnergySumCollection &c);

 private:
  /// the raw ID for this trigger channel
  int layer_{0};
  int hwEnergy_{0};
  /* const float pe_per_adc_{1.2/5}; // gain * pe/mV */
  /* const float mev_per_pe_{4.66/68}; // MeV/PE (MIP) */
  /* const float had_sample_frac_{0.109813*0.818731}; */
  // const float had_samp_frac = (20/77.07)/(20/77.07 + 25/16.77); // 0.148266
  // const float em_samp_frac = (20/41.31)/(20/41.31 + 25/1.757); // 0.032906
  // const float samp_frac = (em_samp_frac + 2*had_samp_frac)/3; // 0.109813
  // const float attn = exp(-1/5.); // 0.818731 5m attenuation length, 1m half-bar

  /// ROOT Dictionary class definition macro
  ClassDef(TrigEnergySum, 1);
};
}  // namespace ldmx

#endif  // RECON_EVENT_CALOTRIGPRIM_H_
