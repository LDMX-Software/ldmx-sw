#ifndef EVENT_ECALHIT_H_
#define EVENT_ECALHIT_H_

// ROOT
#include "Event/CalorimeterHit.h"

namespace event {

class EcalHit : public CalorimeterHit {

    public:

        EcalHit() {;}

        virtual ~EcalHit() {;}

  int getLayer() const {   return layer_; }
  void setLayer(int layer) { layer_=layer; }

  private:

        int layer_{0};

    /**
     * The ROOT class definition.
     */
    ClassDef(EcalHit, 1);
};

}


#endif /* INCLUDE_EVENT_ECALHIT_H_ */
