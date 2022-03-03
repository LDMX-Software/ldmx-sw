/**
 * @file TestBeamHit.h
 * @brief Class that stores full reconstructed (linearized) readout QIE sample from the TS
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef TRIGSCINT_EVENT_TESTBEAMHIT_H
#define TRIGSCINT_EVENT_TESTBEAMHIT_H

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "TrigScint/Event/TrigScintHit.h"

namespace trigscint {

/**
 * @class TestBeamHit
 * @brief This class represents the linearised QIE output
 * from the trigger scintillator, in charge (fC). 
 */
  
class TestBeamHit : public trigscint::TrigScintHit {
 public:
  /**
   * Class constructor.
   */
  TestBeamHit() = default;

  /**
   * Class destructor.
   */
  ~TestBeamHit() = default;

  /**
   * Clear the data in the object.
   */
  void Clear(Option_t *option = "");

  /**
   * Print out the object.
   */
  void Print(Option_t *option = "") const;

  /**
   * Set channel (linearized. charge) pedestal 
   *             
   * @param pedestal The pedestal of the channel
   */
  void setPedestal(const float pedestal) { pedestal_ = pedestal; };

  /// Get the pedestal calculated only from first N time samples
  
  float getEarlyPedestal() const { return earlyPedestal_; }

  /**
   * Set channel (linearized. charge) pedestal calculated from beginning of readout
   *             
   * @param pedestal The pedestal of the first N time samples in the channel
   */
  void setEarlyPedestal(const float earlyPed) { earlyPedestal_ = earlyPed; };

  /// Get the pedestal
  
  float getPedestal() const { return pedestal_; }

  /**
   * Store total charge
   * @param q total charge in pulse making hit 
   */
  void setQ(const float q) { pulseQ_ = q; }

  /**
   * Get hit charge
   */
  float getQ() const { return pulseQ_; }  
  
    /**
   * Set pulse/hit start sample 
   *             
   * @param startSample The start (time) sample in the pulse
   */
  void setStartSample(const int startSample) { startSample_ = startSample; };

  /// Get the pulse/hit startSample

  int getStartSample() const { return startSample_; }


      /**
   * Set number of samples above threshold in pulse/hit
   *             
   * @param sampAboveThr The number of (time) samples above threshold in (or continuing after) the pulse
   */
  void setSampAboveThr(const int sampAboveThr) { sampAboveThr_ = sampAboveThr; };

  /// Get the pulse/hit sampAboveThr

  int getSampAboveThr() const { return sampAboveThr_; }


    /**
   * Set width used to integrate pulse/hit (in time samples)
   *             
   * @param pulseWidth The number of (time) samples used to make up the pulse
   */
  void setPulseWidth(const int pulseWidth) { pulseWidth_ = pulseWidth; };

  /// Get the pulse/hit pulseWidth

  int getPulseWidth() const { return pulseWidth_; }

  
  /**
   * A dummy operator overloading
   * @note required for declaring std::vector<> in EventDef.h
   */
  bool operator<(const TestBeamHit &rhs) const {
    return this->chanID_ < rhs.chanID_;
  }

 private:
  float pedestal_{-999.};       // assumed/average channel pedestal used in subtraction [fC]
  float earlyPedestal_{-999.};  // early pedestal (average over first 5 time samples)   [fC]
  float pulseQ_{-999.};         // integrated, ped subtracted charge over pulse [fC]
  int startSample_{-1};  //start sample 
  int pulseWidth_{-1};   //specified pulse width (in number of samples)
  int sampOverThr_{-1};  //actual number of samples above threshold after start
  
  //        - nPulses -- TODO, when running in continuous mode 
  /*
  - total Q // this is an event readout property 
  - ped subtracted total Q // so is this 
  - PE value // already property of trigscinthit
  - max amplitude in pulse // so is this 
  - store material assumption? isLYSO? -- no, this is a bar property, not a hit property 
  */
  
  ClassDef(TestBeamHit, 1);

};  // TestBeamHit

}  // namespace trigscint

#endif  // TRIGSCINT_EVENT_TESTBEAMHIT_H
