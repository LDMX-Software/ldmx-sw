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
/*   GENIE     */
/*~~~~~~~~~~~~~*/
#include "GENIE/Framework/GHEP/GHepParticle.h"

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
#include "SimCore/Event/GTruth.h"
#include "SimCore/Event/GHepParticle.h"

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

void Simulator::FillGHepParticles(const genie::EventRecord* record,
				  std::vector<ldmx::GHepParticle>& ghep_particle_list)
{
  for(int position=0; position< record->GetEntries(); ++position)
    {
      const genie::GHepParticle* particle = record->Particle(position);

      ldmx::GHepParticle my_particle;

      my_particle.fPosition = position;
      
      my_particle.fPdgCode = particle->Pdg();
      my_particle.fStatus = particle->Status();
      my_particle.fRescatterCode = particle->RescatterCode();
      my_particle.fFirstMother = particle->FirstMother();
      my_particle.fLastMother = particle->LastMother();
      my_particle.fFirstDaugher = particle->FirstDaughter();
      my_particle.fLastDaughter = particle->LastDaughter();
      
      my_particle.fP_x = particle->P4()->Px();
      my_particle.fP_y = particle->P4()->Py();
      my_particle.fP_z = particle->P4()->Pz();
      my_particle.fP_t = particle->P4()->E();
      
      my_particle.fX_x = particle->X4()->X();
      my_particle.fX_y = particle->X4()->Y();
      my_particle.fX_z = particle->X4()->Z();
      my_particle.fX_t = particle->X4()->T();

      my_particle.fPolzTheta = particle->PolzPolarAngle();
      my_particle.fPolzPhi = particle->PolzAzimuthAngle();

      my_particle.fRemovalEnergy = particle->RemovalEnergy();
      my_particle.fIsBound = particle->IsBound();

      ghep_particle_list.push_back(my_particle);
    }
}
  
