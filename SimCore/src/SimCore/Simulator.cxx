/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/Simulator.h"

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
  header.setDescription(parameters_.get<std::string>("description"));
  header.setIntParameter(
      "Included Scoring Planes",
      !parameters_.get<std::string>("scoring_planes").empty());
  header.setIntParameter("Use Random Seed from Event Header",
                         parameters_.get<bool>("root_primary_gen_use_seed"));

  // lambda function for dumping 3-vectors into the run header
  auto three_vector_dump = [&header](const std::string& name,
                                     const std::vector<double>& vec) {
    header.setFloatParameter(name + " X", vec.at(0));
    header.setFloatParameter(name + " Y", vec.at(1));
    header.setFloatParameter(name + " Z", vec.at(2));
  };

  auto beam_spot_smear{
      parameters_.get<std::vector<double>>("beamSpotSmear", {})};
  if (!beam_spot_smear.empty()) {
    three_vector_dump("Smear Beam Spot [mm]", beam_spot_smear);
  }

  // lambda function for dumping vectors of strings to the run header
  auto string_vector_dump = [&header](const std::string& name,
                                      const std::vector<std::string>& vec) {
    int index = 0;
    for (auto const& val : vec) {
      header.setStringParameter(name + " " + std::to_string(++index), val);
    }
  };

  string_vector_dump(
      "Pre Init Command",
      parameters_.get<std::vector<std::string>>("pre_init_commands", {}));
  string_vector_dump(
      "Post Init Command",
      parameters_.get<std::vector<std::string>>("post_init_commands", {}));

  simcore::XsecBiasingOperator::Factory::get().apply(
      [&header](auto bop) { bop->RecordConfig(header); });

  int counter = 0;
  PrimaryGenerator::Factory::get().apply([&header, &counter](auto gen) {
    std::string gen_id = "Gen" + std::to_string(counter++);
    gen->RecordConfig(gen_id, header);
  });

  // Set a string parameter with the Geant4 SHA-1.
  if (G4RunManagerKernel::GetRunManagerKernel()) {
    G4String g4_version{
        G4RunManagerKernel::GetRunManagerKernel()->GetVersionString()};
    header.setStringParameter("Geant4 revision", g4_version);
  } else {
    ldmx_log(warn) << "Unable to access G4 RunManager Kernel. Will not store "
                      "G4 Version string.";
  }

  header.setStringParameter("SIM version", LDMXSW_VERSION);
  header.setStringParameter("SIM revision", GIT_SHA1);
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
  num_events_began_++;
  // Save the state of the random engine to an output stream. A string
  // is then extracted and saved to the event header.
  std::ostringstream stream;
  G4Random::saveFullState(stream);

  SimulatorBase::prepEvent(event);
  run_manager_->ProcessOneEvent(event.getEventHeader().getEventNumber());

  // If a Geant4 event has been aborted, skip the rest of the processing
  // sequence. This will immediately force the simulation to move on to
  // the next event.
  if (run_manager_->GetCurrentEvent()->IsAborted()) {
    run_manager_->TerminateOneEvent();  // clean up event objects
    SensitiveDetector::Factory::get().apply(
        [](auto sd) { sd->onFinishedEvent(); });
    this->abortEvent();  // get out of processors loop
  }

  // Terminate the event.  This checks if an event is to be stored or
  // stacked for later.
  num_events_completed_++;

  // store event-wide information in EventHeader
  auto& event_header = event.getEventHeader();
  updateEventHeader(event_header);

  event_header.setStringParameter("eventSeed", stream.str());

  auto event_info = static_cast<UserEventInformation*>(
      run_manager_->GetCurrentEvent()->GetUserInformation());

  auto hepmc3_events = event_info->getHepMC3GenEvents();
  for (auto& hepmc3ev : hepmc3_events) {
    hepmc3ev.event_number = event.getEventHeader().getEventNumber();
  }
  if (hepmc3_events.size() > 0) event.add("SimHepMC3Events", hepmc3_events);

  saveTracks(event);

  saveSDHits(event);

  savePhotonuclearInteractions(event);

  // Extract and save Bertini cascade histories if any were recorded
  auto& history_store = bertini::CascadeHistoryStore::getInstance();
  ldmx_log(debug) << "Checking cascade history store: "
                  << (history_store.empty() ? "empty" : "has histories");
  if (!history_store.empty()) {
    auto cascade_histories = history_store.extractHistories();
    ldmx_log(info) << "Saving " << cascade_histories.size()
                   << " cascade histories to event";
    event.add("PhotonuclearCascadeHistories", cascade_histories);
  }

  run_manager_->TerminateOneEvent();

  return;
}

void Simulator::onProcessEnd() {
  SimulatorBase::onProcessEnd();
  // Put this to warn level, just so it's printed out for sure
  ldmx_log(warn) << "Started " << num_events_began_ << " events to produce "
                 << num_events_completed_ << " events.";
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
  std::vector<long> seed_vec(max_number_of_seeds, 0);
  for (std::size_t index{0}; index < seeds.size(); ++index) {
    seed_vec[index] = static_cast<long>(seeds[index]);
  }

  // Pass the array of seeds to the random engine.
  G4Random::setTheSeeds(seed_vec.data());
}

}  // namespace simcore

DECLARE_PRODUCER(simcore::Simulator)
