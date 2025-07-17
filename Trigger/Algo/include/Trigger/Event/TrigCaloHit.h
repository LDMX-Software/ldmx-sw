#ifndef TRIGGER_EVENT_TRIGCALOHIT_H
#define TRIGGER_EVENT_TRIGCALOHIT_H

// ROOT
#include "TObject.h"  //For ClassDef

namespace trigger {

// Forward declaration needed by typedef
// class TrigCaloHit;
// typedef std::vector<TrigCaloHit> TrigCaloHitCollection;

/**
 * @class TrigCaloHit
 * @brief Class for calo hits used in trigger computations
 */
class TrigCaloHit {
 public:
  TrigCaloHit() = default;

  TrigCaloHit(float position_x, float position_y, float position_z,
              float energy = 0);

  virtual ~TrigCaloHit() = default;

  bool operator<(const TrigCaloHit &h) { return energy_ < h.energy_; }

  void Clear() {
    position_x_ = 0;
    position_y_ = 0;
    position_z_ = 0;
    energy_ = 0;
    layer_ = 0;
    strip_ = 0;
    module_section_ = 0;
  }

  void setEnergy(float energy) { energy_ = energy; }
  void setXYZ(float position_x, float position_y, float position_z) {
    position_x_ = position_x;
    position_y_ = position_y;
    position_z_ = position_z;
  }
  void setLayer(int layer) { layer_ = layer; }
  void setStrip(int strip) { strip_ = strip; }
  void setModule(int module) { module_section_ = module; }
  void setSection(int module_section) { module_section_ = module_section; }

  float position_x() const { return position_x_; }
  float position_y() const { return position_y_; }
  float position_z() const { return position_z_; }
  float energy() const { return energy_; }

  int layer() const { return layer_; }
  int strip() const { return strip_; }
  int module() const { return module_section_; }
  int section() const { return module_section_; }

 private:
  float position_x_{0};
  float position_y_{0};
  float position_z_{0};
  float energy_{0};
  int layer_{0};
  int strip_{0};
  int module_section_{0};

  /// ROOT Dictionary class definition macro
  ClassDef(TrigCaloHit, 2);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGCALOHIT_H
