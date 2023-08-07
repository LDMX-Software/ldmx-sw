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
SimulatorBase::SimulatorBase(const std::string& name,
                             framework::Process& process)
    : framework::Producer(name, process), conditionsIntf_(this) {
  uiManager_ = G4UImanager::GetUIpointer();
}
void SimulatorBase::updateEventHeader(ldmx::EventHeader& eventHeader) const {
  auto event_info = static_cast<UserEventInformation*>(
      runManager_->GetCurrentEvent()->GetUserInformation());

  eventHeader.setWeight(event_info->getWeight());
  eventHeader.setFloatParameter("total_photonuclear_energy",
                                event_info->getPNEnergy());
  eventHeader.setFloatParameter("total_electronuclear_energy",
                                event_info->getENEnergy());
}
void SimulatorBase::onProcessEnd() {
  runManager_->TerminateEventLoop();
  runManager_->RunTermination();
  // Delete Run Manager
  // From Geant4 Basic Example B01:
  //      Job termination
  //      Free the store: user actions, physics list and detector descriptions
  //      are owned and deleted by the run manager, so they should not be
  //      deleted in the main() program
  // This needs to happen here because otherwise, Geant4 objects are deleted
  // twice:
  //  1. When the histogram file is closed (all ROOT objects created during
  //  processing are put there because ROOT)
  //  2. When Simulator is deleted because runManager_ is a unique_ptr
  runManager_.reset(nullptr);

  // Delete the G4UIsession
  // I don't think this needs to happen here, but since we are cleaning up
  // loose ends...
  sessionHandle_.reset(nullptr);
};
void SimulatorBase::onProcessStart() {
  // initialize run
  runManager_->Initialize();

  for (const std::string& cmd : postInitCommands_) {
    int g4Ret = uiManager_->ApplyCommand(cmd);
    if (g4Ret > 0) {
      EXCEPTION_RAISE("PostInitCmd",
                      "Post Initialization command '" + cmd +
                          "' returned a failue status from Geant4: " +
                          std::to_string(g4Ret));
    }
  }

  // Instantiate the scoring worlds including any parallel worlds.
  runManager_->ConstructScoringWorlds();

  // Initialize the current run
  runManager_->RunInitialization();

  // Initialize the event processing
  runManager_->InitializeEventLoop(1);

  return;
}
void SimulatorBase::verifyParameters() const {
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

void SimulatorBase::createLogging() {
  auto loggingPrefix = parameters_.getParameter<std::string>("logging_prefix");
  if (verbosity_ == 0)
    sessionHandle_ = std::make_unique<BatchSession>();
  else if (verbosity_ > 1) {
    if (loggingPrefix.empty())
      sessionHandle_ = std::make_unique<LoggedSession>();
    else
      sessionHandle_ = std::make_unique<LoggedSession>(
          loggingPrefix + "_G4cout.log", loggingPrefix + "_G4cerr.log");
  }
  if (sessionHandle_ != nullptr)
    uiManager_->SetCoutDestination(sessionHandle_.get());
}

void SimulatorBase::saveTracks(framework::Event& event) {
  TrackMap& tracks{g4user::TrackingAction::get()->getTrackMap()};
  tracks.traceAncestry();
  event.add("SimParticles", tracks.getParticleMap());
}
void SimulatorBase::saveSDHits(framework::Event& event) {
  // Copy hit objects from SD hit collections into the output event.
  SensitiveDetector::Factory::get().apply([&event](auto sd) {
    sd->saveHits(event);
    sd->EndOfEvent();
  });
}

void SimulatorBase::buildGeometry() {
  // Instantiate the GDML parser and corresponding messenger owned and
  // managed by DetectorConstruction
  auto parser{simcore::geo::ParserFactory::getInstance().createParser(
      "gdml", parameters_, conditionsIntf_)};

  // Set the DetectorConstruction instance used to build the detector
  // from the GDML description.
  runManager_->SetUserInitialization(
      new DetectorConstruction(parser, parameters_, conditionsIntf_));

  // Parse the detector geometry and validate if specified.
  auto detectorPath{parameters_.getParameter<std::string>("detector")};
  if (verbosity_ > 0) {
    std::cout << "[ Simulator ] : Reading in geometry from '" << detectorPath
              << "'... " << std::flush;
  }
  G4GeometryManager::GetInstance()->OpenGeometry();
  parser->read();
  runManager_->DefineWorldVolume(parser->GetWorldVolume());
}
}  // namespace simcore
