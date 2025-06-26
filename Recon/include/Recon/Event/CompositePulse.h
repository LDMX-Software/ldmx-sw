#ifndef RECON_EVENT_COMPOSITEPULSE_H_
#define RECON_EVENT_COMPOSITEPULSE_H_

#include <vector>

#include "TF1.h"
#include "TObject.h"  //for ClassDef

namespace ldmx {

/**
 * CompositePulse
 *
 * An emulator for a pulse that the chip needs to read.
 * This handles merging two hits that are "close-enough"
 * to one another.
 */
class CompositePulse {
 public:
  /**
   * Constructore
   *
   * Connect this pulse emulator with the pulse
   * shape function already configured by the chip
   * emulator.
   */
  CompositePulse(TF1 func, const double& g, const double& p)
      : pulseFunc_{func}, gain_{g}, pedestal_{p} {}

  CompositePulse() = default;

  virtual ~CompositePulse(){};

  void Clear() {};

  /**
   * Put another hit into this composite pulse.
   *
   * If the hit is within the merge input of a hit already
   * included, then it is merged with that hit. Otherwise,
   * it is included as its own hit.
   *
   * @param[in] hit voltage,time pair representing a sime hit
   * @param[in] hit_merge_ns maximum time separation [ns] to merge two hits
   */
  void addOrMerge(const std::pair<double, double>& hit, double hit_merge_ns);

  /**
   * Find the time at which we cross the input level.
   *
   * We use the midpoint algorithm, assuming the input low
   * is below the threshold and hight is above.
   *
   * @param[in] low minimum value (below threshold) to start search at [mV]
   * @param[in] high maximum value (above threshold) to start search at [mV]
   * @param[in] level threshold to look for time [mV]
   * @param[in] prec precision with which to look [mV]
   * @returns time [ns] at which the pulse cross level
   */
  double findCrossing(double low, double high, double level,
                      double prec = 0.01);

  /// Configure the pulses for the current chip
  void setGainPedestal(double gain, double pedestal) {
    gain_ = gain;
    pedestal_ = pedestal;
  }

  /**
   * Evaluating this object as a function
   * gives the same result as at.
   *
   * @see at
   */
  double operator()(double time) const { return at(time); }

  /**
   * Measure the voltage at the input time
   *
   * Includes the effects from all pulses but
   * does not put any noise into the measurement.
   *
   * @param[in] time time to measure [ns]
   * @return voltage at that time [mV]
   */
  double at(double time) const {
    double signal = gain_ * pedestal_;
    for (auto hit : hits_)
      signal += hit.first * pulseFunc_.Eval(time - hit.second);
    return signal;
  };

  /// Get list of individual pulses that are entering the chip
  const std::vector<std::pair<double, double>>& hits() const { return hits_; }

 private:
  /**
   * pulses entering the chip
   *
   * The pair is {voltage amplitude [mV], time of peak [ns]}
   */
  std::vector<std::pair<double, double>> hits_;

  /// reference to pulse shape function shared by all pulses
  TF1 pulseFunc_;

  /// gain for current chip we are emulating
  double gain_;

  /// pedestal for current chip we are emulating
  double pedestal_;

  ClassDef(CompositePulse, 1);

};  // CompositePulse

}  // namespace ldmx

#endif
