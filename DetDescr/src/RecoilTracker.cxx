#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include "../include/DetDescr/RecoilTracker.h"

using std::stringstream;
using std::string;
using std::map;
using std::vector;

namespace ldmx {

    RecoilTrackerStation::RecoilTrackerStation(DetectorElementImpl* tagger, TGeoNode* support, int layerNumber) : DetectorElementImpl(tagger, support) {

        if (layerNumber == -1) {
            // Front layer where node's number can be used.
            layerNumber_ = support->GetNumber();
        } else {
            // Back layers where the number needs to be provided explicitly.
            layerNumber_ = layerNumber;
        }

        stringstream ss;
        ss << std::setfill('0') << std::setw(2) << layerNumber_;
        name_ = "RecoilLayer" + ss.str();

        getDetectorID()->setFieldValue(1, layerNumber_);
        id_ = getDetectorID()->pack();
    }

    RecoilTracker::RecoilTracker() {
        name_ = "RecoilTracker";
    }

    void RecoilTracker::initialize() {

        if (!support_) {
            throw std::runtime_error("The RecoilTracker support is not set.");
        }

        if (!parent_) {
            throw std::runtime_error("The RecoilTracker parent is not set.");
        }

        getDetectorID()->clear();
        getDetectorID()->setFieldValue(0, support_->GetNumber());
        id_ = getDetectorID()->pack();

        /*
         * Build a map of copy numbers to a list of nodes so we can tell which
         * layers have an actual node in the geometry.
         */
        map<int, vector<TGeoNode*>> nodeMap;
        auto dauVec = GeometryUtil::findDauNameStartsWith("recoil", support_);
        for (auto dau : dauVec) {
            int dauNum = dau->GetNumber();
            nodeMap[dauNum].push_back(dau);
        }

        /*
         * Create DE for the layers and sensors.
         * Those copy numbers which have multiple nodes in their list
         * are assumed to be back layers where there is no volume for
         * the entire layer.  So a layer is created without a support
         * node and these nodes are added as children to the layer.
         */
        for (auto entry : nodeMap) {
            int copyNum = entry.first;
            vector<TGeoNode*>& nodes = entry.second;
            if (nodes.size() == 1) {
                new RecoilTrackerStation(this, nodes[0]);
            } else {
                RecoilTrackerStation* layer = new RecoilTrackerStation(this, nullptr, copyNum);
                for (auto sensorNode : nodes) {
                    new RecoilTrackerSensor(layer, sensorNode);
                }
            }
        }
    }

    RecoilTrackerSensor::RecoilTrackerSensor(RecoilTrackerStation* layer, TGeoNode* support) : DetectorElementImpl(layer, support) {
        string nodeName = support->GetName();
        //GeometryUtil::stripNamePointer(nodeName);
        string sensorNum = nodeName.substr(16, 2);
        stringstream ss1;
        ss1 << std::setfill('0') << std::setw(2) << sensorNum;
        stringstream ss2;
        ss2 << std::setfill('0') << std::setw(2) << layer->getLayerNumber();
        name_ = "RecoilTrackerLayer" + ss1.str() + "_Sensor" + ss2.str();
        sensorNumber_ = std::stoi(sensorNum);
    }

    DE_ADD(RecoilTracker)
}
