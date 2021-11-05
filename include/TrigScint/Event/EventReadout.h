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

  /// Get the pedestal
  
  int getPedestal() const { return pedestal_; }

    /**
   * Set channel (linearized, charge-equiv) noise 
   *             
   * @param noise The noise of the channel
   */
  void setNoise(const float noise) { noise_ = noise; };

  /// Get the channel noise

  int getNoise() const { return noise_; }

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
   * Set channel (linearized, charge-equiv) average charge 
   *             
   * @param avgQ The (time sample) average charge of the channel
   */
  void setAvgQ(const float avgQ) { avgQ_ = avgQ; };

  /// Get the channel avgQ

  int getAvgQ() const { return avgQ_; }

    /**
   * Set channel (linearized, charge-equiv) minimum charge 
   *             
   * @param minQ The (time sample) minimum charge of the channel
   */
  void setMinQ(const float minQ) { minQ_ = minQ; };

  /// Get the channel minQ

  int getMinQ() const { return minQ_; }

    /**
   * Set channel (linearized, charge-equiv) maximum charge 
   *             
   * @param maxQ The (time sample) maximum charge of the channel
   */
  void setMaxQ(const float maxQ) { maxQ_ = maxQ; };

  /// Get the channel maxQ

  int getMaxQ() const { return maxQ_; }

    /**
   * Set channel (linearized, charge-equiv) median charge 
   *             
   * @param medQ The (time sample) median charge of the channel
   */
  void setMedQ(const float medQ) { medQ_ = medQ; };

  /// Get the channel medQ

  int getMedQ() const { return medQ_; }

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
  //  std::vector<int> adcs_;
  //int chanID_;

  float pedestal_{-999.};
  float noise_{-1.};
  float avgQ_{-999.};
  float minQ_{-999.};
  float maxQ_{-999.};
  float medQ_{-999.};


  ClassDef(EventReadout, 1);

};  // EventReadout

}  // namespace trigscint

#endif  // TRIGSCINT_EVENT_EVENTREADOUT_H
