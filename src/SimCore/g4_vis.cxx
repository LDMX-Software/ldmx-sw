#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4RunManager.hh"
#include "G4GDMLParser.hh"
#include "G4GeometryManager.hh"

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/DetectorConstruction.h"

int main(int argc, char* argv[]) {

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
  parser->Read("detector.gdml",false);
  runManager->DefineWorldVolume(parser->GetWorldVolume());

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

