/*
 * DetectorDataService_test.cxx
 * @brief Test that loads detector data and prints out DetectorElement information from the hierarchy
 * @author JeremyMcCormick, SLAC
 */

// LDMX
#include "DetDescr/DetectorDataServiceImpl.h"
#include "DetDescr/EcalDetectorElement.h"
#include "DetDescr/HcalDetectorElement.h"
#include "DetDescr/RecoilTrackerDetectorElement.h"
#include "DetDescr/TaggerDetectorElement.h"
#include "DetDescr/TargetDetectorElement.h"

// STL
#include <iostream>
#include <vector>
#include <sstream>

using namespace ldmx;
using namespace std;

ostream& operator<<(ostream& stream, const vector<double>& pos) {
    stream << " at (" << pos[0]*10. << ", " << pos[1]*10. << ", " << pos[2]*10. << ")";
    return stream;
}

ostream& operator<<(ostream& stream, DetectorID::FieldValueList& id) {
    stream << " with id [";
    for (int i = 0; i < id.size(); i++) {
        stream << id[i];
        if (i != id.size() - 1)
            stream << "/";    
    }
    stream << "]";
    return stream;
}

int main(int, const char* argv[])  {

    std::cout << "Running DetectorDataService test ..." << std::endl;

    // Initialize a detector.
    DetectorDataServiceImpl* svc = new DetectorDataServiceImpl();
    svc->setDetectorName("ldmx-det-full-v1-fieldmap");
    svc->initialize();

    // Get the top DE.
    DetectorElement* top = svc->getTopDetectorElement();

    std::cout << std::endl;

    DetectorID* decoder = nullptr;
    DetectorID::FieldValueList values;

    // Print ECal info.
    DetectorElement* ecal = top->findChild("Ecal");
    decoder = ecal->getDetectorID();
    decoder->setRawValue(ecal->getID());
    values = decoder->unpack();
    std::cout << "Got 'Ecal' DE with support '" << ecal->getSupport()->GetName() << "'"
            << ecal->getGlobalPosition() << values << std::endl;
    for (auto ecalLayer : ecal->getChildren()) {
        decoder->setRawValue(ecalLayer->getID());
        values = decoder->unpack();
        std::cout << "  " << ecalLayer->getName() << " with support "
                << ecalLayer->getSupport()->GetName() << " and layer num "
                << ((EcalLayer*)ecalLayer)->getLayerNumber()
                << ecalLayer->getGlobalPosition()
                << values
                << std::endl;
    }
    std::cout << std::endl;

    // Print HCal info.
    DetectorElement* hcal = top->findChild("Hcal");
    decoder = hcal->getDetectorID();
    decoder->setRawValue(hcal->getID());
    values = decoder->unpack();
    std::cout << "Got 'Hcal' DE with support '" << hcal->getSupport()->GetName() << "'"
            << hcal->getGlobalPosition() << values << std::endl;
    for (auto hcalStation : hcal->getChildren()) {
        decoder->setRawValue(hcalStation->getID());
        values = decoder->unpack();
        std::cout << "  " << hcalStation->getName() << " with support "
                << hcalStation->getSupport()->GetName() << " and station num "
                << ((HcalStation*)hcalStation)->getStationNumber()
                << hcalStation->getGlobalPosition()
                << values
                << std::endl;
    }

    std::cout << std::endl;

    // Print Tagger info.
    DetectorElement* tagger = top->findChild("Tagger");
    decoder = tagger->getDetectorID();
    decoder->setRawValue(tagger->getID());
    values = decoder->unpack();
    std::cout << "Got 'Tagger' DE with support '" << tagger->getSupport()->GetName() << "'"
            << tagger->getGlobalPosition() << values << std::endl;
    decoder = tagger->getDetectorID();
    for (auto taggerLayer : tagger->getChildren()) {
        decoder->setRawValue(taggerLayer->getID());
        values = decoder->unpack();
        std::cout << "  " << taggerLayer->getName() << " with support "
                        << taggerLayer->getSupport()->GetName() << " and layer num "
                        << ((TaggerLayer*)taggerLayer)->getLayerNumber()
                        << taggerLayer->getGlobalPosition()
                        << values
                        << std::endl;
    }

    std::cout << std::endl;

    // Print Recoil Tracker info.
    DetectorElement* recoilTracker = top->findChild("RecoilTracker");
    decoder = recoilTracker->getDetectorID();
    decoder->setRawValue(recoilTracker->getID());
    values = decoder->unpack();
    std::cout << "Got 'RecoilTracker' DE with support '" << recoilTracker->getSupport()->GetName() << "'"
            << recoilTracker->getGlobalPosition() << values << std::endl;
    for (auto recoilTrackerLayer : recoilTracker->getChildren()) {
        decoder->setRawValue(recoilTrackerLayer->getID());
        values = decoder->unpack();
        if (recoilTrackerLayer->getSupport()) {
            std::cout << "  " << recoilTrackerLayer->getName() << " with support "
                    << recoilTrackerLayer->getSupport()->GetName() << " and layer num "
                    << ((TaggerLayer*)recoilTrackerLayer)->getLayerNumber()
                    << recoilTrackerLayer->getGlobalPosition()
                    << values
                    << std::endl;
        } else {
            std::cout << "  " << recoilTrackerLayer->getName() << " with layer num "
                    << ((TaggerLayer*)recoilTrackerLayer)->getLayerNumber() << values << std::endl;
            for (auto recoilSensor : recoilTrackerLayer->getChildren()) {
                std::cout << "    " << recoilSensor->getName() << " with support "
                        << recoilSensor->getSupport()->GetName() << " and sensor num "
                        << ((RecoilTrackerSensor*)recoilSensor)->getSensorNumber()
                        << recoilSensor->getGlobalPosition()
                        << std::endl;
            }
        }
    }
    std::cout << std::endl;

    // Print target info.
    DetectorElement* target = top->findChild("Target");
    decoder = target->getDetectorID();
    decoder->setRawValue(target->getID());
    values = decoder->unpack();
    std::cout << "Got 'Target' DE with support " << target->getSupport()->GetName() << "'" << target->getGlobalPosition() << values << std::endl;
    std::cout << "  targetThickness = " << ((TargetDetectorElement*)target)->getTargetThickness() << std::endl;
    std::cout << std::endl;

    // Print trigger pad info.
    DetectorElement* triggerPad = top->findChild("TriggerPadUp");
    decoder = triggerPad->getDetectorID();
    decoder->setRawValue(triggerPad->getID());
    values = decoder->unpack();
    std::cout << "Got 'TriggerPadUp' with support '" << triggerPad->getSupport()->GetName() << "'" << triggerPad->getGlobalPosition() << values << std::endl;
    std::cout << std::endl;
    triggerPad = top->findChild("TriggerPadDown");
    decoder = triggerPad->getDetectorID();
    decoder->setRawValue(triggerPad->getID());
    values = decoder->unpack();
    std::cout << "Got 'TriggerPadDown' with support " << triggerPad->getSupport()->GetName() << "'" << triggerPad->getGlobalPosition() << values << std::endl;

    // Delete the service object, which will delete the DetectorElement tree and the ROOT geometry manager.
    delete svc;

    std::cout << std::endl;
    std::cout << "Done running DetectorDataService test!" << std::endl;
}
