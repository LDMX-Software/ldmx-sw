#ifndef EVENT_HCALHIT_H_
#define EVENT_HCALHIT_H_

// ROOT
#include "Event/CalorimeterHit.h"

namespace event {

class HcalHit : public CalorimeterHit {

    public:

        HcalHit() {;}

        virtual ~HcalHit() {;}

  int getLayer() const {   return layer_; }
  void setLayer(int layer) { layer_=layer; }
  int getPE() const { return pe_; }
  void setPE(int pe) { pe_=pe; }
  float getZpos() const { return zpos_; }
  void setZpos(float zpos) { zpos_=zpos; }

  private:

        int layer_{0};
        int pe_{0};
        float zpos_{0};
    /**
     * The ROOT class definition.
     */
    ClassDef(HcalHit, 1);
};

}


#endif /* INCLUDE_EVENT_HCALHIT_H_ */
