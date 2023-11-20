/**
 * @file RunManager.cxx
 * @brief Class providing a Geant4 run manager implementation.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimCore/RunManager.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "G4DarkBreM/G4DarkBremsstrahlung.h"  //for process name
#include "SimCore/APrimePhysics.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/G4User/EventAction.h"
#include "SimCore/G4User/PrimaryGeneratorAction.h"
#include "SimCore/G4User/RunAction.h"
#include "SimCore/G4User/StackingAction.h"
#include "SimCore/G4User/SteppingAction.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/GammaPhysics.h"
#include "SimCore/ParallelWorld.h"
#include "SimCore/XsecBiasingOperator.h"

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
                       ConditionsInterface&) {
  parameters_ = parameters;

  // Set whether the ROOT primary generator should use the persisted seed.
  auto rootPrimaryGenUseSeed{
      parameters.getParameter<bool>("rootPrimaryGenUseSeed")};

  // Validate the geometry if specified.
  setUseRootSeed(rootPrimaryGenUseSeed);
}

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

  pList->RegisterPhysics(new GammaPhysics{"GammaPhysics", parameters_});
  pList->RegisterPhysics(new APrimePhysics(
      parameters_.getParameter<framework::config::Parameters>("dark_brem")));
  pList->RegisterPhysics(new KaonPhysics(
      "KaonPhysics", parameters_.getParameter<framework::config::Parameters>(
                         "kaon_parameters")));

  auto biasing_operators{
      parameters_.getParameter<std::vector<framework::config::Parameters>>(
          "biasing_operators", {})};
  if (!biasing_operators.empty()) {
    std::cout << "[ RunManager ]: Biasing enabled with "
              << biasing_operators.size() << " operator(s)." << std::endl;

    // create all the biasing operators that will be used
    for (framework::config::Parameters& bop : biasing_operators) {
      simcore::XsecBiasingOperator::Factory::get().make(
          bop.getParameter<std::string>("class_name"),
          bop.getParameter<std::string>("instance_name"), bop);
    }

    // Instantiate the constructor used when biasing
    G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();

    // specify which particles are going to be biased
    //  this will put a biasing interface wrapper around *all* processes
    //  associated with these particles
    simcore::XsecBiasingOperator::Factory::get().apply([biasingPhysics](
                                                           auto bop) {
      std::cout << "[ RunManager ]: Biasing operator '" << bop->GetName()
                << "' set to bias " << bop->getParticleToBias() << std::endl;
      biasingPhysics->Bias(bop->getParticleToBias());
    });

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

  // create our G4User actions
  auto primary_action{new g4user::PrimaryGeneratorAction(parameters_)};
  auto run_action{new g4user::RunAction};
  auto event_action{new g4user::EventAction};
  auto tracking_action{new g4user::TrackingAction};
  auto stepping_action{new g4user::SteppingAction};
  auto stacking_action{new g4user::StackingAction};
  // ...and register them with G4
  SetUserAction(primary_action);
  SetUserAction(run_action);
  SetUserAction(event_action);
  SetUserAction(tracking_action);
  SetUserAction(stepping_action);
  SetUserAction(stacking_action);

  // Create all user actions and attch them to the corresponding G4 actions
  auto user_actions{
      parameters_.getParameter<std::vector<framework::config::Parameters>>(
          "actions", {})};
  for (auto& user_action : user_actions) {
    auto ua = UserAction::Factory::get().make(
        user_action.getParameter<std::string>("class_name"),
        user_action.getParameter<std::string>("instance_name"), user_action);
    for (auto& type : ua->getTypes()) {
      if (type == simcore::TYPE::RUN) {
        run_action->registerAction(ua.get());
      } else if (type == simcore::TYPE::EVENT) {
        event_action->registerAction(ua.get());
      } else if (type == simcore::TYPE::TRACKING) {
        tracking_action->registerAction(ua.get());
      } else if (type == simcore::TYPE::STEPPING) {
        stepping_action->registerAction(ua.get());
      } else if (type == simcore::TYPE::STACKING) {
        stacking_action->registerAction(ua.get());
      } else {
        EXCEPTION_RAISE("ActionType", "Action type does not exist.");
      }
    }
  }
}

void RunManager::TerminateOneEvent() {
  // have geant4 do its own thing
  G4RunManager::TerminateOneEvent();

  // go through the processes attached to the electron and
  // reactivate any process that contains the G4DarkBremmstrahlung name
  // this covers both cases where the process is biased and not
  static auto reactivate_dark_brem = [](G4ProcessManager* pman) {
    for (int i_proc{0}; i_proc < pman->GetProcessList()->size(); i_proc++) {
      G4VProcess* p{(*(pman->GetProcessList()))[i_proc]};
      if (p->GetProcessName().contains(G4DarkBremsstrahlung::PROCESS_NAME)) {
        pman->SetProcessActivation(p, true);
        break;
      }
    }
  };

  reactivate_dark_brem(G4Electron::Definition()->GetProcessManager());

  if (this->GetVerboseLevel() > 1) {
    std::cout << "[ RunManager ] : "
              << "Reset the dark brem process (if it was activated)."
              << std::endl;
  }
}

DetectorConstruction* RunManager::getDetectorConstruction() {
  return static_cast<DetectorConstruction*>(this->userDetector);
}

}  // namespace simcore
