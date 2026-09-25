#ifndef TRIGGER_EVENT_TRIGCLUSTERNNSCORE_H
#define TRIGGER_EVENT_TRIGCLUSTERNNSCORE_H

#include <vector>

#include "TObject.h"  //For ClassDef

namespace trigger {

/**
 * @class TrigClusterNNScore
 * @brief Output of the ECal trigger-cluster NN classifier for one event
 *
 * Holds the 9 raw logits in trained class order: 0 = 1e bkg, 1 = A',
 * 2 = eN neutron, 3 = proton, 4 = pi+-, 5 = K+-, 6 = K0, 7 = photon,
 * 8 = other. P(bkg) is the class-0 softmax, computed in double precision
 * because 1 - P(bkg) saturates to 1 in float.
 *
 * The current 1 kHz working point is to fire if pBkg() <= 2.6726722717e-4
 */
class TrigClusterNNScore {
 public:
  TrigClusterNNScore() = default;

  virtual ~TrigClusterNNScore() = default;

  void setLogits(const std::vector<float>& logits) { logits_ = logits; }
  void setPBkg(double p_bkg) { p_bkg_ = p_bkg; }

  const std::vector<float>& logits() const { return logits_; }
  double pBkg() const { return p_bkg_; }

  void clear();

 private:
  std::vector<float> logits_;
  double p_bkg_{0.0};

  /// ROOT Dictionary class definition macro
  ClassDef(TrigClusterNNScore, 1);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGCLUSTERNNSCORE_H
