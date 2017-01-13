#ifndef EVENT_CALORIMETERHIT_H_
#define EVENT_CALORIMETERHIT_H_

// ROOT
#include "TObject.h"

namespace event {

class CalorimeterHit : public TObject {

    public:

        CalorimeterHit() {;}

        virtual ~CalorimeterHit() {;}

        int getID() {
            return id_;
        }

        void setID(int id) {
            id_ = id;
        }

        float getEdep() {
            return edep_;
        }

        void setEdep(float edep) {
            edep_ = edep;
        }

        float getTime() {
            return time_;
        }

        void setTime(float time) {
            time_ = time;
        }

    private:

        int id_{0};
        float edep_{0};
        float time_{0};

    /**
     * The ROOT class definition.
     */
    ClassDef(CalorimeterHit, 1);
};

}





#endif /* INCLUDE_EVENT_CALORIMETERHIT_H_ */
