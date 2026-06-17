#pragma once

#include <cmath>
#include <memory>
#include <string>

namespace tracking::digitization {

/**
 * Abstract base class for silicon-strip readout pulse shapes.
 *
 * The pulse shape describes the time response of the front-end amplifier/
 * shaper to a delta-function charge injection at t = 0.  All shapes are
 * normalised so that their peak value is 1 (peak-normalised).
 *
 * Two concrete shapes are provided:
 *
 *   CRRC     — single CR·RC shaper (one differentiation, one integration):
 *                f(t) = (t/tp) · exp(1 − t/tp)    for t ≥ 0
 *              Peaks at t = tp.
 *
 *   FourPole — fourth-order shaper (CR + three RC stages), as used by the
 *              HPS SVT APV25 readout.  Two time constants tp and tp2:
 *                f(t) ∝ A · [exp(−t/tp) − exp(−t/tp2)·(1 + t·B + ½(t·B)²)]
 *              Peaks near t ≈ 3·(tp·tp2³)^(1/4).
 *
 * Coordinate convention: t is measured in nanoseconds from the moment the
 * charge arrives at the collection electrode.
 */
class PulseShape {
 public:
  virtual ~PulseShape() = default;

  /**
   * Evaluate the peak-normalised pulse amplitude at time t [ns] after
   * charge arrival.  Returns 0 for t < 0.
   */
  virtual double eval(double t) const = 0;

  /**
   * Factory: construct a pulse shape by name.
   *
   * @param name  "CRRC" or "FourPole".
   * @param tp    Peaking time [ns].
   * @param tp2   Second time constant [ns] (FourPole only; ignored for CRRC).
   */
  static std::unique_ptr<PulseShape> make(const std::string& name, double tp,
                                          double tp2 = 0.0);
};

// ---------------------------------------------------------------------------
// CR-RC shaper (single-pole)
// ---------------------------------------------------------------------------

/**
 * Single CR·RC shaper.
 *
 *   f(t) = (t/tp) · exp(1 − t/tp)    for t ≥ 0, else 0
 *
 * Peaks at t = tp with f(tp) = 1.
 *
 * @param tp  Peaking time [ns].
 */
class CRRCShape : public PulseShape {
 public:
  explicit CRRCShape(double tp) : tp_(tp) {}

  double eval(double t) const override {
    if (t <= 0.0) return 0.0;
    return (t / tp_) * std::exp(1.0 - t / tp_);
  }

 private:
  double tp_;
};

// ---------------------------------------------------------------------------
// Four-pole shaper (CR + 3 RC stages)
// ---------------------------------------------------------------------------

/**
 * Fourth-order shaper with two time constants.
 *
 * The un-normalised amplitude is:
 *   g(t) = A · [exp(−t/tp) − exp(−t/tp2) · (1 + t·B + ½·(t·B)²)]
 * where
 *   A = tp² / (tp − tp2)³
 *   B = (tp − tp2) / (tp · tp2)
 *
 * The peak position is approximated by
 *   t_peak ≈ 3 · (tp · tp2³)^(1/4)
 *
 * eval(t) returns g(t) / g(t_peak), i.e. peak-normalised.
 *
 * @param tp   First (slower) time constant [ns].
 * @param tp2  Second (faster) time constant [ns].  Must be < tp.
 */
class FourPoleShape : public PulseShape {
 public:
  FourPoleShape(double tp, double tp2);

  double eval(double t) const override;

 private:
  double tp_, tp2_;
  double A_, B_;
  double peak_amp_;  ///< g(t_peak), used for normalisation.
};

}  // namespace tracking::digitization
