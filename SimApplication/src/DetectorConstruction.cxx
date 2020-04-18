#include "SimApplication/DetectorConstruction.h"

/*~~~~~~~~~~~~~~~*/
/*   Exception   */
/*~~~~~~~~~~~~~~~*/
#include "Exception/Exception.h" 

namespace ldmx {

    DetectorConstruction::DetectorConstruction(G4GDMLParser* theParser, Parameters& parameters) :
            parser_(theParser), auxInfoReader_(new AuxInfoReader(theParser)) {
                parameters_ = parameters; 
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

        auto biasingEnabled{parameters_.getParameter< bool >("biasing.enabled")};
        if (biasingEnabled) {

            auto biasingProcess{parameters_.getParameter< std::string >("biasing.process")}; 
            auto biasingVolume{parameters_.getParameter< std::string >("biasing.volume")};
            auto biasingParticle{parameters_.getParameter< std::string >("biasing.particle")}; 
            auto biasAll{parameters_.getParameter< bool >("biasing.all")}; 
            auto biasIncident{parameters_.getParameter< bool >("biasing.incident")}; 
            auto disableEMBiasing{parameters_.getParameter< bool >("biasing.disableEMBiasing")};
            auto biasThreshold{parameters_.getParameter< double >("biasing.threshold")}; 
            auto biasFactor{parameters_.getParameter< int >("biasing.factor")}; 

            // Instantiate the biasing operator
            // TODO: At some point, this should be more generic i.e. operators should be
            //       similar to plugins.
            XsecBiasingOperator* xsecBiasing{nullptr}; 
            if (biasingProcess.compare("photonNuclear") == 0) { 
                xsecBiasing = new PhotoNuclearXsecBiasingOperator("PhotoNuclearXsecBiasingOperator");
            } else if (biasingProcess.compare("GammaToMuPair") == 0) { 
                xsecBiasing = new GammaToMuPairXsecBiasingOperator("GammaToMuPairXsecBiasingOperator");
            } else if (biasingProcess.compare("electronNuclear") == 0) { 
                xsecBiasing = new ElectroNuclearXsecBiasingOperator("ElectroNuclearXsecBiasingOperator");
            } else if (biasingProcess.compare("eDBrem") == 0) { 
                xsecBiasing = new DarkBremXsecBiasingOperator("DarkBremXsecBiasingOperator");
            } else {
                EXCEPTION_RAISE("BiasingException", "Invalid process name '" + biasingProcess + "'." ); 
            }

            // Configure the operator
            xsecBiasing->setParticleType(biasingParticle);
            xsecBiasing->setThreshold(biasThreshold); 
            xsecBiasing->setBiasFactor(biasFactor); 

            if (biasAll) xsecBiasing->biasAll(); 
            else if (biasIncident) xsecBiasing->biasIncident();
            
            if (disableEMBiasing) xsecBiasing->disableBiasDownEM(); 


            for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
                G4String volumeName = volume->GetName();
                //std::cout << "[ DetectorConstruction ]: " << "Volume: " << volume->GetName() << std::endl;
                if ((biasingVolume.compare("ecal") == 0) 
                        && (volumeName.contains("Wthick") 
                            || volumeName.contains("Si")
                            || volumeName.contains("W")) 
                        && volumeName.contains("volume")) {
                    xsecBiasing->AttachTo(volume);
                    std::cout << "[ DetectorConstruction ]: " << "Attaching biasing operator " 
                              << xsecBiasing->GetName() << " to volume " 
                              << volume->GetName() << std::endl;
                } else if (volumeName.contains(biasingVolume)) {
                    xsecBiasing->AttachTo(volume);
                    std::cout << "[ DetectorConstruction ]: " 
                              << "Attaching biasing operator " << xsecBiasing->GetName() 
                              << " to volume " << volume->GetName() << std::endl;
                }
            }
        }
    }
}