void Simulator::FillGTruth(const genie::EventRecord* record,
			   ldmx::GTruth& truth) {

  //interactions info
  genie::Interaction *inter = record->Summary();
  const genie::ProcessInfo  &procInfo = inter->ProcInfo();
  truth.fGint = (int)procInfo.InteractionTypeId();
  truth.fGscatter = (int)procInfo.ScatteringTypeId();
  
  //Event info
  truth.fweight = record->Weight();
  truth.fprobability = record->Probability();
  truth.fXsec = record->XSec();
  truth.fDiffXsec = record->DiffXSec();
  truth.fGPhaseSpace = (int)record->DiffXSecVars();

  //Initial State info
  const genie::InitialState &initState  = inter->InitState();
  truth.fProbePDG = initState.ProbePdg();
  truth.fProbe_px = initState.GetProbeP4()->Px();
  truth.fProbe_py = initState.GetProbeP4()->Py();
  truth.fProbe_pz = initState.GetProbeP4()->Pz();
  truth.fProbe_e = initState.GetProbeP4()->E();

  truth.fTgt_px = initState.GetTgtP4()->Px();
  truth.fTgt_py = initState.GetTgtP4()->Py();
  truth.fTgt_pz = initState.GetTgtP4()->Pz();
  truth.fTgt_e = initState.GetTgtP4()->E();

  //Target info
  const genie::Target &tgt = initState.Tgt();
  truth.ftgtZ = tgt.Z();
  truth.ftgtA = tgt.A();
  truth.ftgtPDG = tgt.Pdg();
  truth.fHitNucPDG = tgt.HitNucPdg();
  truth.fHitQrkPDG = tgt.HitQrkPdg();
  truth.fIsSeaQuark = tgt.HitSeaQrk();
  truth.fHitNuc_px = tgt.HitNucP4Ptr()->Px();
  truth.fHitNuc_py = tgt.HitNucP4Ptr()->Py();
  truth.fHitNuc_pz = tgt.HitNucP4Ptr()->Pz();
  truth.fHitNuc_e = tgt.HitNucP4Ptr()->E();
  truth.fHitNucPos = tgt.HitNucPosition();

  
  truth.fVertex_x = record->Vertex()->X();
  truth.fVertex_y = record->Vertex()->Y();
  truth.fVertex_z = record->Vertex()->Z();
  truth.fVertex_t = record->Vertex()->T();

  //true reaction information and byproducts
  //(PRE FSI)
  const genie::XclsTag &exclTag = inter->ExclTag();
  truth.fIsCharm          = exclTag.IsCharmEvent();
  truth.fCharmHadronPdg   = exclTag.CharmHadronPdg();
  truth.fIsStrange        = exclTag.IsStrangeEvent();
  truth.fStrangeHadronPdg = exclTag.StrangeHadronPdg();
  truth.fResNum           = (int)exclTag.Resonance();
  truth.fDecayMode        = exclTag.DecayMode();

  truth.fNumPiPlus = exclTag.NPiPlus();
  truth.fNumPiMinus = exclTag.NPiMinus();
  truth.fNumPi0 = exclTag.NPi0();
  truth.fNumProton = exclTag.NProtons();
  truth.fNumNeutron = exclTag.NNeutrons();
  truth.fNumSingleGammas = exclTag.NSingleGammas();

  truth.fNumSingleGammas = exclTag.NSingleGammas();
  truth.fNumRho0         = exclTag.NRho0();
  truth.fNumRhoPlus      = exclTag.NRhoPlus();
  truth.fNumRhoMinus     = exclTag.NRhoMinus();

  truth.fIsFinalQuarkEvent = exclTag.IsFinalQuarkEvent();
  truth.fFinalQuarkPdg  = exclTag.FinalQuarkPdg();
  truth.fIsFinalLeptonEvent = exclTag.IsFinalLeptonEvent();
  truth.fFinalLeptonPdg = exclTag.FinalLeptonPdg();

  // Get the GENIE kinematics info
  const genie::Kinematics &kine = inter->Kine();
  for(int kvar=genie::KineVar_t::kKVNull; kvar!=genie::KineVar_t::kNumOfKineVar; ++kvar)
    truth.fKV[kvar] = kine.GetKV((genie::KineVar_t)kvar);

  truth.fFSlepton_px = kine.FSLeptonP4().Px();
  truth.fFSlepton_py = kine.FSLeptonP4().Py();
  truth.fFSlepton_pz = kine.FSLeptonP4().Pz();
  truth.fFSlepton_e = kine.FSLeptonP4().E();

  truth.fFShadSyst_px = kine.HadSystP4().Px();
  truth.fFShadSyst_py = kine.HadSystP4().Py();
  truth.fFShadSyst_pz = kine.HadSystP4().Pz();
  truth.fFShadSyst_e = kine.HadSystP4().E();

  return;

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
    SensitiveDetector::Factory::get().apply(
        [](auto sd) { sd->OnFinishedEvent(); });
    this->abortEvent();  // get out of processors loop
  }

  // Terminate the event.  This checks if an event is to be stored or
  // stacked for later.
  numEventsCompleted_++;

  // store event-wide information in EventHeader
  auto& event_header = event.getEventHeader();
  updateEventHeader(event_header);

  event_header.setStringParameter("eventSeed", stream.str());

  /*
  PrimaryGenerator::Factory::get().apply([](auto gen){
    std::cout << gen->Name() << std::endl;
  });
  */
  
  auto event_info = static_cast<UserEventInformation*>(
      runManager_->GetCurrentEvent()->GetUserInformation());
  if(event_info->getGENIEEventRecord()){
    //event_info->getGENIEEventRecord()->Print();

    ldmx::GTruth gtruth;
    std::vector<ldmx::GHepParticle> ghep_particles;

    FillGTruth(event_info->getGENIEEventRecord(),gtruth);
    FillGHepParticles(event_info->getGENIEEventRecord(),ghep_particles);

    event.add("SimGTruth", gtruth);
    event.add("SimGHepParticles", ghep_particles);
  }
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
