#include "SimApplication/DetectorConstruction.h"

namespace ldmx {

    DetectorConstruction::DetectorConstruction(G4GDMLParser* theParser) :
            parser_(theParser), auxInfoReader_(new AuxInfoReader(theParser)) {
    }

    DetectorConstruction::~DetectorConstruction() {
        delete auxInfoReader_;
    }

    G4VPhysicalVolume* DetectorConstruction::Construct() {
        auxInfoReader_->readGlobalAuxInfo();
        auxInfoReader_->assignAuxInfoToVolumes();
        addParserAuxInfo();
        return parser_->GetWorldVolume();
    }

    void DetectorConstruction::addParserAuxInfo() {
        // Add user info to writer output.
        const G4GDMLAuxListType* auxList = parser_->GetAuxList();
        for (G4GDMLAuxListType::const_iterator it = auxList->begin();
                it != auxList->end();
                it++) {
            parser_->AddAuxiliary(*it);
        }

        // Add volume auxiliary tags to writer output.
        const G4GDMLAuxMapType* auxMap = parser_->GetAuxMap();
        for (G4GDMLAuxMapType::const_iterator it = auxMap->begin();
                it != auxMap->end();
                it++) {
            for (std::vector<G4GDMLAuxStructType>::const_iterator auxIt = it->second.begin();
                    auxIt != it->second.end();
                    auxIt++) {
                parser_->AddVolumeAuxiliary(*auxIt, it->first);
            }
        }
    }

    void DetectorConstruction::ConstructSDandField() {

        if (BiasingMessenger::isBiasingEnabled()) {

            // Instantiate the biasing operator
            // TODO: At some point, this should be more generic i.e. operators should be
            //       similar to plugins.
            XsecBiasingOperator* xsecBiasing = nullptr; 
            if (BiasingMessenger::getProcess().compare("photonNuclear") == 0) { 
                xsecBiasing = new PhotoNuclearXsecBiasingOperator("PhotoNuclearXsecBiasingOperator");
            } else if (BiasingMessenger::getProcess().compare("GammaToMuPair") == 0) { 
                xsecBiasing = new GammaToMuPairXsecBiasingOperator("GammaToMuPairXsecBiasingOperator");
            } else if (BiasingMessenger::getProcess().compare("electronNuclear") == 0) { 
                xsecBiasing = new ElectroNuclearXsecBiasingOperator("ElectroNuclearXsecBiasingOperator");
            }

            for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
                G4String volumeName = volume->GetName();
                //std::cout << "[ DetectorConstruction ]: " << "Volume: " << volume->GetName() << std::endl;
                if ((BiasingMessenger::getVolume().compare("ecal") == 0) 
                        && (volumeName.contains("Wthick") 
                            || volumeName.contains("Si")
                            || volumeName.contains("W")) 
                        && volumeName.contains("volume")) {
                    xsecBiasing->AttachTo(volume);
                    std::cout << "[ DetectorConstruction ]: " << "Attaching biasing operator " 
                              << xsecBiasing->GetName() << " to volume " 
                              << volume->GetName() << std::endl;
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
