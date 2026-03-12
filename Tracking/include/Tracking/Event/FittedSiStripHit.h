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
                   float chi2, int ndf,
                   int track_id = -1, int pdg_id = 0, int sim_hit_id = -1,
                   float edep = 0.f)
      : layer_id_(layer_id), strip_id_(strip_id),
        amplitude_(amplitude), t0_(t0),
        chi2_(chi2), ndf_(ndf),
        track_id_(track_id), pdg_id_(pdg_id), sim_hit_id_(sim_hit_id),
        edep_(edep) {}

  virtual ~FittedSiStripHit() = default;

  void clear() {
    layer_id_ = -1;  strip_id_ = -1;
    amplitude_ = 0;  t0_ = 0;
    chi2_ = 0;       ndf_ = 0;
    track_id_ = -1;  pdg_id_ = 0;  sim_hit_id_ = -1;
    edep_ = 0.f;
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
  /// Geant4 track ID of the particle that created this hit (-1 if unknown).
  int getTrackID()    const { return track_id_;   }
  /// PDG particle ID of the particle that created this hit (0 if unknown).
  int getPdgID()      const { return pdg_id_;     }
  /// Detector ID of the originating SimTrackerHit (-1 if unknown).
  int getSimHitID()   const { return sim_hit_id_; }
  /// Energy deposited by the parent SimTrackerHit [MeV] (0 if unknown).
  float getEdep()     const { return edep_;       }

  // --- Setters ---
  void setLayerID(int v)    { layer_id_  = v; }
  void setStripID(int v)    { strip_id_  = v; }
  void setAmplitude(float v){ amplitude_ = v; }
  void setT0(float v)       { t0_        = v; }
  void setChi2(float v)     { chi2_      = v; }
  void setNDF(int v)        { ndf_       = v; }
  void setTrackID(int v)    { track_id_  = v; }
  void setPdgID(int v)      { pdg_id_    = v; }
  void setSimHitID(int v)   { sim_hit_id_ = v; }
  void setEdep(float v)     { edep_      = v; }

  friend std::ostream& operator<<(std::ostream& o,
                                  const FittedSiStripHit& h) {
    o << "[ FittedSiStripHit ]: layer=" << h.layer_id_
      << " strip=" << h.strip_id_
      << " amp=" << h.amplitude_ << " ADC"
      << " t0=" << h.t0_ << " ns"
      << " chi2/ndf=" << h.chi2_ << "/" << h.ndf_
      << " track_id=" << h.track_id_
      << " pdg_id=" << h.pdg_id_
      << " sim_hit_id=" << h.sim_hit_id_
      << " edep=" << h.edep_ << " MeV";
    return o;
  }

 protected:
  int   layer_id_{-1};
  int   strip_id_{-1};
  float amplitude_{0};   ///< Fitted peak amplitude [ADC counts], pedestal-subtracted.
  float t0_{0};          ///< Fitted hit time [ns] in sample-window frame.
  float chi2_{0};
  int   ndf_{0};

  // Truth information (for MC truth matching; -1/0 means not set)
  /// Geant4 track ID of the particle that created this hit.
  int track_id_{-1};
  /// PDG particle ID of the particle that created this hit.
  int pdg_id_{0};
  /// Detector ID of the originating SimTrackerHit.
  int sim_hit_id_{-1};
  /// Energy deposited by the parent SimTrackerHit [MeV].
  float edep_{0.f};

  ClassDef(FittedSiStripHit, 3);
};

}  // namespace ldmx
