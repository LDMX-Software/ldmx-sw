#ifndef TRIGGER_EVENT_TRIGMIP_H
#define TRIGGER_EVENT_TRIGMIP_H

#include "TObject.h"  //For ClassDef

namespace trigger {

// Forward declaration needed by typedef
class TrigMip;
typedef std::vector<TrigMip> TrigMipCollection;

/**
 * @class TrigMip
 * @brief Class for clusters built from trigger calo hits
 */
class TrigMip {
 public:
  TrigMip() = default;

  // TrigMip(float x, float y, float z, float e = 0);

  virtual ~TrigMip() = default;

  bool operator<(const TrigMip &h) { return length_ < h.length_; }

  // void Clear();

  void setStartLayer(int startLayer) { startLayer_ = startLayer; }
  void setEndLayer(int endLayer) { endLayer_ = endLayer; }
  void setNHits(int nHits) { nHits_ = nHits; }
  void setNHoles(int nHoles) { nHoles_ = nHoles; }
  void setLength(int length) { length_ = length; }
  void setSumEinIsolationRegion(float sum) { SumEinIsolationRegion_ = sum; }

  int startLayer() const { return startLayer_; }
  int endLayer() const { return endLayer_; }
  int nHits() const { return nHits_; }
  int nHoles() const { return nHoles_; }
  int length() const { return length_; }
  float SumEinIsolationRegion() const { return SumEinIsolationRegion_; }

 private:
  // first draft based on hcal
  int startLayer_{0};
  int endLayer_{0};
  int nHits_{0};
  int nHoles_{0};
  int length_{0};
  float SumEinIsolationRegion_{0.0f};

  /// ROOT Dictionary class definition macro
  ClassDef(TrigMip, 1);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGCALOCLUSTER_H
