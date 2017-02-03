#include "SimApplication/DetectorConstruction.h"

//
#include "SimApplication/RunManager.h"

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

        BiasingMessenger* biasingMessenger  
            = static_cast<RunManager*>(G4RunManager::GetRunManager())->getBiasingMessenger(); 

        if (biasingMessenger->isBiasingEnabled()) { 

            // Instantiate the biasing operator
            // TODO: At some point, this should be more generic i.e. operators should be
            //       similar to plugins.
            XsecBiasingOperator* xsecBiasing 
                = new XsecBiasingOperator("XsecBiasingOperator");

            for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) { 
                G4String volumeName = volume->GetName();
                if (volumeName.contains(biasingMessenger->getVolume())) { 
                    xsecBiasing->AttachTo(volume);
                    std::cout << "[ DetectorConstruction ]: "
                              << "Attaching biasing operator " << xsecBiasing->GetName()
                              << " to volume " << volume->GetName() << std::endl;
                }
            } 
        } 
    }
}
