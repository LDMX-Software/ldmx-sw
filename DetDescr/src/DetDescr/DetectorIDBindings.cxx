#include "DetDescr/DetectorIDBindings.h"

BOOST_PYTHON_MODULE(libDetDescr) {
  using namespace boost::python;
  using namespace ldmx;
  using RawValue = DetectorID::RawValue;
  class_<DetectorID>("DetectorID", init<>());
  class_<HcalAbstractID>("HcalAbstractID", init<>());
  class_<EcalAbstractID>("EcalAbstractID", init<>());
  class_<EcalID>("EcalID", init<>())
      .def(init<RawValue>())
      .def(init<unsigned int, unsigned int, unsigned int>())
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>())
      .def(init<unsigned int, unsigned int,
                std::pair<unsigned int, unsigned int>>())
      .def("module", &EcalID::module)
      .def("layer", &EcalID::layer)
      .def("cell", &EcalID::cell)
      .def("getCellUV", &EcalID::getCellUV)
      .def("raw", &EcalID::raw);
  class_<EcalElectronicsID>("EcalElectronicsID", init<>())
      .def(init<RawValue>())
      .def(init<unsigned int, unsigned int, unsigned int>())
      .def("fiber", &EcalElectronicsID::fiber)
      .def("elink", &EcalElectronicsID::elink)
      .def("channel", &EcalElectronicsID::channel)
      .def("index", &EcalElectronicsID::index)
      .def("raw", &EcalElectronicsID::raw);
  class_<EcalTriggerID>("EcalTriggerID", init<>())
      .def(init<RawValue>())
      .def(init<unsigned int, unsigned int, unsigned int>())
      .def("module", &EcalTriggerID::module)
      .def("layer", &EcalTriggerID::layer)
      .def("triggercell", &EcalTriggerID::triggercell)
      .def("raw", &EcalTriggerID::raw);
  //  Currently not actually defined
  // .def("getCellUV", &EcalTriggerID::getCellUV);
  class_<HcalID>("HcalID", init<>())
      .def(init<RawValue>())
      .def(init<unsigned int, unsigned int, unsigned int>())
      .def("section", &HcalID::section)
      .def("layer", &HcalID::layer)
      .def("strip", &HcalID::strip)
      .def("raw", &HcalID::raw);
  class_<HcalElectronicsID>("HcalElectronicsID", init<>())
      .def(init<RawValue>())
      .def(init<unsigned int, unsigned int, unsigned int>())
      .def("fiber", &HcalElectronicsID::fiber)
      .def("elink", &HcalElectronicsID::elink)
      .def("channel", &HcalElectronicsID::channel)
      .def("index", &HcalElectronicsID::index)
      .def("idFromIndex", &HcalElectronicsID::idFromIndex)
      .def("raw", &HcalElectronicsID::raw);
  class_<HcalDigiID>("HcalDigiID", init<>())
      .def(init<RawValue>())
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>())
      .def("section", &HcalDigiID::section)
      .def("layer", &HcalDigiID::layer)
      .def("strip", &HcalDigiID::strip)
      .def("end", &HcalDigiID::end)
      .def("isNegativeEnd", &HcalDigiID::isNegativeEnd)
      .def("raw", &HcalElectronicsID::raw);
  class_<HcalTriggerID>("HcalTriggerID", init<>())
      .def(init<RawValue>())
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>())
      .def("section", &HcalTriggerID::section)
      .def("layer", &HcalTriggerID::layer)
      .def("superstrip", &HcalTriggerID::superstrip)
      .def("end", &HcalTriggerID::end)
      .def("isNegativeEnd", &HcalTriggerID::isNegativeEnd)
      .def("isComposite", &HcalTriggerID::isComposite)
      .def("raw", &HcalTriggerID::raw);
}
