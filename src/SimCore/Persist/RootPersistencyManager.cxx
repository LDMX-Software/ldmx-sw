
#include "SimCore/Persist/RootPersistencyManager.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>
#include <memory>

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Framework/EventHeader.h"
#include "Framework/RunHeader.h"
#include "Framework/Version.h"
#include "SimCore/Event/SimTrackerHit.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/DetectorConstruction.h"
#include "SimCore/RunManager.h"
#include "SimCore/UserEventInformation.h"
#include "SimCore/UserTrackingAction.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4RunManagerKernel.hh"

namespace simcore {
namespace persist {

RootPersistencyManager::RootPersistencyManager(EventFile &file,
                                               Parameters &parameters,
                                               const int &runNumber)
    : G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(),
                           "RootPersistencyManager"),
      file_(file) {

  // Let Geant4 know what to use this persistency manager
  G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
  G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(
      this, "RootPersistencyManager");

  // Set the parameters, used laster when printing run header
  parameters_ = parameters;

  ecalHitIO_.configure(parameters_);

  run_ = runNumber;
}

G4bool RootPersistencyManager::Store(const G4Event *anEvent) {

  // Check if the event has been aborted.  If so, skip storage of the
  // event.
  if (G4RunManager::GetRunManager()->GetCurrentEvent()->IsAborted())
    return false;

  // Build the output collections.
  buildEvent(anEvent);

  return true;
}

G4bool RootPersistencyManager::Store(const G4Run *) {

  // NOTE: This method is called once the run is terminated through
  // the run manager.

  // Get the detector header from the user detector construction
  auto detector = static_cast<RunManager *>(RunManager::GetRunManager())
                      ->getDetectorConstruction();

  // Create the run header.
  RunHeader runHeader(run_, detector->getDetectorHeader()->getName(),
                      parameters_.getParameter<std::string>("description"));

  // Set parameter value with number of events processed.
  runHeader.setIntParameter("Event Count", eventsCompleted_);
  runHeader.setIntParameter("Events Began", eventsBegan_);

  runHeader.setIntParameter(
      "Save ECal Hit Contribs",
      parameters_.getParameter<bool>("enableHitContribs"));
  runHeader.setIntParameter(
      "Compress ECal Hit Contribs",
      parameters_.getParameter<bool>("compressHitContribs"));
  runHeader.setIntParameter(
      "Included Scoring Planes",
      !parameters_.getParameter<std::string>("scoringPlanes").empty());
  runHeader.setIntParameter(
      "Use Random Seed from Event Header",
      parameters_.getParameter<bool>("rootPrimaryGenUseSeed"));

  // lambda function for dumping 3-vectors into the run header
  auto threeVectorDump = [&runHeader](const std::string &name,
                                      const std::vector<double> &vec) {
    runHeader.setFloatParameter(name + " X", vec.at(0));
    runHeader.setFloatParameter(name + " Y", vec.at(1));
    runHeader.setFloatParameter(name + " Z", vec.at(2));
  };

  auto beamSpotSmear{
      parameters_.getParameter<std::vector<double>>("beamSpotSmear", {})};
  if (!beamSpotSmear.empty())
    threeVectorDump("Smear Beam Spot [mm]", beamSpotSmear);

  // lambda function for dumping vectors of strings to the run header
  auto stringVectorDump = [&runHeader](const std::string &name,
                                       const std::vector<std::string> &vec) {
    int index = 0;
    for (auto const &val : vec) {
      runHeader.setStringParameter(name + " " + std::to_string(++index), val);
    }
  };

  stringVectorDump("Pre Init Command",
                   parameters_.getParameter<std::vector<std::string>>(
                       "preInitCommands", {}));
  stringVectorDump("Post Init Command",
                   parameters_.getParameter<std::vector<std::string>>(
                       "postInitCommands", {}));

  if (parameters_.getParameter<bool>("biasing_enabled")) {
    runHeader.setStringParameter(
        "Biasing Process",
        parameters_.getParameter<std::string>("biasing_process"));
    runHeader.setStringParameter(
        "Biasing Volume",
        parameters_.getParameter<std::string>("biasing_volume"));
    runHeader.setStringParameter(
        "Biasing Particle",
        parameters_.getParameter<std::string>("biasing_particle"));
    runHeader.setIntParameter("Biasing All",
                              parameters_.getParameter<bool>("biasing_all"));
    runHeader.setIntParameter(
        "Biasing Incident", parameters_.getParameter<bool>("biasing_incident"));
    runHeader.setIntParameter(
        "Biasing Disable EM",
        parameters_.getParameter<bool>("biasing_disableEMBiasing"));
    runHeader.setIntParameter("Biasing Factor",
                              parameters_.getParameter<int>("biasing_factor"));
    runHeader.setFloatParameter(
        "Biasing Threshold",
        parameters_.getParameter<double>("biasing_threshold"));
  }

  auto apMass{parameters_.getParameter<double>("APrimeMass")};
  if (apMass > 0) {
    runHeader.setFloatParameter("A' Mass [MeV]", apMass);
    runHeader.setFloatParameter(
        "Dark Brem Global Bias",
        parameters_.getParameter<double>("darkbrem_globalxsecfactor"));
    runHeader.setStringParameter(
        "Dark Brem Vertex Library Path",
        parameters_.getParameter<std::string>("darkbrem_madgraphfilepath"));
    runHeader.setIntParameter("Dark Brem Interpretation Method",
                              parameters_.getParameter<int>("darkbrem_method"));
  }

  auto generators{
      parameters_.getParameter<std::vector<Parameters>>("generators")};
  int counter = 0;
  for (auto const &gen : generators) {

    std::string genID = "Gen " + std::to_string(++counter);
    auto className{gen.getParameter<std::string>("class_name")};
    runHeader.setStringParameter(genID + " Class", className);

    if (className.find("ldmx::ParticleGun") != std::string::npos) {
      runHeader.setFloatParameter(genID + " Time [ns]",
                                  gen.getParameter<double>("time"));
      runHeader.setFloatParameter(genID + " Energy [MeV]",
                                  gen.getParameter<double>("energy"));
      runHeader.setStringParameter(genID + " Particle",
                                   gen.getParameter<std::string>("particle"));
      threeVectorDump(genID + " Position [mm]",
                      gen.getParameter<std::vector<double>>("position"));
      threeVectorDump(genID + " Direction",
                      gen.getParameter<std::vector<double>>("direction"));
    } else if (className.find("ldmx::MultiParticleGunPrimaryGenerator") !=
               std::string::npos) {
      runHeader.setIntParameter(genID + " Poisson Enabled",
                                gen.getParameter<bool>("enablePoisson"));
      runHeader.setIntParameter(genID + " N Particles",
                                gen.getParameter<int>("nParticles"));
      runHeader.setIntParameter(genID + " PDG ID",
                                gen.getParameter<int>("pdgID"));
      threeVectorDump(genID + " Vertex [mm]",
                      gen.getParameter<std::vector<double>>("vertex"));
      threeVectorDump(genID + " Momentum [MeV]",
                      gen.getParameter<std::vector<double>>("momentum"));
    } else if (className.find("ldmx::LHEPrimaryGenerator") !=
               std::string::npos) {
      runHeader.setStringParameter(genID + " LHE File",
                                   gen.getParameter<std::string>("filePath"));
    } else if (className.find("ldmx::RootCompleteReSim") != std::string::npos) {
      runHeader.setStringParameter(genID + " ROOT File",
                                   gen.getParameter<std::string>("filePath"));
    } else if (className.find("ldmx::RootSimFromEcalSP") != std::string::npos) {
      runHeader.setStringParameter(genID + " ROOT File",
                                   gen.getParameter<std::string>("filePath"));
      runHeader.setFloatParameter(genID + " Time Cutoff [ns]",
                                  gen.getParameter<double>("time_cutoff"));
    } else if (className.find("ldmx::GeneralParticleSource") !=
               std::string::npos) {
      stringVectorDump(
          genID + " Init Cmd",
          gen.getParameter<std::vector<std::string>>("initCommands"));
    } else {
      std::cerr << "[ RootPersistencyManager ] [WARN] : "
                << "Unrecognized primary generator '" << className << "'. "
                << "Will not be saving details to RunHeader." << std::endl;
    }
  }

  // Set a string parameter with the Geant4 SHA-1.
  G4String g4Version{
      G4RunManagerKernel::GetRunManagerKernel()->GetVersionString()};
  runHeader.setStringParameter("Geant4 revision", g4Version);

  runHeader.setStringParameter("ldmx-sw revision", GIT_SHA1);

  // debug printout TODO add to logging
  runHeader.Print();

  // Write the header to the file.
  file_.writeRunHeader(runHeader);

  return true;
}

void RootPersistencyManager::Initialize() {}

void RootPersistencyManager::buildEvent(const G4Event *anEvent) {

  // Set basic event information.
  writeHeader(anEvent);

  // Set pointer to current G4Event.
  simParticleBuilder_.setCurrentEvent(anEvent);

  // Build the SimParticle list for the output ROOT event.
  simParticleBuilder_.buildSimParticles(event_);

  // Copy hit objects from SD hit collections into the output event.
  writeHitsCollections(anEvent, event_);
}

void RootPersistencyManager::writeHeader(const G4Event *anEvent) {

  // Retrieve a mutable version of the event header
  EventHeader &eventHeader = event_->getEventHeader();

  // Set the event weight
  double weight{1};
  if (anEvent->GetUserInformation() != nullptr) {
    weight = static_cast<UserEventInformation *>(anEvent->GetUserInformation())
                 ->getWeight();
  } else if (anEvent->GetPrimaryVertex(0)) {
    weight = anEvent->GetPrimaryVertex(0)->GetWeight();
  }
  eventHeader.setWeight(weight);

  // Save the state of the random engine to an output stream. A string
  // is then extracted and saved to the event header.
  std::ostringstream stream;
  G4Random::saveFullState(stream);
  // std::cout << stream.str() << std::endl;
  eventHeader.setStringParameter("eventSeed", stream.str());
}

void RootPersistencyManager::writeHitsCollections(const G4Event *anEvent,
                                                  Event *outputEvent) {

  // Get the HC of this event.
  G4HCofThisEvent *hce = anEvent->GetHCofThisEvent();
  int nColl = hce->GetNumberOfCollections();

  // Loop over all hits collections.
  for (int iColl = 0; iColl < nColl; iColl++) {

    // Get a hits collection and its name.
    G4VHitsCollection *hc = hce->GetHC(iColl);
    if (!hc) {
      EXCEPTION_RAISE("G4HitColl", "G4VHitsCollection indexed " +
                                       std::to_string(iColl) +
                                       " returned a nullptr.");
    }

    std::string collName = hc->GetName();

    if (dynamic_cast<G4TrackerHitsCollection *>(hc) != nullptr) {

      // Write G4TrackerHit collection to output SimTrackerHit collection.
      G4TrackerHitsCollection *trackerHitsColl =
          dynamic_cast<G4TrackerHitsCollection *>(hc);
      std::vector<SimTrackerHit> outputColl;
      writeTrackerHitsCollection(trackerHitsColl, outputColl);

      // Add hits collection to output event.
      outputEvent->add(collName, outputColl);

    } else if (dynamic_cast<G4CalorimeterHitsCollection *>(hc) != nullptr) {

      G4CalorimeterHitsCollection *calHitsColl =
          dynamic_cast<G4CalorimeterHitsCollection *>(hc);
      std::vector<SimCalorimeterHit> outputColl;
      if (collName == "EcalSimHits") {
        // Write ECal G4CalorimeterHit collection to output SimCalorimeterHit
        // collection using helper class.
        ecalHitIO_.writeHitsCollection(calHitsColl, outputColl);
      } else {
        // Write generic G4CalorimeterHit collection to output SimCalorimeterHit
        // collection.
        writeCalorimeterHitsCollection(calHitsColl, outputColl);
      }

      // Add hits collection to output event.
      outputEvent->add(collName, outputColl);
    } // switch on type of hit collection

  } // loop through geant4 hit collections

  return;
}

void RootPersistencyManager::writeTrackerHitsCollection(
    G4TrackerHitsCollection *hc, std::vector<SimTrackerHit> &outputColl) {

  outputColl.clear();
  int nHits = hc->GetSize();
  for (int iHit = 0; iHit < nHits; iHit++) {
    G4TrackerHit *g4hit = (G4TrackerHit *)hc->GetHit(iHit);
    const G4ThreeVector &momentum = g4hit->getMomentum();
    const G4ThreeVector &position = g4hit->getPosition();

    SimTrackerHit simTrackerHit;
    simTrackerHit.setID(g4hit->getID());
    simTrackerHit.setTime(g4hit->getTime());
    simTrackerHit.setLayerID(g4hit->getLayerID());
    simTrackerHit.setModuleID(g4hit->getModuleID());
    simTrackerHit.setEdep(g4hit->getEdep());
    simTrackerHit.setEnergy(g4hit->getEnergy());
    simTrackerHit.setPathLength(g4hit->getPathLength());
    simTrackerHit.setTrackID(g4hit->getTrackID());
    simTrackerHit.setPdgID(g4hit->getPdgID());
    simTrackerHit.setPosition(position.x(), position.y(), position.z());
    simTrackerHit.setMomentum(momentum.x(), momentum.y(), momentum.z());

    outputColl.push_back(simTrackerHit);
  }

  return;
}

void RootPersistencyManager::writeCalorimeterHitsCollection(
    G4CalorimeterHitsCollection *hc,
    std::vector<SimCalorimeterHit> &outputColl) {

  // get ancestral tracking information
  auto trackMap{UserTrackingAction::getUserTrackingAction()->getTrackMap()};

  int nHits = hc->GetSize();
  for (int iHit = 0; iHit < nHits; iHit++) {
    G4CalorimeterHit *g4hit = (G4CalorimeterHit *)hc->GetHit(iHit);
    const G4ThreeVector &pos = g4hit->getPosition();

    SimCalorimeterHit simHit;
    simHit.setID(g4hit->getID());
    simHit.addContrib(trackMap->findIncident(g4hit->getTrackID()),
                      g4hit->getTrackID(), g4hit->getPdgCode(),
                      g4hit->getEdep(), g4hit->getTime());
    simHit.setPosition(pos.x(), pos.y(), pos.z());

    outputColl.push_back(simHit);
  }

  return;
}

} // namespace persist
} // namespace simcore
