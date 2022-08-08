/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/Simulator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Process.h"
#include "Framework/RandomNumberSeedService.h"
#include "Framework/Version.h"  //for LDMX_INSTALL path
#include "Framework/EventFile.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/DarkBrem/G4eDarkBremsstrahlung.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/G4Session.h"
#include "SimCore/XsecBiasingOperator.h"
#include "SimCore/PrimaryGenerator.h"
#include "SimCore/SensitiveDetector.h"
#include "SimCore/Geo/ParserFactory.h"
#include "SimCore/UserEventInformation.h"
#include "SimCore/G4User/TrackingAction.h"

/*~~~~~~~~~~~~~~*/
/*    Geant4    */
/*~~~~~~~~~~~~~~*/
#include "G4UIsession.hh"
#include "G4UImanager.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4Electron.hh"
#include "G4CascadeParameters.hh"
#include "G4GDMLParser.hh"
#include "G4GeometryManager.hh"
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "Randomize.hh"

namespace simcore {

const std::vector<std::string> Simulator::invalidCommands_ = {
    "/run/initialize",        // hard coded at the right time
    "/run/beamOn",            // passed commands should only be sim setup
    "/random/setSeeds",       // handled by own config parameter (if passed)
    "ldmx",                   // all ldmx messengers have been removed
    "/persistency/gdml/read"  // detector description is read after passed a
                              // path to the detector description (required)
};

Simulator::Simulator(const std::string& name, framework::Process& process)
    : framework::Producer(name, process), conditionsIntf_(this) {
  // Get the ui manager from geant
  //      This pointer is handled by Geant4
  uiManager_ = G4UImanager::GetUIpointer();
}

Simulator::~Simulator() {}

void Simulator::configure(framework::config::Parameters& parameters) {
  // parameters used to configure the simulation
  parameters_ = parameters;

  // Set the verbosity level.  The default level  is 0.
  verbosity_ = parameters_.getParameter<int>("verbosity");

  // If the verbosity level is set to 0,
  // If the verbosity level is > 1, log everything to a file. Otherwise,
  // dump the output. If a prefix has been specified, append it ot the
  // log message.
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

  // Instantiate the run manager.
  runManager_ = std::make_unique<RunManager>(parameters_, conditionsIntf_);

  // Instantiate the GDML parser and corresponding messenger owned and
  // managed by DetectorConstruction
  auto parser{simcore::geo::ParserFactory::getInstance().createParser(
      "gdml", parameters, conditionsIntf_)};

  // Instantiate the class so cascade parameters can be set.
  G4CascadeParameters::Instance();

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

  auto preInitCommands =
      parameters_.getParameter<std::vector<std::string>>("preInitCommands", {});
  for (const std::string& cmd : preInitCommands) {
    if (allowed(cmd)) {
      int g4Ret = uiManager_->ApplyCommand(cmd);
      if (g4Ret > 0) {
        EXCEPTION_RAISE("PreInitCmd",
                        "Pre Initialization command '" + cmd +
                            "' returned a failue status from Geant4: " +
                            std::to_string(g4Ret));
      }
    } else {
      EXCEPTION_RAISE(
          "PreInitCmd",
          "Pre Initialization command '" + cmd +
              "' is not allowed because another part of Simulator handles it.");
    }
  }
}

void Simulator::beforeNewRun(ldmx::RunHeader& header) {
  // Get the detector header from the user detector construction
  DetectorConstruction* detector =
      static_cast<RunManager*>(RunManager::GetRunManager())
          ->getDetectorConstruction();

  header.setDetectorName(detector->getDetectorName());
  header.setDescription(parameters_.getParameter<std::string>("description"));
  header.setIntParameter(
      "Included Scoring Planes",
      !parameters_.getParameter<std::string>("scoringPlanes").empty());
  header.setIntParameter(
      "Use Random Seed from Event Header",
      parameters_.getParameter<bool>("rootPrimaryGenUseSeed"));

  // lambda function for dumping 3-vectors into the run header
  auto threeVectorDump = [&header](const std::string& name,
                                   const std::vector<double>& vec) {
    header.setFloatParameter(name + " X", vec.at(0));
    header.setFloatParameter(name + " Y", vec.at(1));
    header.setFloatParameter(name + " Z", vec.at(2));
  };

  auto beamSpotSmear{
      parameters_.getParameter<std::vector<double>>("beamSpotSmear", {})};
  if (!beamSpotSmear.empty())
    threeVectorDump("Smear Beam Spot [mm]", beamSpotSmear);

  // lambda function for dumping vectors of strings to the run header
  auto stringVectorDump = [&header](const std::string& name,
                                    const std::vector<std::string>& vec) {
    int index = 0;
    for (auto const& val : vec) {
      header.setStringParameter(name + " " + std::to_string(++index), val);
    }
  };

  stringVectorDump("Pre Init Command",
                   parameters_.getParameter<std::vector<std::string>>(
                       "preInitCommands", {}));
  stringVectorDump("Post Init Command",
                   parameters_.getParameter<std::vector<std::string>>(
                       "postInitCommands", {}));

  simcore::XsecBiasingOperator::Factory::get().apply([&header](auto bop) {
      bop->RecordConfig(header);
      });

  auto dark_brem{
    parameters_.getParameter<framework::config::Parameters>("dark_brem")};
  if (dark_brem.getParameter<bool>("enable")) {
    // the dark brem process is enabled, find it and then record its
    // configuration
    G4ProcessVector* electron_processes =
      G4Electron::Electron()->GetProcessManager()->GetProcessList();
    int n_electron_processes = electron_processes->size();
    for (int i_process = 0; i_process < n_electron_processes; i_process++) {
      G4VProcess* process = (*electron_processes)[i_process];
      if (process->GetProcessName().contains(
            darkbrem::G4eDarkBremsstrahlung::PROCESS_NAME)) {
        // reset process to wrapped process if it is biased
        if (dynamic_cast<G4BiasingProcessInterface*>(process))
          process = dynamic_cast<G4BiasingProcessInterface*>(process)
            ->GetWrappedProcess();
        // record the process configuration to the run header
        dynamic_cast<darkbrem::G4eDarkBremsstrahlung*>(process)->RecordConfig(
            header);
        break;
      }  // this process is the dark brem process
    }    // loop through electron processes
  }      // dark brem has been enabled

  // TODO check that this is working properly
  int counter = 0;
  PrimaryGenerator::Factory::get().apply([&header,&counter](auto gen) {
        std::string gen_id = "Gen"+std::to_string(counter++);
        gen->RecordConfig(gen_id, header);
      });

  /**
  SensitiveDetector::Factory::get().apply([&header](auto sd) {
    sd->RecordConfig(header);
  });
   */

  // Set a string parameter with the Geant4 SHA-1.
  if (G4RunManagerKernel::GetRunManagerKernel()) {
    G4String g4Version{
        G4RunManagerKernel::GetRunManagerKernel()->GetVersionString()};
    header.setStringParameter("Geant4 revision", g4Version);
  } else {
    ldmx_log(warn) << "Unable to access G4 RunManager Kernel. Will not store "
                      "G4 Version string.";
  }

  header.setStringParameter("ldmx-sw revision", GIT_SHA1);
}

void Simulator::onNewRun(const ldmx::RunHeader& rh) {
  const framework::RandomNumberSeedService& rseed = getCondition<framework::RandomNumberSeedService>(
      framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  std::vector<int> seeds;
  seeds.push_back(rseed.getSeed("Simulator[0]"));
  seeds.push_back(rseed.getSeed("Simulator[1]"));
  setSeeds(seeds);

  run_ = rh.getRunNumber();
}

void Simulator::produce(framework::Event& event) {
  // Generate and process a Geant4 event.
  numEventsBegan_++;
  runManager_->ProcessOneEvent(event.getEventHeader().getEventNumber());

  // If a Geant4 event has been aborted, skip the rest of the processing
  // sequence. This will immediately force the simulation to move on to
  // the next event.
  if (runManager_->GetCurrentEvent()->IsAborted()) {
    runManager_->TerminateOneEvent();  // clean up event objects
    SensitiveDetector::Factory::get().apply([&event](auto sd) {
        sd->EndOfEvent();
    });
    this->abortEvent();                // get out of processors loop
  }

  // Terminate the event.  This checks if an event is to be stored or
  // stacked for later.
  numEventsCompleted_++;

  // store event-wide information in EventHeader
  auto event_info = static_cast<UserEventInformation *>(
      runManager_->GetCurrentEvent()
        ->GetUserInformation());
  auto& event_header = event.getEventHeader();
  event_header.setWeight(event_info->getWeight());
  event_header.setFloatParameter("total_photonuclear_energy",
                                event_info->getPNEnergy());
  event_header.setFloatParameter("total_electronuclear_energy",
                                event_info->getENEnergy());

  // Save the state of the random engine to an output stream. A string
  // is then extracted and saved to the event header.
  std::ostringstream stream;
  G4Random::saveFullState(stream);
  // std::cout << stream.str() << std::endl;
  event_header.setStringParameter("eventSeed", stream.str());

  // track storage
  TrackMap& tracks{g4user::TrackingAction::get()->getTrackMap()};
  tracks.traceAncestry();
  event.add("SimParticles", tracks.getParticleMap());

  // Copy hit objects from SD hit collections into the output event.
  SensitiveDetector::Factory::get().apply([&event](auto sd) {
        sd->saveHits(event);
        sd->EndOfEvent();
      });

  runManager_->TerminateOneEvent();

  return;
}

void Simulator::onProcessStart() {
  // initialize run
  runManager_->Initialize();

  // Get the extra simulation configuring commands
  auto postInitCommands = parameters_.getParameter<std::vector<std::string>>(
      "postInitCommands", {});
  for (const std::string& cmd : postInitCommands) {
    if (allowed(cmd)) {
      int g4Ret = uiManager_->ApplyCommand(cmd);
      if (g4Ret > 0) {
        EXCEPTION_RAISE("PostInitCmd",
                        "Post Initialization command '" + cmd +
                            "' returned a failue status from Geant4: " +
                            std::to_string(g4Ret));
      }
    } else {
      EXCEPTION_RAISE(
          "PostInitCmd",
          "Post Initialization command '" + cmd +
              "' is not allowed because another part of Simulator handles it.");
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

void Simulator::onFileClose(framework::EventFile& file) {
  // End the current run and print out some basic statistics if verbose
  // level > 0.
  runManager_->TerminateEventLoop();

  // Pass the **real** number of events to the persistency manager
  auto rh = file.getRunHeader(run_);
  rh.setIntParameter("Event Count", numEventsCompleted_);
  rh.setIntParameter("Events Began", numEventsBegan_);

  // Persist any remaining events, call the end of run action and
  // terminate the Geant4 kernel.
  runManager_->RunTermination();
}

void Simulator::onProcessEnd() {
  std::cout << "[ Simulator ] : "
            << "Started " << numEventsBegan_ << " events to produce "
            << numEventsCompleted_ << " events." << std::endl;

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
  // I don't think this needs to happen here, but since we are cleaning up loose
  // ends...
  sessionHandle_.reset(nullptr);
}

bool Simulator::allowed(const std::string& command) const {
  for (const std::string& invalidSubstring : invalidCommands_) {
    if (command.find(invalidSubstring) != std::string::npos) {
      // found invalid substring in this command ==> NOT ALLOWED
      return false;
    }
  }
  // checked all invalid commands ==> ALLOWED
  return true;
}

void Simulator::setSeeds(std::vector<int> seeds) {
  // If no seeds have been specified then return immediately.
  if (seeds.empty()) return;

  // If seeds are specified, make sure that the container has at least
  // two seeds.  If not, throw an exception.
  if (seeds.size() == 1) {
    EXCEPTION_RAISE("ConfigurationException",
                    "At least two seeds need to be specified.");
  }

  // Create the array of seeds and pass them to G4Random.  Currently,
  // only 100 seeds can be specified at a time.  If less than 100
  // seeds are specified, the remaining slots are set to 0.
  std::vector<long> seedVec(100, 0);
  for (std::size_t index{0}; index < seeds.size(); ++index)
    seedVec[index] = static_cast<long>(seeds[index]);

  // Pass the array of seeds to the random engine.
  G4Random::setTheSeeds(&seedVec[0]);
}

}  // namespace simcore

DECLARE_PRODUCER_NS(simcore, Simulator)
