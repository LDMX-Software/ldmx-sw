#include "DetDescr/DetectorDataServiceImpl.h"

using namespace ldmx;

#include "DetDescr/EcalDetectorElement.h"
#include "DetDescr/HcalDetectorElement.h"
#include "DetDescr/TaggerDetectorElement.h"

#include <iostream>

int main(int, const char* argv[])  {

    std::cout << "Running DetectorDataService test ..." << std::endl;

    // Initialize a detector.
    DetectorDataServiceImpl* svc = new DetectorDataServiceImpl();
    svc->setDetectorName("ldmx-det-full-v1-fieldmap");
    svc->initialize();

    // Get the top DE.
    DetectorElement* top = svc->getTopDetectorElement();

    std::cout << std::endl;

    // Print ECal info.
    DetectorElement* ecal = top->findChild("Ecal");
    std::cout << "Got 'Ecal' DE with support '" << ecal->getSupport()->GetName() << "'" << std::endl;
    for (auto ecalLayer : ecal->getChildren()) {
        std::cout << "  " << ecalLayer->getName() << " with support "
                << ecalLayer->getSupport()->GetName() << " and layer num "
                << ((EcalLayer*)ecalLayer)->getLayerNumber()
                << std::endl;
    }

    std::cout << std::endl;

    // Print HCal info.
    DetectorElement* hcal = top->findChild("Hcal");
    std::cout << "Got 'Hcal' DE with support '" << hcal->getSupport()->GetName() << "'" << std::endl;
    for (auto hcalStation : hcal->getChildren()) {
        std::cout << "  " << hcalStation->getName() << " with support "
                << hcalStation->getSupport()->GetName() << " and station num "
                << ((HcalStation*)hcalStation)->getStationNumber()
                << std::endl;
    }

    std::cout << std::endl;

    // Print Tagger info.
    DetectorElement* tagger = top->findChild("Tagger");
    std::cout << "Got 'Tagger' DE with support '" << tagger->getSupport()->GetName() << "'" << std::endl;
    for (auto taggerLayer : tagger->getChildren()) {
        std::cout << "  " << taggerLayer->getName() << " with support "
                        << taggerLayer->getSupport()->GetName() << " and layer num "
                        << ((TaggerLayer*)taggerLayer)->getLayerNumber()
                        << std::endl;
    }

    std::cout << std::endl;

    // Print Recoil Tracker info.
    DetectorElement* recoilTracker = top->findChild("RecoilTracker");
    std::cout << "Got 'RecoilTracker' DE with support " << recoilTracker->getSupport()->GetName() << "'" << std::endl;
    for (auto recoilTrackerLayer : recoilTracker->getChildren()) {
        if (recoilTrackerLayer->hasSupport()) {
            std::cout << "  " << recoilTrackerLayer->getName() << " with support "
                    << recoilTrackerLayer->getSupport()->GetName() << " and layer num "
                    << ((TaggerLayer*)recoilTrackerLayer)->getLayerNumber()
                    << std::endl;
        } else {
            std::cout << "  " << recoilTrackerLayer->getName() << std::endl;
            for (auto recoilSensor : recoilTrackerLayer->getChildren()) {
                std::cout << "    " << recoilSensor->getName() << " with support "
                        << recoilSensor->getSupport()->GetName() << std::endl;
            }
        }
    }

    // Destroy the service object, which will delete the DetectorElement tree and the ROOT geometry manager.
    delete svc;

    std::cout << "Done running DetectorDataService test!" << std::endl;
}
