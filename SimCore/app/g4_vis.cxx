#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "G4GDMLParser.hh"
#include "G4GeometryManager.hh"
#include "G4PhysListFactory.hh"
#include "G4RunManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/Geo/Parser.h"

static void printUsage() {
  std::cout << "usage: g4-vis {detector.gdml} (macro.mac)" << std::endl;
  std::cout << "  {detector.gdml} is the geometry description "
               "that you wish to visualize."
            << std::endl
            << "  (macro.mac) is an optional argument to execute"
               " a macro file immediately after initialization."
            << std::endl
            << "  The macro file must be in the current directory"
               " or in a subdirectory of the current directory."
            << std::endl
            << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 2 && argc != 3) {
    printUsage();
    std::cerr << "** Need to be given a single detector description and "
                 ")optionally) a single macro to execute. **"
              << std::endl;
    return 1;
  }

  std::string the_arg{argv[1]};
  std::string the_macro;
  if (argc == 3) the_macro = std::string(argv[2]);
  if (the_arg == "-h" or the_arg == "--help") {
    // ask for help, let's give it to them.
    printUsage();
    return 0;
  }

  framework::EventProcessor* null_processor{nullptr};
  simcore::ConditionsInterface empty_interface(null_processor);
  framework::config::Parameters parser_parameters;
  parser_parameters.addParameter("validate_detector", true);
  parser_parameters.addParameter<std::string>("detector", the_arg);

  // RunManager
  G4RunManager* run_manager = new G4RunManager;

  // Detector components
  auto parser{simcore::geo::Parser::Factory::get().make(
      "gdml", parser_parameters, empty_interface)};
  if (not parser) {
    std::cerr << "Unable to create a 'gdml' parser to read the geometry."
              << std::endl;
    return 1;
  }
  auto parser_ptr{parser.value()};
  run_manager->SetUserInitialization(new simcore::DetectorConstruction(
      parser_ptr, parser_parameters, empty_interface));
  G4GeometryManager::GetInstance()->OpenGeometry();
  parser_ptr->read();
  run_manager->DefineWorldVolume(parser_ptr->getWorldVolume());

  // required to define a physics list to complete initialization
  G4PhysListFactory lists;
  run_manager->SetUserInitialization(lists.GetReferencePhysList("FTFP_BERT"));

  run_manager->Initialize();

  // Define (G)UI
  G4UIExecutive* ui = new G4UIExecutive(argc, argv);
  G4VisManager* vis_manager = new G4VisExecutive;
  vis_manager->Initialize();
  if (argc == 3) {
    auto* uimanager = G4UImanager::GetUIpointer();
    G4String command = "/control/execute " + the_macro;
    uimanager->ApplyCommand(command);
  }

  ui->SessionStart();

  delete ui;
  delete run_manager;
  delete vis_manager;

  return 0;
}
