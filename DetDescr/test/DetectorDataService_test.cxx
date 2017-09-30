/*
 * DetectorDataService_test.cxx
 * @brief Test that loads detector data and prints out DetectorElement information from the hierarchy
 * @author JeremyMcCormick, SLAC
 */

// LDMX
#include "DetDescr/DetectorDataServiceImpl.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "DetDescr/Ecal.h"
#include "DetDescr/Hcal.h"
#include "DetDescr/RecoilTracker.h"
#include "DetDescr/Tagger.h"
#include "DetDescr/Target.h"

using namespace ldmx;
using namespace std;

ostream& operator<<(ostream& stream, const vector<double>& pos) {
    stream << " at (" << pos[0]*10. << ", " << pos[1]*10. << ", " << pos[2]*10. << ")";
    return stream;
}

void find(DetectorDataService* svc, DetectorElement* de, const char* indent = "    ") {
    DetectorElement* srch = nullptr;

    // Check ID lookup.
    if (de->getID()) {
        srch = svc->getDetectorElement(de->getID());
        if (srch) {
            auto decoder = srch->getDetectorID();
            decoder->setRawValue(srch->getID());
            auto values = decoder->unpack();
            if (srch) {
                std::cout << indent << "ID lookup found '" << srch->getName() << "' with ID " << values << std::endl;
            }
        }
    } else {
        std::cout << indent << "Skipping lookup of '" << de->getName() << "' with ID = 0" << std::endl;
    }

    // Check global pos lookup for leaf nodes.
    vector<double> pos = de->getGlobalPosition();
    srch = svc->locateDetectorElement(pos);
    if (srch) {
        std::cout << indent << "Global position lookup found '" << srch->getName() << "'" << de->getGlobalPosition() << std::endl;
    }

    // Check node lookup.
    if (de->getSupport()) {
        TGeoNode* node = de->getSupport();
        srch = svc->findDetectorElement(node);
        if (srch) {
            std::cout << indent << "Node lookup found '" << srch->getName() << "' with support '" << node->GetName() << "'" << std::endl;
        }
    }
}

