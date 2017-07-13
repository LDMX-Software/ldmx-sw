#include "DetDescr/DetectorDataServiceImpl.h"

// ROOT
#include "TGeoManager.h"
#include "TGeoNavigator.h"
#include "TGeoExtension.h"
#include "TMap.h"
#include "TObjString.h"

// LDMX
#include "DetDescr/Top.h"

// C++
#include <queue>
#include <iostream>
#include <cstdlib>
#include <dirent.h>

using namespace std;

namespace ldmx {

    void DetectorDataServiceImpl::setupLocalAliases() {
        const char* envDir = std::getenv("LDMXSW_DIR");
        if (!envDir) {
            // FIXME: Should use base Exception class here.
            throw std::runtime_error("The LDMXSW_DIR env variable is not set.");
        }
        std::string baseDir = std::string(envDir) + "/data/detectors";
        DIR* dir = opendir(baseDir.c_str());
        struct dirent* entry = readdir(dir);
        while (entry != nullptr) {
            if (entry->d_type == DT_DIR) {
                std::string detectorName = entry->d_name;
                if (detectorName != "." && detectorName != "..") {
                    std::string location = baseDir + "/" + detectorName + "/detector_full.gdml";
                    //std::cout << "adding detector alias: " << detectorName << " -> " << location << std::endl;
                    addAlias(detectorName, location);
                }
            }
            entry = readdir(dir);
        }

    }

    void DetectorDataServiceImpl::initialize() {

        // Throw an error if detector name is not set.
        if (detectorName_.empty()) {
            // FIXME: Should use base Exception class here.
            throw std::runtime_error("The detector name was not set.");
        }

        // Throw an error if there is no known alias (location) for this detector.
        if (aliasMap_.find(detectorName_) == aliasMap_.end()) {
            // FIXME: Should use base Exception class here.
            std::cerr << "ERROR: There is no entry for the detector name "
                    << detectorName_ << " in the aliases." << std::endl;
            throw std::runtime_error("Detector name was not found in the aliases.");
        }

        // Get the location on disk of the detector name (presumably a GDML file).
        std::string location = aliasMap_[detectorName_];

        std::cout << "[ DetectorDataServiceImpl ] : Importing detector " << detectorName_
                << " from " << location << std::endl;

        // Import the geometry into ROOT.
        geoManager_ = TGeoManager::Import(location.c_str(), detectorName_.c_str());

        // Build detector element tree from loaded ROOT geometry.
        buildDetectorElements();

        // Build the global matrix cache in the ROOT geometry manager.
        TGeoNavigator* nav = geoManager_->GetCurrentNavigator();
        nav->BuildCache();

        // Set the global positions on each DetectorElement by walking the hierarchy.
        GlobalPositionCacheBuilder globPosBuilder(nav);
        DetectorElementVisitor::walk(deTop_, &globPosBuilder);

        // Cache maps of ID and nodes to DetectorElements.
        DetectorElementCacheBuilder cacheBuilder(&deCache_);
        DetectorElementVisitor::walk(deTop_, &cacheBuilder);
    }

    DetectorElement* DetectorDataServiceImpl::findDetectorElement(TGeoNode* node) {
        DetectorElement* de = nullptr;
        if (deCache_.contains(node)) {
            /*
             * Return assigned node from the cache.
             */
            de = deCache_.get(node);
        } else {
            /*
             * Search up the geometry hierarchy until a DetectorElement is found
             * or the top is reached.
             */
            TGeoNavigator* nav = geoManager_->GetCurrentNavigator();
            TGeoNode* top = deTop_->getSupport();
            TGeoNode* cur = node;
            while (cur) {
                nav->CdUp();
                cur = nav->GetCurrentNode();
                if (deCache_.contains(cur)) {
                    de = deCache_.get(cur);
                    break;
                }
                if (cur == top) {
                    de = deTop_;
                    break;
                }
            }
        }
        return de;
    }


    DetectorElement* DetectorDataServiceImpl::locateDetectorElement(std::vector<double>& globalPosition) {
        TGeoNavigator* nav = geoManager_->GetCurrentNavigator();
        nav->SetCurrentPoint(globalPosition[0], globalPosition[1], globalPosition[2]);
        TGeoNode* node = nav->FindNode();
        return findDetectorElement(node);
    }

    void DetectorDataServiceImpl::buildDetectorElements() {
        geoManager_->GetCache()->BuildIdArray();
        int nnodes = geoManager_->GetNNodes();
        for (int inode = 0; inode < nnodes; inode++) {
            geoManager_->CdNode(inode);
            TGeoNode* curr = geoManager_->GetCurrentNode();
            TGeoRCExtension* ext = (TGeoRCExtension*) curr->GetVolume()->GetUserExtension();
            if (ext) {
                TMap* aux = (TMap*) ext->GetUserObject();
                TObjString* obj = (TObjString*) aux->GetValue("DetElem");
                if (obj) {
                    TString str = obj->GetString();
                    //std::cout << "creating DE with type <" << str << ">" << std::endl;
                    DetectorElementImpl* de = (DetectorElementImpl*) fac_->create(str.Data());
                    de->setSupport(curr);
                    if (str == "Top") {
                        this->deTop_ = de;
                        //std::cout << "assigned top DE with node " << curr->GetName() << std::endl;
                    } else {
                        de->setParent(deTop_);
                        deTop_->addChild(de);
                    }
                    de->initialize();
                    //std::cout << "created DE <" << de->getName() << "> with support <" << de->getSupport()->GetName() << ">" << std::endl;
                }
            }
        }
    }

    /**
     * @todo Make this function more efficient, because it currently resets the navigator for every node
     * in the hierarchy.
     */
    void GlobalPositionCacheBuilder::visit(DetectorElement* de) {

        /*
         * Make a list of nodes from this DetectorElement to the top.
         */
        vector<TGeoNode*> nodes;
        DetectorElement* cur = de;
        while (cur) {
            if (cur->getSupport()) {
                nodes.push_back(cur->getSupport());
            }
            cur = cur->getParent();
        }

        /*
         * Set the navigator by going into each daughter node.
         */
        nav_->cd();
        for (int i = nodes.size() - 1; i >= 0; i--) {
            nav_->CdDown(nodes[i]);
        }

        /*
         * Set the global position on the DetectorElement from the navigator state.
         */
        Double_t* trans = nav_->GetCurrentMatrix()->GetTranslation();
        static_cast<DetectorElementImpl*>(de)->setGlobalPosition(trans[0], trans[1], trans[2]);
        const vector<double>& pos = de->getGlobalPosition();
    }

}
