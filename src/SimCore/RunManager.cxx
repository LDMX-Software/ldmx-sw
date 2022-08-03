/**
 * @file RunManager.cxx
 * @brief Class providing a Geant4 run manager implementation.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/RunManager.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimCore/DarkBrem/APrimePhysics.h"
#include "SimCore/DarkBrem/G4eDarkBremsstrahlung.h"  //for process name
#include "SimCore/DetectorConstruction.h"
#include "SimCore/GammaPhysics.h"
#include "SimCore/ParallelWorld.h"
#include "SimCore/PluginFactory.h"
#include "SimCore/PrimaryGeneratorAction.h"

#include "SimCore/G4User/SteppingAction.h"
#include "SimCore/G4User/EventAction.h"
#include "SimCore/G4User/RunAction.h"
#include "SimCore/G4User/StackingAction.h"
#include "SimCore/G4User/TrackingAction.h"

//------------//
//   Geant4   //
//------------//
#include "FTFP_BERT.hh"
#include "G4GDMLParser.hh"
#include "G4GenericBiasingPhysics.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4ProcessTable.hh"
#include "G4VModularPhysicsList.hh"

namespace simcore {

RunManager::RunManager(framework::config::Parameters& parameters,
                       ConditionsInterface& ci)
    : conditionsIntf_(ci) {
  parameters_ = parameters;

  // Set whether the ROOT primary generator should use the persisted seed.
  auto rootPrimaryGenUseSeed{
      parameters.getParameter<bool>("rootPrimaryGenUseSeed")};

  // Validate the geometry if specified.
  setUseRootSeed(rootPrimaryGenUseSeed);
}

RunManager::~RunManager() {}

void RunManager::setupPhysics() {
  auto pList{physicsListFactory_.GetReferencePhysList("FTFP_BERT")};

  parallelWorldPath_ = parameters_.getParameter<std::string>("scoringPlanes");
  isPWEnabled_ = !parallelWorldPath_.empty();
  if (isPWEnabled_) {
    std::cout
        << "[ RunManager ]: Parallel worlds physics list has been registered."
        << std::endl;
    pList->RegisterPhysics(new G4ParallelWorldPhysics("ldmxParallelWorld"));
  }

  pList->RegisterPhysics(new GammaPhysics);
  pList->RegisterPhysics(new darkbrem::APrimePhysics(
      parameters_.getParameter<framework::config::Parameters>("dark_brem")));

  auto biasing_operators{
      parameters_.getParameter<std::vector<framework::config::Parameters>>(
          "biasing_operators", {})};
  if (!biasing_operators.empty()) {
    std::cout << "[ RunManager ]: Biasing enabled with "
              << biasing_operators.size() << " operator(s)." << std::endl;

    // create all the biasing operators that will be used
    for (framework::config::Parameters& bop : biasing_operators) {
      simcore::PluginFactory::getInstance().createBiasingOperator(
          bop.getParameter<std::string>("class_name"),
          bop.getParameter<std::string>("instance_name"), bop);
    }

    // Instantiate the constructor used when biasing
    G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();

    // specify which particles are going to be biased
    //  this will put a biasing interface wrapper around *all* processes
    //  associated with these particles
    for (const simcore::XsecBiasingOperator* bop :
         simcore::PluginFactory::getInstance().getBiasingOperators()) {
      std::cout << "[ RunManager ]: Biasing operator '" << bop->GetName()
                << "' set to bias " << bop->getParticleToBias() << std::endl;
      biasingPhysics->Bias(bop->getParticleToBias());
    }

    // Register the physics constructor to the physics list:
    pList->RegisterPhysics(biasingPhysics);
  }

  this->SetUserInitialization(pList);
}

void RunManager::Initialize() {
  setupPhysics();

  // The parallel world needs to be registered before the mass world is
  // constructed i.e. before G4RunManager::Initialize() is called.
  if (isPWEnabled_) {
    std::cout << "[ RunManager ]: Parallel worlds have been enabled."
              << std::endl;

    auto validateGeometry_{parameters_.getParameter<bool>("validate_detector")};
    G4GDMLParser* pwParser = new G4GDMLParser();
    pwParser->Read(parallelWorldPath_, validateGeometry_);
    this->getDetectorConstruction()->RegisterParallelWorld(
        new ParallelWorld(pwParser, "ldmxParallelWorld"));
  }

  // This is where the physics lists are told to construct their particles and
  // their processes
  //  They are constructed in order, so it is important to register the biasing
  //  physics *after* any other processes that need to be able to be biased
  G4RunManager::Initialize();

  // Instantiate the primary generator action
  auto primaryGeneratorAction{new PrimaryGeneratorAction(parameters_)};
  SetUserAction(primaryGeneratorAction);

  // Get instances of all G4 actions
  //      also create them in the factory
  auto actions{PluginFactory::getInstance().getActions()};

  // Create all user actions
  auto userActions{
      parameters_.getParameter<std::vector<framework::config::Parameters>>(
          "actions", {})};
  for (auto& userAction : userActions) {
    PluginFactory::getInstance().createAction(
        userAction.getParameter<std::string>("class_name"),
        userAction.getParameter<std::string>("instance_name"), userAction);
  }

  // Register all actions with the G4 engine
  for (const auto& [key, act] : actions) {
    std::visit([this](auto&& arg) { this->SetUserAction(arg); }, act);
  }
}

void RunManager::TerminateOneEvent() {
  // have geant4 do its own thing
  G4RunManager::TerminateOneEvent();

  // reset dark brem process (if needed)
  G4ProcessTable* ptable = G4ProcessTable::GetProcessTable();
  G4int verbosity = ptable->GetVerboseLevel();

  // Only one of these processes should be in the table
  //  (i.e. either the Dark Brem is biased or its not)
  // BUT we want to be able to cover both options without
  // the user having to configure it, so we set both
  // of these processes to active (by passing 'true')
  // while the table is silenced. If the table isn't silenced,
  // the process that isn't in the table will cause the table
  // to throw a "not found" warning.
  std::vector<G4String> dark_brem_processes = {
      darkbrem::G4eDarkBremsstrahlung::PROCESS_NAME,
      "biasWrapper(" + darkbrem::G4eDarkBremsstrahlung::PROCESS_NAME + ")"};
  ptable->SetVerboseLevel(
      0);  // silent ptable while searching for process that may/may not exist
  for (auto const& name : dark_brem_processes)
    ptable->SetProcessActivation(name, true);
  ptable->SetVerboseLevel(verbosity);

  if (this->GetVerboseLevel() > 1) {
    std::cout << "[ RunManager ] : "
              << "Reset the dark brem process (if it was activated)."
              << std::endl;
  }
  ptable->SetVerboseLevel(verbosity);
}

DetectorConstruction* RunManager::getDetectorConstruction() {
  return static_cast<DetectorConstruction*>(this->userDetector);
}

}  // namespace simcore
