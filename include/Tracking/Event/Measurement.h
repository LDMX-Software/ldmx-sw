#ifndef TRACKING_EVENT_MEASUREMENT_H_
#define TRACKING_EVENT_MEASUREMENT_H_

#include <array>
#include "TObject.h"

namespace ldmx {
class Measurement {
  
 public:
  
  Measurement(){};
  
  virtual ~Measurement(){};
  
  void setGlobalPosition(float x, float y, float z) {x_ = x; y_ = y; z_ = z;};
  std::array<float,3>  getGlobalPosition() {return std::array<float,3>{{x_,y_,z_}};};

  void setLocalPosition(float u, float v) {u_ = u; v_ = v;};
  std::array<float,2>  getLocalPosition() {return std::array<float,2>{{u_, v_}};};
  
  void setTime(float t) {t_ = t; };
  float getTime() {return t_;};

  void setLayer(int layerid) {layerid_ = layerid;};
  int getLayer() {return layerid_;};

 private:

  float x_, y_, z_, t_;
  float u_, v_;
  int layerid_;
    
  ClassDef(Measurement,1);
}; //Measurement

}//namespace ldmx

#endif //TRACKING_EVENT_MEASUREMENT_H_
