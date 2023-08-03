#ifndef SIMCORE_SIMULATOR_BASE_H_
#define SIMCORE_SIMULATOR_BASE_H_

#include <G4CascadeParameters.hh>
#include <G4GeometryManager.hh>
#include <G4UImanager.hh>
#include <G4UIsession.hh>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventDef.h"
#include "Framework/EventFile.h"
#include "Framework/EventHeader.h"
#include "Framework/EventProcessor.h"
#include "SimCore/ConditionsInterface.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/G4Session.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/Geo/ParserFactory.h"
#include "SimCore/RunManager.h"
#include "SimCore/SensitiveDetector.h"
#include "SimCore/UserEventInformation.h"
namespace simcore {
class SimulatorBase : public framework::Producer {
 public:
  SimulatorBase(const std::string& name, framework::Process& process)
      : framework::Producer(name, process), conditionsIntf_(this) {
    uiManager_ = G4UImanager::GetUIpointer();
  }
  virtual ~SimulatorBase() = default;
  void configure(framework::config::Parameters& parameters) override;

 protected:
  virtual void updateEventHeader(ldmx::EventHeader& eventHeader) const {
    auto event_info = static_cast<UserEventInformation*>(
        runManager_->GetCurrentEvent()->GetUserInformation());

    eventHeader.setWeight(event_info->getWeight());
    eventHeader.setFloatParameter("total_photonuclear_energy",
                                  event_info->getPNEnergy());
    eventHeader.setFloatParameter("total_electronuclear_energy",
                                  event_info->getENEnergy());
  }
  /// Callback called once processing is complete.
  void onProcessEnd() override {
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
  /**
   * Initialization of simulation
   *
   * This uses the parameters set in the configure method to
   * construct and initialize the simulation objects.
   *
   * This function runs the post init setup commands.
   */
  void onProcessStart() override {
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
  ///  Conditions interface
  ConditionsInterface conditionsIntf_;

  /// User interface handle
  G4UImanager* uiManager_{nullptr};

  /// Manager controlling G4 simulation run
  std::unique_ptr<RunManager> runManager_;

  /// Handle to the G4Session -> how to deal with G4cout and G4cerr
  std::unique_ptr<G4UIsession> sessionHandle_;

  /// Commands not allowed to be passed from python config file
  ///     This is because Simulator already runs them.
  static const std::vector<std::string> invalidCommands_;

  /*********************************************************
   * Python Configuration Parameters
   *********************************************************/

  /// Vebosity for the simulation
  int verbosity_{1};
  /// The parameters used to configure the simulation
  framework::config::Parameters parameters_;

  std::vector<std::string> preInitCommands_;

  std::vector<std::string> postInitCommands_;

 private:
  void createLogging() {
    // If the verbosity level is set to 0,
    // If the verbosity level is > 1, log everything to a file. Otherwise,
    // dump the output. If a prefix has been specified, append it ot the
    // log message.
    auto loggingPrefix =
        parameters_.getParameter<std::string>("logging_prefix");
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

 protected:
  virtual void saveTracks(framework::Event& event) {
    TrackMap& tracks{g4user::TrackingAction::get()->getTrackMap()};
    tracks.traceAncestry();
    event.add("SimParticles", tracks.getParticleMap());
  }
  virtual void saveSDHits(framework::Event& event) {
    // Copy hit objects from SD hit collections into the output event.
    SensitiveDetector::Factory::get().apply([&event](auto sd) {
      sd->saveHits(event);
      sd->EndOfEvent();
    });
  }
  virtual void produce(framework::Event& event) = 0;

 private:
  void buildGeometry() {
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
    auto validateGeometry{parameters_.getParameter<bool>("validate_detector")};
    if (verbosity_ > 0) {
      std::cout << "[ Simulator ] : Reading in geometry from '" << detectorPath
                << "'... " << std::flush;
    }
    G4GeometryManager::GetInstance()->OpenGeometry();
    parser->read();
    runManager_->DefineWorldVolume(parser->GetWorldVolume());
  }

  void verifyParameters() const;
};
}  // namespace simcore

#endif /* SIMCORE_SIMULATOR_BASE_H_ */
