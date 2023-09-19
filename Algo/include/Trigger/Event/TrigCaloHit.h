#ifndef TRIGGER_EVENT_TRIGCALOHIT_H
#define TRIGGER_EVENT_TRIGCALOHIT_H

// ROOT
#include "TObject.h"  //For ClassDef

namespace trigger {

// Forward declaration needed by typedef
class TrigCaloHit;
typedef std::vector<TrigCaloHit> TrigCaloHitCollection;

/**
 * @class TrigCaloHit
 * @brief Class for calo hits used in trigger computations
 */
class TrigCaloHit {
 public:

  TrigCaloHit() = default;

  TrigCaloHit(float x, float y, float z, float e=0);

  virtual ~TrigCaloHit() = default;

  bool operator<(const TrigCaloHit &h) { return e_ < h.e_; }

  void Clear() { x_=0; y_=0; z_=0; e_=0; }
  
  void setEnergy(float e) { e_ = e; }
  void setXYZ(float x, float y, float z) { x_=x;  y_=y;  z_=z; }

  float x(){ return x_; }
  float y(){ return y_; }
  float z(){ return z_; }
  float e(){ return e_; }
  float energy(){ return e_; }

 private:
  
  float x_{0};
  float y_{0};
  float z_{0};
  float e_{0};
  
  /// ROOT Dictionary class definition macro
  ClassDef(TrigCaloHit, 1);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGCALOHIT_H
