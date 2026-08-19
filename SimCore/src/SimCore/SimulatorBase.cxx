#include "SimCore/SimulatorBase.h"

#include "SimCore/G4User/PhotonuclearTracker.h"
#include "SimCore/Generators/PrimaryGenerator.h"

namespace simcore {

const std::vector<std::string> SimulatorBase::INVALID_COMMANDS = {
    "/run/initialize",        // hard coded at the right time
    "/run/beamOn",            // passed commands should only be sim setup
    "/random/setSeeds",       // handled by own config parameter (if passed)
    "ldmx",                   // all ldmx messengers have been removed
    "/persistency/gdml/read"  // detector description is read after passed a
                              // path to the detector description (required)
};
SimulatorBase::SimulatorBase(const std::string& name,
                             framework::Process& process)
    : framework::Producer(name, process), conditions_intf_(this) {
  ui_manager_ = G4UImanager::GetUIpointer();
}
void SimulatorBase::prepEvent(framework::Event& event) {
  /// right now, we just make sure the PrimaryVertices are all prepared
  PrimaryGenerator::Factory::get().apply([&event](auto gen) {
      gen->PrepEvent(event);
  });
}
void SimulatorBase::updateEventHeader(ldmx::EventHeader& eventHeader) const {
  auto event_info = static_cast<UserEventInformation*>(
      run_manager_->GetCurrentEvent()->GetUserInformation());

  eventHeader.setWeight(event_info->getWeight());
  eventHeader.setFloatParameter("total_photonuclear_energy",
                                event_info->getPNEnergy());
  eventHeader.setFloatParameter("total_electronuclear_energy",
                                event_info->getENEnergy());
  eventHeader.setFloatParameter("db_material_z",
                                event_info->getDarkBremMaterialZ());
  eventHeader.setFloatParameter("aprime_conversion_material_z",
                                event_info->getAPrimeConversionMaterialZ());
  eventHeader.setIntParameter("pn_target_z", event_info->getPNTargetZ());
  eventHeader.setIntParameter("pn_target_a", event_info->getPNTargetA());
  eventHeader.setIntParameter("pn_resample_count",
                              event_info->getPNResampleCount());
}
void SimulatorBase::onProcessEnd() {
  run_manager_->TerminateEventLoop();
  run_manager_->RunTermination();
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
  //  2. When Simulator is deleted because run_manager_ is a unique_ptr
  run_manager_.reset(nullptr);

  // Delete the G4UIsession
  // I don't think this needs to happen here, but since we are cleaning up
  // loose ends...
  session_handle_.reset(nullptr);
};
void SimulatorBase::onProcessStart() {
  // initialize run
  run_manager_->Initialize();

  for (const std::string& cmd : post_init_commands_) {
    int g4_ret = ui_manager_->ApplyCommand(cmd);
    if (g4_ret > 0) {
      EXCEPTION_RAISE("PostInitCmd",
                      "Post Initialization command '" + cmd +
                          "' returned a failue status from Geant4: " +
                          std::to_string(g4_ret));
    }
  }

  // Instantiate the scoring worlds including any parallel worlds.
  run_manager_->ConstructScoringWorlds();

  // Initialize the current run
  run_manager_->RunInitialization();

  // Initialize the event processing
  run_manager_->InitializeEventLoop(1);

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
  for (const auto& invalid_command : INVALID_COMMANDS) {
    for (const auto& cmd : pre_init_commands_) {
      if (cmd.find(invalid_command) != std::string::npos) {
        EXCEPTION_RAISE("PreInitCmd", "Pre Initialization command '" + cmd +
                                          "' is not allowed because another "
                                          "part of Simulator handles it.");
      }
    }
    for (const auto& cmd : post_init_commands_) {
      if (cmd.find(invalid_command) != std::string::npos) {
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

  pre_init_commands_ =
      parameters_.get<std::vector<std::string>>("pre_init_commands", {});

  // Get the extra simulation configuring commands
  post_init_commands_ =
      parameters_.get<std::vector<std::string>>("post_init_commands", {});

  verifyParameters();
  if (run_manager_) {
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
  run_manager_ = std::make_unique<RunManager>(parameters_, conditions_intf_);
  // Instantiate the class so cascade parameters can be set.
  // TODO: Are we actually using this?
  G4CascadeParameters::Instance();

  buildGeometry();
  for (const std::string& cmd : pre_init_commands_) {
    int g4_ret = ui_manager_->ApplyCommand(cmd);
    if (g4_ret > 0) {
      EXCEPTION_RAISE("PreInitCmd",
                      "Pre Initialization command '" + cmd +
                          "' returned a failure status from Geant4: " +
                          std::to_string(g4_ret));
    }
  }
}
void SimulatorBase::createLogging() {
  auto logging_prefix = parameters_.get<std::string>("logging_prefix");
  session_handle_ = std::make_unique<LoggedSession>(logging_prefix);

  if (session_handle_ != nullptr)
    ui_manager_->SetCoutDestination(session_handle_.get());
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
    sd->onFinishedEvent();
  });
}

void SimulatorBase::savePhotonuclearInteractions(framework::Event& event) {
  // Save photonuclear interactions if the PhotonuclearTracker is active
  auto pn_tracker = PhotonuclearTracker::get();
  if (pn_tracker) {
    auto pn_interactions = pn_tracker->getInteractions();
    if (!pn_interactions.empty()) {
      event.add("PhotonuclearInteractions", pn_interactions);
    }
  }
}

void SimulatorBase::buildGeometry() {
  // Instantiate the GDML parser and corresponding messenger owned and
  // managed by DetectorConstruction
  auto parser{simcore::geo::Parser::Factory::get().make("gdml", parameters_,
                                                        conditions_intf_)};
  if (not parser) {
    EXCEPTION_RAISE(
        "UnableToCreate",
        "Unable to find a parser registered under the name 'gdml'.");
  }
  auto parser_ptr{parser.value()};

  // Set the DetectorConstruction instance used to build the detector
  // from the GDML description.
  run_manager_->SetUserInitialization(
      new DetectorConstruction(parser_ptr, parameters_, conditions_intf_));

  // Parse the detector geometry and validate if specified.
  auto detector_path{parameters_.get<std::string>("detector")};
  ldmx_log(trace) << "Reading in geometry from '" << detector_path << "'";
  G4GeometryManager::GetInstance()->OpenGeometry();
  parser_ptr->read();
  run_manager_->DefineWorldVolume(parser_ptr->getWorldVolume());
}
}  // namespace simcore
