#ifndef TRIGGER_EVENT_TRIGMIP_H
#define TRIGGER_EVENT_TRIGMIP_H

#include "TObject.h"  //For ClassDef

namespace trigger {

/**
 * @class TrigMip
 * @brief Class for clusters built from trigger calo hits_
 */
class TrigMip {
 public:
  TrigMip() = default;

  virtual ~TrigMip() = default;

  bool operator<(const TrigMip &h) const { return length_ < h.length_; }

  void setStartLayer(int startLayer) { start_layer_ = startLayer; }
  void setEndLayer(int endLayer) { end_layer_ = endLayer; }
  void setNHits(int nHits) { n_hits_ = nHits; }
  void setNHoles(int nHoles) { n_holes_ = nHoles; }
  void setLength(int length) { length_ = length; }
  void setSumEinIsolationRegion(float sum) { sum_e_in_isolation_region_ = sum; }

  int startLayer() const { return start_layer_; }
  int endLayer() const { return end_layer_; }
  int nHits() const { return n_hits_; }
  int nHoles() const { return n_holes_; }
  int length() const { return length_; }
  float SumEinIsolationRegion() const { return sum_e_in_isolation_region_; }

  void clear();

 private:
  // first draft based on hcal
  int start_layer_{0};
  int end_layer_{0};
  int n_hits_{0};
  int n_holes_{0};
  int length_{0};
  float sum_e_in_isolation_region_{0.0f};

  /// ROOT Dictionary class definition macro
  ClassDef(TrigMip, 1);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGMIP_H
