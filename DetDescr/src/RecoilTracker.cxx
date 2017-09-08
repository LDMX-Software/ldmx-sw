#include "DetDescr/RecoilTracker.h"

#include "DetDescr/TrackerID.h"

// C++
#include <map>
#include <vector>
#include <iostream>
#include <sstream>

using std::stringstream;
using std::string;
using std::map;
using std::vector;

namespace ldmx {

    RecoilTrackerStation::RecoilTrackerStation(DetectorElementImpl* tagger, TGeoNode* support) : DetectorElementImpl(tagger, support) {

        stationNum_ = support->GetNumber();
        layerNum_ = support->GetNumber() / 10;
        moduleNum_ = support->GetNumber() % 10;
        
        stringstream ss;
        ss << std::setfill('0') << std::setw(3) << support->GetNumber();
        name_ = "RecoilTrackerStation" + ss.str();

        getDetectorID()->setFieldValue(1, layerNum_);
        getDetectorID()->setFieldValue(2, moduleNum_);
        id_ = getDetectorID()->pack();
    }

    RecoilTracker::RecoilTracker() {
        name_ = "RecoilTracker";
        detectorID_ = new TrackerID();
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

        auto dauVec = GeometryUtil::findDauNameStartsWith("recoil", support_);
        for (auto dau : dauVec) {
            new RecoilTrackerStation(this, dau);
        }
    }

    DE_ADD(RecoilTracker)
}
