/**
 * @file Trigscinthit.h
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Andrew Whitbeck, Texas Tech University
 */

#ifndef TRIGSCINT_EVENT_TRIGSCINTHIT_H
#define TRIGSCINT_EVENT_TRIGSCINTHIT_H

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Hcal/Event/HcalHit.h"

namespace ldmx {

/**
 * @class Trigscinthit
 * @brief Stores reconstructed hit information from the HCAL
 *
 * @note This class represents the reconstructed hit information
 * from the trigger scintillator.
 */
class TrigScintHit : public ldmx::HcalHit {
 public:
  /**
   * Class constructor.
   */
  TrigScintHit() {}

  /**
   * Class destructor.
   */
  ~TrigScintHit() {}

  /**
   * Clear the data in the object.
   */
  void Clear(Option_t *option = "");

  /**
   * Print out the object.
   */
  void Print(Option_t *option = "") const;

  /**
   * Set hit bar ID.
   *
   * @param barID The bar ID of the hit.
   */
  void setBarID(const int barID) { barID_ = barID; };

  /// Get the bar ID
  int getBarID() const { return barID_; }

  /**
   * Set hit module ID.
   *
   * @param moduleID The module ID of the hit.
   */
  void setModuleID(const int moduleID) { moduleID_ = moduleID; };

  /// Get the module ID
  int getModuleID() const { return moduleID_; }

  /**
   * Set beam energy fraction of hit.
   *
   * @param beamEfrac The beam energy fraction of the hit.
   */
  void setBeamEfrac(const float beamEfrac) { beamEfrac_ = beamEfrac; };

  /// Get the beam energy fraction
  float getBeamEfrac() const { return beamEfrac_; }

 private:
  // bar/channel number associated with the hit
  int barID_{-1};
  // module/pad number associated with the hit
  int moduleID_{-1};

  /// The fraction of energy associated with beam electrons.
  float beamEfrac_{0};

  ClassDef(TrigScintHit, 2);

};  // TrigScintHit

} // namespace ldmx

#endif  // TRIGSCINT_EVENT_TRIGSCINTHIT_H
