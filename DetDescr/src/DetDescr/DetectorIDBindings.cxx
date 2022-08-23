#if DETECTORID_BINDINGS_ENABLED
#include <boost/python.hpp>

#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalAbstractID.h"
#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include "DetDescr/HcalAbstractID.h"
#include "DetDescr/HcalDigiID.h"
#include "DetDescr/HcalElectronicsID.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/HcalTriggerID.h"
#include "DetDescr/SimSpecialID.h"
#include "DetDescr/TrackerID.h"
#include "DetDescr/TrigScintID.h"

BOOST_PYTHON_MODULE(libDetDescr) {
  // init<Ts...>(...) defines constructors.
  //
  // To add names to parameters, add an args() call as a parameter to
  // the init function call.
  //
  // To add a docstring to a constructor, pass it as the last parameter to the
  // init function call (as a string literal)
  //
  // One constructor can be called within the class_<>() function call. To have
  // multiple constructors, you use init<> together with .def() to append
  // others.
  //
  // If you want to add a docstring to the class as a whole, pass it as the
  // second parameter (*note:* not last if you want a constructor) to the
  // class_<>() function call
  //
  // def(...) defines functions and .def() defines member functions.
  //
  // To add names names to parameters, add an args() call as a parameter
  //
  // To add a docstring, pass a string literal as the last parameter to the
  // def/.def function call
  //
  // args(...) give names to parameters. This both adds the names to the
  // generated documentation and allows using python's named parameters
  //
  // .staticmethod("name") is required for any static member functions. Note
  // that things will compile without it but the function won't be callable from
  // python
  //
  // Enums need each value to be specified with the .value() function. Any enum
  // values specified before export_values() will be accessible directly from
  // the module while others will need fully qualified names (module.enum.value)

  using namespace boost::python;
  using namespace ldmx;
  using RawValue = DetectorID::RawValue;

  enum_<SubdetectorIDType>("SubdetectorID")
      .value("SD_NULL", SubdetectorIDType::SD_NULL)
      .value("SD_TRACKER_TAGGER", SubdetectorIDType::SD_TRACKER_TAGGER)
      .value("SD_TRIGGER_SCINT", SubdetectorIDType::SD_TRIGGER_SCINT)
      .value("SD_ACTVE_TARGET", SubdetectorIDType::SD_ACTVE_TARGET)
      .value("SD_TRACKER_RECOIL", SubdetectorIDType::SD_TRACKER_RECOIL)
      .value("SD_ECAL", SubdetectorIDType::SD_ECAL)
      .value("SD_HCAL", SubdetectorIDType::SD_HCAL)
      .value("SD_SIM_SPECIAL", SubdetectorIDType::SD_SIM_SPECIAL)
      .value("EID_TRACKER", SubdetectorIDType::EID_TRACKER)
      .value("EID_TRIGGER_SCINT", SubdetectorIDType::EID_TRIGGER_SCINT)
      .value("EID_HCAL", SubdetectorIDType::EID_HCAL)
      .value("EID_HCAL", SubdetectorIDType::EID_HCAL)
      .export_values();

  class_<DetectorID>("DetectorID", init<>(args("self")));
  class_<HcalAbstractID>("HcalAbstractID", init<>(args("self")));
  class_<EcalAbstractID>("EcalAbstractID", init<>(args("self")));
  class_<EcalID>("EcalID",
                 "Extension of DetectorID providing access to ECal layers and "
                 "cell numbers in a hex grid",
                 init<>(args("self"), "Empty ECAL id (but not null!)"))
      .def(init<RawValue>(args("self", "rawid"), "Create from raw number"))
      .def(init<unsigned int, unsigned int, unsigned int>(
          args("self", "layer", "module", "cell"), "Create from pieces"))
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>(
          args("self", "layer", "module", "u", "v"),
          "Create from pieces including u/v cell"))
      .def(init<unsigned int, unsigned int,
                std::pair<unsigned int, unsigned int>>(
          args("self", "layer", "module", "uv"),
          "Create from pieces including u/v cell"))
      .def("module", &EcalID::module,
           "Get the value of the module field from the ID.", args("self"))
      .def("layer", &EcalID::layer,
           "Get the value of the layer field from the ID.", args("self"))
      .def("cell", &EcalID::cell,
           "Get the value of the cell field from the ID.", args("self"))
      //  Requires defining a translator for pair
      // .def("getCellUV", &EcalID::getCellUV, "Get the cell u,v index assuming
      // a CMS-standard 432-cell sensor" )
      .def("raw", &EcalID::raw, "The raw value", args("self"));
  class_<EcalElectronicsID>(
      "EcalElectronicsID",
      "Identifies a location in the Ecal readout chain\n"
      "-- fiber : optical fiber number (backend number), range "
      "assumed O(0-96)\n"
      "-- elink : electronic link number, range assumed O(0-47)\n"
      "-- channel : channel-on-elink, range O(0-37)\n"
      "For transient use only i.e. we use this ID to\n"
      "help translate the digitized data coming off the detector\n"
      "into spatially-important EcalIDs.",
      init<>(args("self"), "Empty EcalElectronics id (but not null!)"))
      .def(init<RawValue>(
          "Create from raw number\n\n Importantly, this is NOT the PackedIndex "
          "value, it is the entire raw value including the subsystem ID.",
          args("self", "rawid")))
      .def(init<unsigned int, unsigned int, unsigned int>(
          args("self", "fiber", "elink", "channel"), "Create from pieces"))
      .def("fiber", &EcalElectronicsID::fiber,
           "Get the value of the fiber from the ID.", args("self"))
      .def("elink", &EcalElectronicsID::elink,
           "Get the value of the elink from the ID.", args("self"))
      .def("channel", &EcalElectronicsID::channel,
           "Get the value of the channel from the ID.", args("self"))
      .def("index", &EcalElectronicsID::index, "Get the compact index value",
           args("self"))
      .def("idFromIndex", &EcalElectronicsID::idFromIndex, args("index"),
           "Construct an electronics id from an index")
      .def("raw", &EcalElectronicsID::raw, "The raw value", args("self"))
      .staticmethod("idFromIndex");
  class_<EcalTriggerID>(
      "EcalTriggerID",
      "Extension of DetectorID providing access to ECal "
      "trigger cell information",
      init<>(args("self"), "Empty EcALTrigger id (but not null!)"))
      .def(init<RawValue>(args("self", "rawid"), "Create from raw number"))
      .def(init<unsigned int, unsigned int, unsigned int>(
          args("self", "layer", "module", "cell"), "Create from pieces"))
      .def("module", &EcalTriggerID::module,
           "Get the value of the module field from the ID.", args("self"))
      .def("layer", &EcalTriggerID::layer,
           "Get the value of the layer field from the ID.", args("self"))
      .def("triggercell", &EcalTriggerID::triggercell,
           "Get the value of the trigger cell field from the ID.", args("self"))
      .def("raw", &EcalTriggerID::raw, "The raw value", args("self"));
  //  Currently not actually defined
  // .def("getCellUV", &EcalTriggerID::getCellUV);
  class_<HcalID>("HcalID", "Implements detector ids for Hcal subdetector",
                 init<>(args("self"), "Empty HcalID (but not null)"))
      .def(init<RawValue>(args("self", "rawid"), "Create from raw number"))
      .def(init<unsigned int, unsigned int, unsigned int>(
          args("self", "section", "layer", "strip"), "Create from pieces"))
      .def("section", &HcalID::section,
           "Get the value of the 'section' field from the ID.", args("self"))
      .def("layer", &HcalID::layer,
           "Get the value of the layer field from the ID.", args("self"))
      .def("strip", &HcalID::strip,
           "Get the value of the 'strip' (bar) field from the ID.",
           args("self"))
      .def("raw", &HcalID::raw, "The raw value", args("self"));
  class_<HcalElectronicsID>(
      "HcalElectronicsID",

      "Identifies a location in the Hcal readout chain\n"
      "-- fiber : optical fiber number (backend number), range assumed "
      "O(0-96)\n"
      "-- elink : electronic link number, range assumed O(0-47)\n"
      "-- channel : channel-on-elink, range O(0-37)\n\n"
      "For transient use only i.e. we use this ID to help translate the "
      "digitized data coming off the detector into spatially-important "
      "HcalDigiIDs.",
      init<>(args("self"), "Empty HCAL id (but not null!)"))
      .def(init<RawValue>("Create from raw number", args("self", "rawid")))
      .def(init<unsigned int, unsigned int, unsigned int>(
          "Create from pieces", args("self", "fiber", "elink", "channel")))
      .def("fiber", &HcalElectronicsID::fiber,
           "Get the value of the fiber from the ID.", args("self"))
      .def("elink", &HcalElectronicsID::elink,
           "Get the value of the elink from the ID.", args("self"))
      .def("channel", &HcalElectronicsID::channel,
           "Get the value of the channel from the ID.", args("self"))
      .def("index", &HcalElectronicsID::index, "Get the compact index value",
           args("self"))
      .def("idFromIndex", &HcalElectronicsID::idFromIndex, args("index"),
           "Create an electronics ID from an index")
      .def("raw", &HcalElectronicsID::raw, "The raw value", args("self"))
      .staticmethod("idFromIndex");
  class_<HcalDigiID>("HcalDigiID",
                     "Extension of HcalAbstractID providing access to HCal "
                     "digi information",
                     init<>(args("self"), "Empty HcalDigiID (but not null)"))
      .def(init<RawValue>("Create from raw number", args("self", "rawid")))
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>(
          "Create from pieces",
          args("self", "section", "layer", "strip", "end")))
      .def("section", &HcalDigiID::section,
           "Get the value of the 'section' field from the ID.", args("self"))
      .def("layer", &HcalDigiID::layer,
           "Get the value of the 'layer' field from the ID.", args("self"))
      .def("strip", &HcalDigiID::strip,
           "Get the value of the 'strip' (bar) field from the ID.",
           args("self"))
      .def("end", &HcalDigiID::end,
           "Get the value of the 'end' (positive/negative) field from the ID.",
           args("self"))
      .def("isNegativeEnd", &HcalDigiID::isNegativeEnd,
           "Get whether the 'end' field from the ID is negative.", args("self"))
      .def("raw", &HcalElectronicsID::raw, "The raw value", args("self"));
  class_<HcalTriggerID>(
      "HcalTriggerID",
      "Extension of DetectorID providing access to HCal trigger cell",
      init<>(args("self"), "Empty HCAL trigger id (but not null!)"))
      .def(init<RawValue>("Create from raw number", args("self", "rawid")))
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>(
          "Create from pieces",
          args("self", "section", "layer", "superstrip", "end")))
      .def("section", &HcalTriggerID::section,
           "Get the value of the 'section' field from the ID.", args("self"))
      .def("layer", &HcalTriggerID::layer,
           "Get the value of the 'layer' field from the ID.", args("self"))
      .def("superstrip", &HcalTriggerID::superstrip,
           "Get the value of the 'superstrip' field from the ID.", args("self"))
      .def("end", &HcalTriggerID::end,
           "Get the value of the 'end' field from the ID.", args("self"))
      .def("isNegativeEnd", &HcalTriggerID::isNegativeEnd,
           "Get whether the 'end' field from the ID is negative.", args("self"))
      .def("isComposite", &HcalTriggerID::isComposite,
           "Get whether the ID is the composite of two bar ends.", args("self"))
      .def("raw", &HcalTriggerID::raw, "The raw value", args("self"));
  class_<SimSpecialID>("SimSpecialID",
                       "Implements detector ids for special simulation-derived "
                       "hits like scoring planes",
                       init<>(args("self"), "Empty id (but not null!)"))
      .def(init<RawValue>("Create from raw number", args("self", "rawid")))
      .def("ScoringPlaneID", &SimSpecialID::ScoringPlaneID,
           "Create a scoring id from pieces", args("plane"))
      .staticmethod("ScoringPlaneID")
      .def("plane", &SimSpecialID::plane,
           "Get the value of the plane field from the ID, if it is a scoring "
           "plane. Otherwise, return -1",
           args("self"))
      .def("subtypePayload", &SimSpecialID::subtypePayload,
           "Get the raw payload contents", args("self"))
      .def("raw", &SimSpecialID::raw, "The raw value", args("self"));
  class_<TrackerID>("TrackerID",
                    "Extension of DetectorID providing access to layer and "
                    "module number for tracker IDs")
      .def(init<RawValue>("Create from a raw id, but check",
                          args("self", "rawid")))
      .def(init<SubdetectorIDType, unsigned int, unsigned int>(
          "Create from pieces", args("self", "system", "layer", "module")))
      .def("module", &TrackerID::module,
           "Get the value of the module field from the ID", args("self"))
      .def("layer", &TrackerID::layer,
           "Get the value of the layer field from the ID", args("self"))
      .def("raw", &TrackerID::raw, "The raw value");
  class_<TrigScintID>(
      "TrigScintID",
      "Class that defines the detector ID of the trigger scintillator.",
      init<>(args("self"), "Default constructor"))
      .def(init<RawValue>("Constructor with raw id", args("self", "rawid")))
      .def(init<unsigned int, unsigned int>("Create from pieces",
                                            args("self", "module", "bar")));
}
#endif
