/**
 * @file EventReadout.h
 * @brief Class that stores full reconstructed (linearized) readout QIE sample
 * from the TS
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
  void clear(Option_t* option = "");

  /**
   * Print out the object.
   */
  void print(Option_t* option = "") const;

  /**
   * Set channel (linearized. charge) pedestal
   *
   * @param pedestal The pedestal of the channel
   */
  void setPedestal(const float pedestal) { pedestal_ = pedestal; };

  /// Get the pedestal calculated only from first N time samples

  float getEarlyPedestal() const { return early_pedestal_; }

  /**
   * Set channel (linearized. charge) pedestal calculated from beginning of
   * readout
   *
   * @param pedestal The pedestal of the first N time samples in the channel
   */
  void setEarlyPedestal(const float earlyPed) { early_pedestal_ = earlyPed; };

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
   * Set channel data quality flag. This is the binary
   * combination of 4 flags:
   * spike: 1
   * plateau: 10
   * long pulse: 100 (not implemented yet)
   * oscillation: 1000
   * @param flag The quality flag of the channel
   */
  void setQualityFlag(const uint flag) { this->flag_ = flag; };

  /// Get the channel data quality flag

  float getQualityFlag() const { return flag_; }

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
  void setQError(const std::vector<float> qErr) { q_errs_ = qErr; }

  /**
   * Get charges of all time samples
   */
  std::vector<float> getQError() const { return q_errs_; }

  /**
   * Set channel (linearized, charge-equiv) average charge
   *
   * @param totQ The (time sample) average charge of the channel
   */
  void setTotQ(const float totQ) { tot_q_ = totQ; };

  /// Get the channel totQ
  float getTotQ() const { return tot_q_; }

  /**
   * Set channel (linearized, charge-equiv) average charge
   *
   * @param avgQ The (time sample) average charge of the channel
   */
  void setAvgQ(const float avgQ) { avg_q_ = avgQ; };

  /// Get the channel avgQ

  float getAvgQ() const { return avg_q_; }

  /**
   * Set channel (linearized, charge-equiv) minimum charge
   *
   * @param minQ The (time sample) minimum charge of the channel
   */
  void setMinQ(const float minQ) { min_q_ = minQ; };

  /// Get the channel minQ

  float getMinQ() const { return min_q_; }

  /**
   * Set channel (linearized, charge-equiv) maximum charge
   *
   * @param maxQ The (time sample) maximum charge of the channel
   */
  void setMaxQ(const float maxQ) { max_q_ = maxQ; };

  /// Get the channel maxQ

  float getMaxQ() const { return max_q_; }

  /**
   * Set channel (linearized, charge-equiv) median charge
   *
   * @param medQ The (time sample) median charge of the channel
   */
  void setMedQ(const float medQ) { med_q_ = medQ; };

  /// Get the channel medQ

  float getMedQ() const { return med_q_; }

  /**
   * Set channel readout itme offset (in units of samples)
   *
   * @param timeOffset The (time sample) offset in channel readout
   */
  void setTimeOffset(const int timeOffset) { time_offset_ = timeOffset; };

  /// Get the channel timeOffset

  int getTimeOffset() const { return time_offset_; }

  /**
   * Set channel readout fiber number
   *
   * @param fiberNb The channel readout fiber number
   */
  void setFiberNb(const int fiberNb) { fiber_nb_ = fiberNb; };

  /// Get the channel fiberNb

  int getFiberNb() const { return fiber_nb_; }

  /**
   * A dummy operator overloading
   * @note required for declaring std::vector<> in EventDef.h
   */
  bool operator<(const EventReadout& rhs) const {
    return this->chan_id_ < rhs.chan_id_;
  }

 private:
  /// analog to digital counts
  std::vector<float> qs_;
  std::vector<float> q_errs_;

  float pedestal_{-999.};
  float early_pedestal_{-999.};
  float noise_{-1.};
  float tot_q_{-999.};
  float avg_q_{-999.};
  float min_q_{-999.};
  float max_q_{-999.};
  float med_q_{-999.};
  int time_offset_{0};
  int fiber_nb_{-1};

  uint flag_{0};
  ClassDef(EventReadout, 2);

};  // EventReadout

}  // namespace trigscint

#endif  // TRIGSCINT_EVENT_EVENTREADOUT_H
