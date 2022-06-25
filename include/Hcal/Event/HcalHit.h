/**
 * @file HcalHit.h
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef HCAL_EVENT_HCALHIT_H_
#define HCAL_EVENT_HCALHIT_H_

// LDMX
#include "Recon/Event/CalorimeterHit.h"

namespace ldmx {

/**
 * @class HcalHit
 * @brief Stores reconstructed hit information from the HCAL
 *
 * @note This class represents the reconstructed hit information
 * from the HCAL, providing particular information for the HCAL,
 * above and beyond what is available in the CalorimeterHit.
 */
class HcalHit : public ldmx::CalorimeterHit {
 public:
  /**
   * Class constructor.
   */
  HcalHit() {}

  /**
   * Class destructor.
   */
  virtual ~HcalHit() {}

  /**
   * Clear the data in the object.
   */
  void Clear();

  /**
   * Print out the object.
   */
  void Print() const;

  /**
   * Get the number of photoelectrons estimated for this hit.
   * @return Number of photoelectrons, including noise which affects the
   * estimate.
   */
  float getPE() const { return pe_; }

  /**
   * Get the minimum number of photoelectrons estimated for this hit if two
   * sided readout.
   * @return Minimum number of photoelectrons, including noise which affects the
   * estimate.
   */
  float getMinPE() const { return minpe_; }

  /**
   * Get the section for this hit.
   * @return section number
   */
  int getSection() const { return section_;}

  /**
   * Get the layer for this hit.
   * @return layer number
   */
  int getLayer() const { return layer_;}

  /**
   * Get the strip for this hit.
   * @return strip number
   */
  int getStrip() const { return strip_;}

  /**
   * Get end for this hit.
   * @return end
   */
  int getEnd() const {return end_;}

  /**
   * Get if hit was reconstructed using ADC.
   * @return isADC
   */
  int getIsADC() const { return isADC_;}
  
  /**
   * Set the number of photoelectrons estimated for this hit.
   * @param pe Number of photoelectrons, including noise which affects the
   * estimate.
   */
  void setPE(float pe) { pe_ = pe; }

  /**
   * Set the minimum number of photoelectrons estimated for this hit.
   * @param pe Minimum number of photoelectrons, including noise which affects
   * the estimate.
   */
  void setMinPE(float minpe) { minpe_ = minpe; }

  /**
   * Set the section for this hit.
   * @param section number
   */
  void setSection(int section) { section_ = section; }

  /**
   * Set the layer for this hit.
   * @param layer number
   */
  void setLayer(int layer) { layer_ = layer; }

  /**
   * Set the strip for this hit.
   * @param strip number
   */
  void setStrip(int strip) { strip_ = strip; }

  /**
   * Set the end (0 neg, 1 pos side).
   * @param end
   */
  void setEnd(int end) { end_ = end; }

  /**
   * Set if the hit is reconstructed using ADC 
   * @param isADC int
   */
  void setIsADC(int isADC) { isADC_ = isADC; }
  
 private:
  /** The number of PE estimated for this hit. */
  float pe_{0};

  /** The minimum number of PE estimated for this hit, different from pe_ when
   * you have two ended readout */
  float minpe_{-99};

  /// section, layer, strip and end
  int section_;
  int layer_;
  int strip_;
  int end_;

  /// isADC
  int isADC_;
  
  /**
   * The ROOT class definition.
   */
  ClassDef(HcalHit, 2);
};
}  // namespace ldmx

#endif /* HCAL_EVENT_HCALHIT_H_ */
