#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4RunManager.hh"
#include "G4GDMLParser.hh"
#include "G4GeometryManager.hh"
#include "G4PhysListFactory.hh"

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/DetectorConstruction.h"

static void printUsage() {
  std::cout << "usage: g4-vis {detector.gdml}" << std::endl;
  std::cout << "  {detector.gdml} is the geometry description "
               "that you wish to visualize." << std::endl;
}

int main(int argc, char* argv[]) {

  if (argc != 2) {
    printUsage();
    std::cerr << "** Need to be given the detector description. **" << std::endl;
  }

  framework::EventProcessor* null_processor{nullptr};
  framework::config::Parameters empty_parameters;
  simcore::ConditionsInterface empty_interface(null_processor);

  // RunManager 
  G4RunManager *runManager = new G4RunManager;

  // Detector components
  auto parser = new G4GDMLParser;
  runManager->SetUserInitialization(
      new simcore::DetectorConstruction(parser, empty_parameters, empty_interface)
      );
  G4GeometryManager::GetInstance()->OpenGeometry();
  parser->Read(argv[1],false);
  runManager->DefineWorldVolume(parser->GetWorldVolume());

  // required to define a physics list to complete initialization
  G4PhysListFactory lists;
  runManager->SetUserInitialization(lists.GetReferencePhysList("FTFP_BERT"));

  runManager->Initialize();

  // Define (G)UI
  G4UIExecutive* ui = new G4UIExecutive(argc, argv);
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  ui->SessionStart();

  delete ui;
  delete runManager;
  delete visManager;

  return 0;
}

