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
#include "Framework/EventFile.h"
#include "Framework/Process.h"
#include "Framework/RandomNumberSeedService.h"
#include "Framework/Version.h"  //for LDMX_INSTALL path

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/APrimePhysics.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/G4Session.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/Geo/ParserFactory.h"
#include "SimCore/PrimaryGenerator.h"
#include "SimCore/SensitiveDetector.h"
#include "SimCore/UserEventInformation.h"
#include "SimCore/XsecBiasingOperator.h"

/*~~~~~~~~~~~~~~*/
/*    Geant4    */
/*~~~~~~~~~~~~~~*/
#include "G4BiasingProcessInterface.hh"
#include "G4CascadeParameters.hh"
#include "G4Electron.hh"
#include "G4GDMLParser.hh"
#include "G4GeometryManager.hh"
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "Randomize.hh"

namespace simcore {

Simulator::Simulator(const std::string& name, framework::Process& process)
    : simcore::SimulatorBase(name, process) {}

void Simulator::configure(framework::config::Parameters& parameters) {
  SimulatorBase::configure(parameters);
}

void Simulator::beforeNewRun(ldmx::RunHeader& header) {
  // Get the detector header from the user detector construction
  DetectorConstruction* detector =
      dynamic_cast<RunManager*>(RunManager::GetRunManager())
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
  if (!beamSpotSmear.empty()) {
    threeVectorDump("Smear Beam Spot [mm]", beamSpotSmear);
  }

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

  simcore::XsecBiasingOperator::Factory::get().apply(
      [&header](auto bop) { bop->RecordConfig(header); });

  int counter = 0;
  PrimaryGenerator::Factory::get().apply([&header, &counter](auto gen) {
    std::string gen_id = "Gen" + std::to_string(counter++);
    gen->RecordConfig(gen_id, header);
  });

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

void Simulator::onNewRun(const ldmx::RunHeader& runHeader) {
  const framework::RandomNumberSeedService& rseed =
      getCondition<framework::RandomNumberSeedService>(
          framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  std::vector<int> seeds;
  seeds.push_back(rseed.getSeed("Simulator[0]"));
  seeds.push_back(rseed.getSeed("Simulator[1]"));
  setSeeds(seeds);

  run_ = runHeader.getRunNumber();
}

void Simulator::produce(framework::Event& event) {
  // Generate and process a Geant4 event.
  numEventsBegan_++;
  // Save the state of the random engine to an output stream. A string
  // is then extracted and saved to the event header.
  std::ostringstream stream;
  G4Random::saveFullState(stream);
  runManager_->ProcessOneEvent(event.getEventHeader().getEventNumber());

  // If a Geant4 event has been aborted, skip the rest of the processing
  // sequence. This will immediately force the simulation to move on to
  // the next event.
  if (runManager_->GetCurrentEvent()->IsAborted()) {
    runManager_->TerminateOneEvent();  // clean up event objects
    SensitiveDetector::Factory::get().apply([](auto sd) { sd->EndOfEvent(); });
    this->abortEvent();  // get out of processors loop
  }

  // Terminate the event.  This checks if an event is to be stored or
  // stacked for later.
  numEventsCompleted_++;

  // store event-wide information in EventHeader
  auto& event_header = event.getEventHeader();
  updateEventHeader(event_header);

  event_header.setStringParameter("eventSeed", stream.str());

  saveTracks(event);

  saveSDHits(event);

  runManager_->TerminateOneEvent();

  return;
}

void Simulator::onFileClose(framework::EventFile& file) {
  // Pass the **real** number of events to the persistency manager
  auto rh = file.getRunHeader(run_);
  rh.setIntParameter("Event Count", numEventsCompleted_);
  rh.setIntParameter("Events Began", numEventsBegan_);
}

void Simulator::onProcessEnd() {
  SimulatorBase::onProcessEnd();
  std::cout << "[ Simulator ] : "
            << "Started " << numEventsBegan_ << " events to produce "
            << numEventsCompleted_ << " events." << std::endl;
}

void Simulator::setSeeds(std::vector<int> seeds) {
  // If no seeds have been specified then return immediately.
  if (seeds.empty()) {
    return;
  }

  // If seeds are specified, make sure that the container has at least
  // two seeds.  If not, throw an exception.
  if (seeds.size() == 1) {
    EXCEPTION_RAISE("ConfigurationException",
                    "At least two seeds need to be specified.");
  }

  // Create the array of seeds and pass them to G4Random.  Currently,
  // only 100 seeds can be specified at a time.  If less than 100
  // seeds are specified, the remaining slots are set to 0.

  constexpr int max_number_of_seeds{100};
  std::vector<long> seedVec(max_number_of_seeds, 0);
  for (std::size_t index{0}; index < seeds.size(); ++index) {
    seedVec[index] = static_cast<long>(seeds[index]);
  }

  // Pass the array of seeds to the random engine.
  G4Random::setTheSeeds(seedVec.data());
}

}  // namespace simcore

DECLARE_PRODUCER_NS(simcore, Simulator)
