#include "SimApplication/DetectorConstruction.h"

namespace ldmx {

    DetectorConstruction::DetectorConstruction(G4GDMLParser* theParser) :
        parser_(theParser),
        auxInfoReader_(new AuxInfoReader(theParser)) {
    }

    DetectorConstruction::~DetectorConstruction() {
        delete auxInfoReader_;
    }

    G4VPhysicalVolume* DetectorConstruction::Construct() {
        auxInfoReader_->readGlobalAuxInfo();
        auxInfoReader_->assignAuxInfoToVolumes();
        return parser_->GetWorldVolume();
    }

    void DetectorConstruction::ConstructSDandField() {

        if (BiasingMessenger::isBiasingEnabled()) { 

            // Instantiate the biasing operator
            // TODO: At some point, this should be more generic i.e. operators should be
            //       similar to plugins.
            XsecBiasingOperator* xsecBiasing 
                = new XsecBiasingOperator("XsecBiasingOperator");

            for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) { 
                G4String volumeName = volume->GetName();
                if ((BiasingMessenger::getVolume().compare("Ecal") == 0)
                        && (volumeName.contains("W") || volumeName.contains("Si"))
                        && volumeName.contains("log")) {
                    xsecBiasing->AttachTo(volume);
                    std::cout << "[ DetectorConstruction ]: "
                              << "Attaching biasing operator " << xsecBiasing->GetName()
                              << " to volume " << volume->GetName() << std::endl;
                } else if (volumeName.contains(BiasingMessenger::getVolume())) { 
                    xsecBiasing->AttachTo(volume);
                    std::cout << "[ DetectorConstruction ]: "
                              << "Attaching biasing operator " << xsecBiasing->GetName()
                              << " to volume " << volume->GetName() << std::endl;
                }
            } 
        } 
    }
}
