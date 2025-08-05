/**
 * @file HcalHit.h
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Jeremy Mans, University of Minnesota
 * @author Tamas Almos Vami, UCSB, added orientation members
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
  HcalHit() = default;

  /**
   * Class destructor.
   */
  virtual ~HcalHit() = default;

  /**
   * Clear the data in the object.
   */
  void clear();

  /**
   * Print out the object.
   */
  friend std::ostream &operator<<(std::ostream &o, const HcalHit &d);

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
  int getSection() const { return section_; }

  /**
   * Get the layer for this hit.
   * @return layer number
   */
  int getLayer() const { return layer_; }

  /**
   * Get the strip for this hit.
   * @return strip number
   */
  int getStrip() const { return strip_; }

  /**
   * Get end for this hit.
   * @return end
   */
  int getEnd() const { return end_; }

  /**
   * Get if hit was reconstructed using ADC.
   * @return isADC
   */
  int getIsADC() const { return isADC_; }

  /**
   * Get the toa of the positive end.
   * @return toaPos
   */
  int getToaPos() const { return toaPos_; }

  /**
   * Get the toa of the negative end.
   * @return toaNeg
   */
  int getToaNeg() const { return toaNeg_; }

  /**
   * Get the amplitude of the positive end.
   * @return amplitudePos
   */
  int getAmplitudePos() const { return amplitudePos_; }

  /**
   * Get the amplitude of the negative end.
   * @return amplitudeNeg
   */
  int getAmplitudeNeg() const { return amplitudeNeg_; }

  /**
   * Get the position of the hit
   * @return position
   */
  double getPosition() const { return position_; }

  /**
   * Get if bar is oriented in X
   * @return true if oriented in X
   */
  bool isOrientationX() const { return orientation_ == 0; }

  /**
   * Get if bar is oriented in Y
   * @return true if oriented in Y
   */
  bool isOrientationY() const { return orientation_ == 1; }

  /**
   * Get if bar is oriented in Z
   * @return true if oriented in Z
   */
  bool isOrientationZ() const { return orientation_ == 2; }

  /**
   * Get the time difference between the two ends
   * Note: only applies for double ended readout
   * @return timeDiff
   */
  double getTimeDiff() const { return timeDiff_; }

  // Now the setters

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
   * Set the end (0 neg, 1 pos_ side).
   * @param end
   */
  void setEnd(int end) { end_ = end; }

  /**
   * Set if the hit is reconstructed using ADC
   * @param isADC int
   */
  void setIsADC(int isADC) { isADC_ = isADC; }

  /**
   * Set time difference (uncorrected)
   * @param time
   */
  void setTimeDiff(double timeDiff) { timeDiff_ = timeDiff; }

  /**
   * Set toa of the positive end
   * @param time
   */
  void setToaPos(double toaPos) { toaPos_ = toaPos; }

  /**
   * Set toa of the negative end
   * @param time
   */
  void setToaNeg(double toaNeg) { toaNeg_ = toaNeg; }

  /**
   * Set amplitude of the positive end
   * @param amplitude
   */
  void setAmplitudePos(double amplitudePos) { amplitudePos_ = amplitudePos; }

  /**
   * Set amplitude of the negative end
   * @param amplitude
   */
  void setAmplitudeNeg(double amplitudeNeg) { amplitudeNeg_ = amplitudeNeg; }

  /**
   * Set original position
   */
  void setPositionUnchanged(double position, int orientation) {
    position_ = position;
    orientation_ = orientation;
  }

  /**
   * Set if the bar is orientied in X / Y / Z
   * meanig 0 / 1 / 2, respectively
   */
  void setOrientation(int orientation) { orientation_ = orientation; }

 private:
  /** The number of PE estimated for this hit. */
  float pe_{0.0};

  /** The minimum number of PE estimated for this hit, different from pe_ when
   * you have two ended readout */
  float minpe_{-99.0};

  /// section, layer, strip and end
  int section_{-99};
  int layer_{-99};
  int strip_{-99};
  int end_{-99};

  /// isADC
  int isADC_{-99};

  double timeDiff_{-9999.};
  double position_{-9999.};
  int orientation_{-9999};

  double toaPos_{-9999.};
  double toaNeg_{-9999.};
  double amplitudePos_{-9999.};
  double amplitudeNeg_{-9999.};

  /**
   * The ROOT class definition.
   */
  ClassDef(HcalHit, 6);
};
}  // namespace ldmx

#endif /* HCAL_EVENT_HCALHIT_H_ */
