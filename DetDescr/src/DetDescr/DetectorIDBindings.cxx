#include "DetDescr/DetectorIDBindings.h"

BOOST_PYTHON_MODULE(libDetDescr) {
  using namespace boost::python;
  using namespace ldmx;
  using RawValue = DetectorID::RawValue;
  class_<DetectorID>("DetectorID", init<>());
  class_<HcalAbstractID>("HcalAbstractID", init<>());
  class_<EcalAbstractID>("EcalAbstractID", init<>());
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
