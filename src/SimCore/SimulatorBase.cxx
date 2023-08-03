#include "SimCore/SimulatorBase.h"

namespace simcore {

const std::vector<std::string> SimulatorBase::invalidCommands_ = {
    "/run/initialize",        // hard coded at the right time
    "/run/beamOn",            // passed commands should only be sim setup
    "/random/setSeeds",       // handled by own config parameter (if passed)
    "ldmx",                   // all ldmx messengers have been removed
    "/persistency/gdml/read"  // detector description is read after passed a
                              // path to the detector description (required)
};
void SimulatorBase::verifyParameters() const {
  std::cout << "Verifying parameters!" << std::endl;
  // in past versions of SimCore, the run number for the simulation was
  // passed directly to the simulator class rather than pulled from central
  // framework. This is here to prevent the user from accidentally using the
  // old style.
  if (parameters_.exists("runNumber")) {
    EXCEPTION_RAISE("InvalidParam",
                    "Remove old-style of setting the simulation run number "
                    "(sim.runNumber)."
                    " Replace with using the Process object (p.run).");
  }
  // Looks for sub-strings matching the ones listed as an invalid command.
  // These invalid commands are mostly commands where control has been handed
  // over to Simulator.
  for (const auto& invalidCommand : invalidCommands_) {
    for (const auto& cmd : preInitCommands_) {
      if (cmd.find(invalidCommand) != std::string::npos) {
        EXCEPTION_RAISE("PreInitCmd", "Pre Initialization command '" + cmd +
                                          "' is not allowed because another "
                                          "part of Simulator handles it.");
      }
    }
    for (const auto& cmd : postInitCommands_) {
      if (cmd.find(invalidCommand) != std::string::npos) {
        EXCEPTION_RAISE("PostInitCmd", "Post Initialization command '" + cmd +
                                           "' is not allowed because another "
                                           "part of Simulator handles it.");
      }
    }
  }
}

void SimulatorBase::configure(framework::config::Parameters& parameters) {
  std::cout << "SimCoreBase::configure" << std::endl;
  // parameters used to configure the simulation
  parameters_ = parameters;
  // Set the verbosity level.  The default level  is 0.
  verbosity_ = parameters_.getParameter<int>("verbosity");

  preInitCommands_ =
      parameters_.getParameter<std::vector<std::string>>("preInitCommands", {});

  // Get the extra simulation configuring commands
  postInitCommands_ = parameters_.getParameter<std::vector<std::string>>(
      "postInitCommands", {});

  verifyParameters();
  if (runManager_) {
    // TODO: This won't work, need to think of a better solution
    EXCEPTION_RAISE(
        "MultipleSimulators",
        "A simulator or resimulator producer has already been created. Only "
        "one of them can be present in a given run. To run the resimulator, "
        "use a an existing eventFile as input.");
  }
  // Set up logging before creating the run manager so that output from the
  // creation of the runManager goes to the appropriate place.
  createLogging();
  runManager_ = std::make_unique<RunManager>(parameters_, conditionsIntf_);
  // Instantiate the class so cascade parameters can be set.
  // TODO: Are we actually using this?
  G4CascadeParameters::Instance();

  buildGeometry();
  for (const std::string& cmd : preInitCommands_) {
    int g4Ret = uiManager_->ApplyCommand(cmd);
    if (g4Ret > 0) {
      EXCEPTION_RAISE("PreInitCmd",
                      "Pre Initialization command '" + cmd +
                          "' returned a failure status from Geant4: " +
                          std::to_string(g4Ret));
    }
  }
}
}  // namespace simcore
