#pragma once

#include <iostream>
#include "TObject.h"

namespace ldmx {

/**
 * Result of fitting a pulse shape to the ADC samples of a single readout strip.
 *
 * Stores the fitted amplitude and hit time extracted from the N-sample ADC
 * waveform, along with the goodness-of-fit.  Downstream clustering combines
 * adjacent strips into a Measurement (local position + covariance).
 *
 * Coordinate convention
 * ---------------------
 *   amplitude  : fitted peak ADC counts (pedestal-subtracted).
 *   t0         : fitted hit arrival time [ns] in the sample-window reference
 *                frame (i.e., t0 = 0 means the hit peaked at sample 0).
 *   chi2 / ndf : goodness of fit (ndf = n_samples − 2).
 */
class FittedSiStripHit {
 public:
  FittedSiStripHit() = default;

  FittedSiStripHit(int layer_id, int strip_id,
                   float amplitude, float t0,
                   float chi2, int ndf)
      : layer_id_(layer_id), strip_id_(strip_id),
        amplitude_(amplitude), t0_(t0),
        chi2_(chi2), ndf_(ndf) {}

  virtual ~FittedSiStripHit() = default;

  void clear() {
    layer_id_ = -1;  strip_id_ = -1;
    amplitude_ = 0;  t0_ = 0;
    chi2_ = 0;       ndf_ = 0;
  }

  // --- Getters ---
  int   getLayerID()   const { return layer_id_;  }
  int   getStripID()   const { return strip_id_;  }
  /// Fitted pedestal-subtracted peak amplitude [ADC counts].
  float getAmplitude() const { return amplitude_; }
  /// Fitted hit arrival time [ns] in the sample-window reference frame.
  float getT0()        const { return t0_;        }
  float getChi2()      const { return chi2_;      }
  int   getNDF()       const { return ndf_;       }
  float getReducedChi2() const {
    return (ndf_ > 0) ? chi2_ / ndf_ : 0.f;
  }

  // --- Setters ---
  void setLayerID(int v)    { layer_id_  = v; }
  void setStripID(int v)    { strip_id_  = v; }
  void setAmplitude(float v){ amplitude_ = v; }
  void setT0(float v)       { t0_        = v; }
  void setChi2(float v)     { chi2_      = v; }
  void setNDF(int v)        { ndf_       = v; }

  friend std::ostream& operator<<(std::ostream& o,
                                  const FittedSiStripHit& h) {
    o << "[ FittedSiStripHit ]: layer=" << h.layer_id_
      << " strip=" << h.strip_id_
      << " amp=" << h.amplitude_ << " ADC"
      << " t0=" << h.t0_ << " ns"
      << " chi2/ndf=" << h.chi2_ << "/" << h.ndf_;
    return o;
  }

 protected:
  int   layer_id_{-1};
  int   strip_id_{-1};
  float amplitude_{0};   ///< Fitted peak amplitude [ADC counts], pedestal-subtracted.
  float t0_{0};          ///< Fitted hit time [ns] in sample-window frame.
  float chi2_{0};
  int   ndf_{0};

  ClassDef(FittedSiStripHit, 1);
};

}  // namespace ldmx
