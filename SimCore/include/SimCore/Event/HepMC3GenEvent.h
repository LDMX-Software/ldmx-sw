//
// Created by Wesley Ketchum on 4/26/24.
//
// This is just a simple extension of the HepMC3::GenEvent
// for use on the ldmx Event Bus.
//

#ifndef SIM_CORE_HEPMC3GENEVENT_H
#define SIM_CORE_HEPMC3GENEVENT_H

#include "TObject.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/Data/GenEventData.h"

namespace ldmx {

    class HepMC3GenEvent : public HepMC3::GenEventData {

    public:

        //HepMC3GenEvent() : HepMC3::GenEventData() {}
        //HepMC3GenEvent(const HepMC3::GenEvent& event) : HepMC3::GenEvent(event) {}

        void Clear();
        void Print() const;

        HepMC3::GenEvent getHepMCGenEvent() const;

        std::string get_as_string() const;

    public:
        ClassDef(HepMC3GenEvent, 1);

    };

}

#endif //SIM_CORE_HEPMC3GENEVENT_H