int main(int argc, const char* argv[])  {

    std::cout << "Running DetectorDataService test ..." << std::endl;

    // Initialize a test detector.
    DetectorDataServiceImpl* svc = new DetectorDataServiceImpl();
    svc->setDetectorName("ldmx-det-full-v3-fieldmap");
    svc->initialize();

    // Get the top DE.
    DetectorElement* top = svc->getTopDetectorElement();

    std::cout << std::endl;

    DetectorID* decoder = nullptr;
    FieldValueList values;

    // Print ECal info.
    DetectorElement* ecal = top->findChild("Ecal");
    decoder = ecal->getDetectorID();
    decoder->setRawValue(ecal->getID());
    values = decoder->unpack();
    std::cout << "Got '" << ecal->getName() << "' with support '" << ecal->getSupport()->GetName() << "'"
            << ecal->getGlobalPosition() << "and ID " << values << std::endl;
    for (auto ecalStation : ecal->getChildren()) {
        decoder->setRawValue(ecalStation->getID());
        values = decoder->unpack();
        std::cout << "  Station '" << ecalStation->getName() << "' with support '"
                << ecalStation->getSupport()->GetName() << "' and layer num "
                << ((EcalStation*)ecalStation)->getLayerNum()
                << ecalStation->getGlobalPosition()
                << " and ID "
                << values
                << std::endl;
        find(svc, ecalStation);
    }
    std::cout << std::endl;

    // Print HCal info.
    DetectorElement* hcal = top->findChild("Hcal");
    decoder = hcal->getDetectorID();
    decoder->setRawValue(hcal->getID());
    values = decoder->unpack();
    std::cout << "Got '" << hcal->getName() << "' with support '" << hcal->getSupport()->GetName() << "'"
            << hcal->getGlobalPosition() << values << std::endl;
    for (auto hcalStation : hcal->getChildren()) {
        decoder->setRawValue(hcalStation->getID());
        values = decoder->unpack();
        std::cout << "  Station '" << hcalStation->getName() << "' with support '"
                << hcalStation->getSupport()->GetName() << "' and station num "
                << ((HcalStation*)hcalStation)->getStationNum()
                << hcalStation->getGlobalPosition()
                << " and ID "
                << values
                << std::endl;
        find(svc, hcalStation);
    }
    std::cout << std::endl;

    // Print Tagger info.
    DetectorElement* tagger = top->findChild("Tagger");
    decoder = tagger->getDetectorID();
    decoder->setRawValue(tagger->getID());
    values = decoder->unpack();
    std::cout << "Got '" << tagger->getName() << "' with support '" << tagger->getSupport()->GetName() << "'"
            << tagger->getGlobalPosition() << values << std::endl;
    decoder = tagger->getDetectorID();
    for (auto taggerLayer : tagger->getChildren()) {
        decoder->setRawValue(taggerLayer->getID());
        values = decoder->unpack();
        std::cout << "  Station '" << taggerLayer->getName() << "' with support '"
                        << taggerLayer->getSupport()->GetName() << "' and layer num "
                        << ((TaggerStation*)taggerLayer)->getLayerNum()
                        << taggerLayer->getGlobalPosition()
                        << " and ID "
                        << values
                        << std::endl;
        find(svc, taggerLayer);
    }
    std::cout << std::endl;

    // Print Recoil Tracker info.
    DetectorElement* recoilTracker = top->findChild("RecoilTracker");
    decoder = recoilTracker->getDetectorID();
    decoder->setRawValue(recoilTracker->getID());
    values = decoder->unpack();
    std::cout << "Got '" << recoilTracker->getName() << "' with support '" << recoilTracker->getSupport()->GetName() << "'"
            << recoilTracker->getGlobalPosition() << values << std::endl;
    for (auto recoilTrackerLayer : recoilTracker->getChildren()) {
        decoder->setRawValue(recoilTrackerLayer->getID());
        values = decoder->unpack();
        std::cout << "  Station '" << recoilTrackerLayer->getName() << "' with support '"
                  << recoilTrackerLayer->getSupport()->GetName() << "' and layer num "
                  << ((TaggerStation*)recoilTrackerLayer)->getLayerNum()
                  << recoilTrackerLayer->getGlobalPosition()
                  << " and ID "
                  << values
                  << std::endl;
        find(svc, recoilTrackerLayer);
    }
    std::cout << std::endl;

    // Print target info.
    DetectorElement* target = top->findChild("Target");
    decoder = target->getDetectorID();
    decoder->setRawValue(target->getID());
    values = decoder->unpack();
    std::cout << "Got '" << target->getName() << "' with support '" << target->getSupport()->GetName() << "'" << target->getGlobalPosition() << values << std::endl;
    std::cout << "  targetThickness = " << ((Target*)target)->getTargetThickness() << std::endl;
    find(svc, target, "  ");
    std::cout << std::endl;

    // Print trigger pad info.
    DetectorElement* triggerPad = top->findChild("TriggerPadUp");
    decoder = triggerPad->getDetectorID();
    decoder->setRawValue(triggerPad->getID());
    values = decoder->unpack();
    std::cout << "Got '" << triggerPad->getName() << "' with support '" << triggerPad->getSupport()->GetName() << "'" << triggerPad->getGlobalPosition() << values << std::endl;
    find(svc, triggerPad, "  ");
    std::cout << std::endl;
    triggerPad = top->findChild("TriggerPadDown");
    decoder = triggerPad->getDetectorID();
    decoder->setRawValue(triggerPad->getID());
    values = decoder->unpack();
    std::cout << "Got '" << triggerPad->getName() << "' with support '" << triggerPad->getSupport()->GetName() << "'" << triggerPad->getGlobalPosition() << values << std::endl;
    find(svc, triggerPad, "  ");

    // Delete the service object, which will delete the DetectorElement tree and the ROOT geometry manager.
    delete svc;

    std::cout << std::endl;

    std::cout << "Done running DetectorDataService test!" << std::endl;
}
