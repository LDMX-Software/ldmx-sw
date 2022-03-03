/**
 * @file EventReadout.h
 * @brief Class that stores full reconstructed (linearized) readout QIE sample from the TS
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef TRIGSCINT_EVENT_EVENTREADOUT_H
#define TRIGSCINT_EVENT_EVENTREADOUT_H

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "TrigScint/Event/TrigScintQIEDigis.h"

namespace trigscint {

/**
 * @class EventReadout
 * @brief This class represents the linearised QIE output
 * from the trigger scintillator, in charge (fC). 
 */
  
class EventReadout : public trigscint::TrigScintQIEDigis {
 public:
  /**
   * Class constructor.
   */
  EventReadout() = default;

  /**
   * Class destructor.
   */
  ~EventReadout() = default;

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
   * Set channel (linearized, charge-equiv) noise 
   *             
   * @param noise The noise of the channel
   */
  void setNoise(const float noise) { noise_ = noise; };

  /// Get the channel noise

  float getNoise() const { return noise_; }

  /**
   * Store charges of all time samples
   * @param q_ array of qs
   */
  void setQ(const std::vector<float> q) { qs_ = q; }

  /**
   * Get charges of all time samples
   */
  std::vector<float> getQ() const { return qs_; }  

  /**
   * Store charge quantization errors of all time samples
   * @param qErr_ array of quantization errors
   */
  void setQError(const std::vector<float> qErr) { qErrs_ = qErr; }

  /**
   * Get charges of all time samples
   */
  std::vector<float> getQError() const { return qErrs_; }  

  /**
   * Set channel (linearized, charge-equiv) average charge 
   *             
   * @param totQ The (time sample) average charge of the channel
   */
  void setTotQ(const float totQ) { totQ_ = totQ; };

  /// Get the channel totQ
  float getTotQ() const { return totQ_; }

  /**
   * Set channel (linearized, charge-equiv) average charge 
   *             
   * @param avgQ The (time sample) average charge of the channel
   */
  void setAvgQ(const float avgQ) { avgQ_ = avgQ; };

  /// Get the channel avgQ

  float getAvgQ() const { return avgQ_; }

    /**
   * Set channel (linearized, charge-equiv) minimum charge 
   *             
   * @param minQ The (time sample) minimum charge of the channel
   */
  void setMinQ(const float minQ) { minQ_ = minQ; };

  /// Get the channel minQ

  float getMinQ() const { return minQ_; }

    /**
   * Set channel (linearized, charge-equiv) maximum charge 
   *             
   * @param maxQ The (time sample) maximum charge of the channel
   */
  void setMaxQ(const float maxQ) { maxQ_ = maxQ; };

  /// Get the channel maxQ

  float getMaxQ() const { return maxQ_; }

    /**
   * Set channel (linearized, charge-equiv) median charge 
   *             
   * @param medQ The (time sample) median charge of the channel
   */
  void setMedQ(const float medQ) { medQ_ = medQ; };

  /// Get the channel medQ

  float getMedQ() const { return medQ_; }

  
    /**
   * Set channel readout itme offset (in units of samples)
   *             
   * @param timeOffset The (time sample) offset in channel readout 
   */
  void setTimeOffset(const int timeOffset) { timeOffset_ = timeOffset; };

  /// Get the channel timeOffset

  int getTimeOffset() const { return timeOffset_; }

    /**
   * Set channel readout fiber number 
   *             
   * @param fiberNb The channel readout fiber number
   */
  void setFiberNb(const int fiberNb) { fiberNb_ = fiberNb; };

  /// Get the channel fiberNb

  int getFiberNb() const { return fiberNb_; }



  
  /**
   * A dummy operator overloading
   * @note required for declaring std::vector<> in EventDef.h
   */
  bool operator<(const EventReadout &rhs) const {
    return this->chanID_ < rhs.chanID_;
  }

 private:
  /// analog to digital counts
  std::vector<float> qs_;
  std::vector<float> qErrs_;

  float pedestal_{-999.};
  float earlyPedestal_{-999.};
  float noise_{-1.};
  float totQ_{-999.};
  float avgQ_{-999.};
  float minQ_{-999.};
  float maxQ_{-999.};
  float medQ_{-999.};
  int timeOffset_{0};
  int fiberNb_{-1};

  ClassDef(EventReadout, 1);

};  // EventReadout

}  // namespace trigscint

#endif  // TRIGSCINT_EVENT_EVENTREADOUT_H
