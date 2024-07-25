//
// Created by Wesley Ketchum on 4/26/24.
//

#include "SimCore/Event/HepMC3GenEvent.h"
#include "HepMC3/Print.h"
#include "HepMC3/WriterAscii.h"

#include <string>
#include <sstream>

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

    HepMC3::GenEvent HepMC3GenEvent::getHepMCGenEvent() const {
      HepMC3::GenEvent ev; ev.read_data(*this);
      return ev;
    }

    std::string HepMC3GenEvent::get_as_string() const {
      HepMC3::GenEvent ev; ev.read_data(*this);

      std::stringstream ss;
      HepMC3::WriterAscii writer(ss);

      writer.write_event(ev);
      return ss.str();

    }
}
