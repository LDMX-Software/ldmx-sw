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
#include "SimCore/BiasOperators/XsecBiasingOperator.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/FCPPhysics.h"
#include "SimCore/G4User/EventAction.h"
#include "SimCore/G4User/RunAction.h"
#include "SimCore/G4User/StackingAction.h"
#include "SimCore/G4User/SteppingAction.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/GammaPhysics.h"
#include "SimCore/GenieElectroNuclearProcess.h"  //for process name
#include "SimCore/GenieNuclearPhysics.h"
#include "SimCore/ParallelWorld.h"
#include "SimCore/PrimaryGeneratorAction.h"

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
  auto root_primary_gen_use_seed{
      parameters.get<bool>("root_primary_gen_use_seed")};

  // Validate the geometry if specified.
  setUseRootSeed(root_primary_gen_use_seed);
}

void RunManager::setupPhysics() {
  auto p_list{physics_list_factory_.GetReferencePhysList("FTFP_BERT")};
  p_list->SetVerboseLevel(0);

  parallel_world_path_ = parameters_.get<std::string>("scoring_planes");
  is_pw_enabled_ = !parallel_world_path_.empty();
  if (is_pw_enabled_) {
    ldmx_log(debug) << "Parallel worlds physics list has been registered";
    p_list->RegisterPhysics(new G4ParallelWorldPhysics("ldmxParallelWorld"));
  }

  p_list->RegisterPhysics(new GammaPhysics{"GammaPhysics", parameters_});
  p_list->RegisterPhysics(new APrimePhysics(
      parameters_.get<framework::config::Parameters>("dark_brem")));
  p_list->RegisterPhysics(new KaonPhysics(
      "KaonPhysics",
      parameters_.get<framework::config::Parameters>("kaon_parameters")));
  p_list->RegisterPhysics(new FCPPhysics(
      "FCPPhysics",
      parameters_.get<framework::config::Parameters>("fcp_physics")));
  p_list->RegisterPhysics(new GenieNuclearPhysics(
      parameters_.get<framework::config::Parameters>("genie_nuclear")));

  auto biasing_operators{
      parameters_.get<std::vector<framework::config::Parameters>>(
          "biasing_operators", {})};
  if (!biasing_operators.empty()) {
    ldmx_log(info) << " Biasing enabled with " << biasing_operators.size()
                   << " operator(s)";

    // create all the biasing operators that will be used
    for (framework::config::Parameters& bop : biasing_operators) {
      if (not simcore::XsecBiasingOperator::Factory::get().make(
              bop.get<std::string>("class_name"),
              bop.get<std::string>("instance_name"), bop)) {
        EXCEPTION_RAISE("UnableToCreate",
                        "Unable to create a XsecBiasingOperator of type " +
                            bop.get<std::string>("class_name"));
      }
    }

    // Instantiate the constructor used when biasing
    G4GenericBiasingPhysics* biasing_physics = new G4GenericBiasingPhysics();

    // specify which particles are going to be biased
    //  this will put a biasing interface wrapper around *all* processes
    //  associated with these particles
    simcore::XsecBiasingOperator::Factory::get().apply(
        [this, biasing_physics](auto bop) {
          ldmx_log(info) << "Biasing operator '" << bop->GetName()
                         << "' set to bias " << bop->getParticleToBias();
          biasing_physics->Bias(bop->getParticleToBias());
        });

    // Register the physics constructor to the physics list:
    p_list->RegisterPhysics(biasing_physics);
  }

  this->SetUserInitialization(p_list);
}

void RunManager::Initialize() {
  setupPhysics();

  // The parallel world needs to be registered before the mass world is
  // constructed i.e. before G4RunManager::Initialize() is called.
  if (is_pw_enabled_) {
    ldmx_log(debug) << "Parallel worlds have been enabled";

    auto validate_geometry{parameters_.get<bool>("validate_detector")};
    G4GDMLParser* pw_parser = new G4GDMLParser();
    pw_parser->Read(parallel_world_path_, validate_geometry);
    this->getDetectorConstruction()->RegisterParallelWorld(
        new ParallelWorld(pw_parser, "ldmxParallelWorld"));
  }

  // This is where the physics lists are told to construct their particles and
  // their processes
  //  They are constructed in order, so it is important to register the biasing
  //  physics *after* any other processes that need to be able to be biased
  G4RunManager::Initialize();

  // create our G4User actions
  auto primary_action{new PrimaryGeneratorAction(parameters_)};
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
  auto user_actions{parameters_.get<std::vector<framework::config::Parameters>>(
      "actions", {})};
  for (auto& user_action : user_actions) {
    auto ua = UserAction::Factory::get().make(
        user_action.get<std::string>("class_name"),
        user_action.get<std::string>("instance_name"), user_action);
    if (not ua) {
      EXCEPTION_RAISE(
          "UnableToCreate",
          "Unable to create a UserAction of type " +
              user_action.get<std::string>("class_name") +
              ". Did you inherit from simcore::UserAction? "
              "Do you have DECLARE_ACTION in your implementation (.cxx) file? "
              "Did you include the fully-specified class name in your python "
              "configuration class? "
              "Did you specify the correct library in the python configuration "
              "class?");
    }
    for (auto& type : ua.value()->getTypes()) {
      if (type == simcore::TYPE::RUN) {
        run_action->registerAction(ua.value());
      } else if (type == simcore::TYPE::EVENT) {
        event_action->registerAction(ua.value());
      } else if (type == simcore::TYPE::TRACKING) {
        tracking_action->registerAction(ua.value());
      } else if (type == simcore::TYPE::STEPPING) {
        stepping_action->registerAction(ua.value());
      } else if (type == simcore::TYPE::STACKING) {
        stacking_action->registerAction(ua.value());
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

  // Reactivate GENIE electronNuclear process if it was deactivated
  // (only_one_per_event mode) — same pattern as DarkBrem above
  static auto reactivate_genie_en = [](G4ProcessManager* pman) {
    for (int i_proc{0}; i_proc < pman->GetProcessList()->size(); i_proc++) {
      G4VProcess* p{(*(pman->GetProcessList()))[i_proc]};
      if (p->GetProcessName().contains(
              GenieElectroNuclearProcess::PROCESS_NAME)) {
        pman->SetProcessActivation(p, true);
        break;
      }
    }
  };

  reactivate_genie_en(G4Electron::Definition()->GetProcessManager());
}

DetectorConstruction* RunManager::getDetectorConstruction() {
  return static_cast<DetectorConstruction*>(this->userDetector);
}

}  // namespace simcore
