#include "DetDescr/DetectorIDBindings.h"
#if DETECTORID_BINDINGS_ENABLED

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

  // def(...) defines functions and .def() defines member functions.
  //
  // To add names names to parameters, add an args() call as a parameter
  //
  // To add a docstring, pass a string literal as the last parameter to the
  // def/.def function call

  // args(...) give names to parameters. This both adds the names to the
  // generated documentation and allows using python's named parameters
  //
  // .staticmethod("name") is required for any static member functions. Note
  // that things will compile without it but the function won't be callable from
  // python

  using namespace boost::python;
  using namespace ldmx;
  using RawValue = DetectorID::RawValue;
  class_<DetectorID>("DetectorID", init<>());
  class_<HcalAbstractID>("HcalAbstractID", init<>());
  class_<EcalAbstractID>("EcalAbstractID", init<>());
  class_<EcalID>("EcalID",
                 "Extension of DetectorID providing access to ECal layers and "
                 "cell numbers in a hex grid",
                 init<>("Empty ECAL id (but not null!)"))
      .def(init<RawValue>(args("rawid"), "Create from raw number"))
      .def(init<unsigned int, unsigned int, unsigned int>(
          args("layer", "module", "cell"), "Create from pieces"))
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>(
          args("layer", "module", "u", "v"),
          "Create from pieces including u/v cell"))
      .def(init<unsigned int, unsigned int,
                std::pair<unsigned int, unsigned int>>(
          args("layer", "module", "uv"),
          "Create from pieces including u/v cell"))
      .def("module", &EcalID::module,
           "Get the value of the module field from the ID.")
      .def("layer", &EcalID::layer,
           "Get the value of the layer field from the ID.")
      .def("cell", &EcalID::cell,
           "Get the value of the cell field from the ID.")
      //  Requires defining a translator for pair
      // .def("getCellUV", &EcalID::getCellUV, "Get the cell u,v index assuming
      // a CMS-standard 432-cell sensor" )
      .def("raw", &EcalID::raw, "The raw value");
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
      init<>("Empty EcalElectronics id (but not null!)"))
      .def(init<RawValue>(
          "Create from raw number\n\n Importantly, this is NOT the PackedIndex "
          "value, it is the entire raw value including the subsystem ID.",
          args("rawid")))
      .def(init<unsigned int, unsigned int, unsigned int>(
          args("fiber", "elink", "channel"), "Create from pieces"))
      .def("fiber", &EcalElectronicsID::fiber,
           "Get the value of the fiber from the ID.")
      .def("elink", &EcalElectronicsID::elink,
           "Get the value of the elink from the ID.")
      .def("channel", &EcalElectronicsID::channel,
           "Get the value of the channel from the ID.")
      .def("index", &EcalElectronicsID::index, "Get the compact index value")
      .def("idFromIndex", &EcalElectronicsID::idFromIndex, args("index"),
           "Construct an electronics id from an index")
      .def("raw", &EcalElectronicsID::raw, "The raw value")
      .staticmethod("idFromIndex");
  class_<EcalTriggerID>("EcalTriggerID",
                        "Extension of DetectorID providing access to ECal "
                        "trigger cell information",
                        init<>("Empty EcALTrigger id (but not null!)"))
      .def(init<RawValue>(args("rawid"), "Create from raw number"))
      .def(init<unsigned int, unsigned int, unsigned int>(
          args("layer", "module", "cell"), "Create from pieces"))
      .def("module", &EcalTriggerID::module,
           "Get the value of the module field from the ID.")
      .def("layer", &EcalTriggerID::layer,
           "Get the value of the layer field from the ID.")
      .def("triggercell", &EcalTriggerID::triggercell,
           "Get the value of the trigger cell field from the ID.")
      .def("raw", &EcalTriggerID::raw, "The raw value");
  //  Currently not actually defined
  // .def("getCellUV", &EcalTriggerID::getCellUV);
  class_<HcalID>("HcalID", "Implements detector ids for Hcal subdetector",
                 init<>())
      .def(init<RawValue>("Empty HCAL id (but not null!)"))
      .def(init<unsigned int, unsigned int, unsigned int>(
          "Create from pieces", args("section, layer, strip")))
      .def("section", &HcalID::section,
           "Get the value of the 'section' field from the ID.")
      .def("layer", &HcalID::layer,
           "Get the value of the layer field from the ID.")
      .def("strip", &HcalID::strip,
           "Get the value of the 'strip' (bar) field from the ID.")
      .def("raw", &HcalID::raw, "The raw value");
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
      init<>("Empty HCAL id (but not null!)"))
      .def(init<RawValue>("Create from raw number", args("rawid")))
      .def(init<unsigned int, unsigned int, unsigned int>(
          "Create from pieces", args("fiber", "elink", "channel")))
      .def("fiber", &HcalElectronicsID::fiber,
           "Get the value of the fiber from the ID.")
      .def("elink", &HcalElectronicsID::elink,
           "Get the value of the elink from the ID.")
      .def("channel", &HcalElectronicsID::channel,
           "Get the value of the channel from the ID.")
      .def("index", &HcalElectronicsID::index, "Get the compact index value")
      .def("idFromIndex", &HcalElectronicsID::idFromIndex, args("index"),
           "Create an electronics ID from an index")
      .def("raw", &HcalElectronicsID::raw, "The raw value")
      .staticmethod("idFromIndex");
  class_<HcalDigiID>("HcalDigiID",
                     "Extension of HcalAbstractID providing access to HCal "
                     "digi information",
                     init<>())
      .def(init<RawValue>("Create from raw number", args("rawid")))
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>(
          "Create from pieces", args("section", "layer", "strip", "end")))
      .def("section", &HcalDigiID::section,
           "Get the value of the 'section' field from the ID.")
      .def("layer", &HcalDigiID::layer,
           "Get the value of the 'layer' field from the ID.")
      .def("strip", &HcalDigiID::strip,
           "Get the value of the 'strip' (bar) field from the ID.")
      .def("end", &HcalDigiID::end,
           "Get the value of the 'end' (positive/negative) field from the ID.")
      .def("isNegativeEnd", &HcalDigiID::isNegativeEnd,
           "Get whether the 'end' field from the ID is negative.")
      .def("raw", &HcalElectronicsID::raw, "The raw value");
  class_<HcalTriggerID>(
      "HcalTriggerID",
      "Extension of DetectorID providing access to HCal trigger cell",
      init<>("Empty HCAL trigger id (but not null!)"))
      .def(init<RawValue>("Create from raw number", args("rawid")))
      .def(init<unsigned int, unsigned int, unsigned int, unsigned int>(
          "Create from pieces", args("section", "layer", "superstrip", "end")))
      .def("section", &HcalTriggerID::section,
           "Get the value of the 'section' field from the ID.")
      .def("layer", &HcalTriggerID::layer,
           "Get the value of the 'layer' field from the ID.")
      .def("superstrip", &HcalTriggerID::superstrip,
           "Get the value of the 'superstrip' field from the ID.")
      .def("end", &HcalTriggerID::end,
           "Get the value of the 'end' field from the ID.")
      .def("isNegativeEnd", &HcalTriggerID::isNegativeEnd,
           "Get whether the 'end' field from the ID is negative.")
      .def("isComposite", &HcalTriggerID::isComposite,
           "Get whether the ID is the composite of two bar ends.")
      .def("raw", &HcalTriggerID::raw, "The raw value");
  class_<SimSpecialID>("SimSpecialID",
                       "Implements detector ids for special simulation-derived "
                       "hits like scoring planes",
                       init<>("Empty id (but not null!)"))
      .def(init<RawValue>("Create from raw number", args("rawid")));
}
#endif
