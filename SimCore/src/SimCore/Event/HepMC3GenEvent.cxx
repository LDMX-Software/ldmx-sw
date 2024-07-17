//
// Created by Wesley Ketchum on 4/26/24.
//

#include "SimCore/Event/HepMC3GenEvent.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/Print.h"

namespace ldmx{

    void HepMC3GenEvent::Clear() {
        this->particles.clear();
        this->vertices.clear();
        this->links1.clear();
        this->links2.clear();
        this->attribute_id.clear();
        this->attribute_name.clear();
        this->attribute_string.clear();
    }

    void HepMC3GenEvent::Print() const {
        HepMC3::GenEvent ev; ev.read_data(*this);
        HepMC3::Print::line(ev,true); //print attributes
    }

}
