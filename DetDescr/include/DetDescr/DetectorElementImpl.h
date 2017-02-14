/*
 * DetectorElementImpl.h
 * @brief Implementation of DetectorElement class
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORELEMENTIMPL_H_
#define DETDESCR_DETECTORELEMENTIMPL_H_

// ROOT
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TGeoManager.h"

// LDMX
#include "DetDescr/DetectorElement.h"
#include "DetDescr/DetectorID.h"

namespace ldmx {

    class DetectorDataService;

    class DetectorElementImpl : public DetectorElement {

        public:

            /**
             * Class constructor.
             * @param parent The parent DetectorElement of this one.
             * @param support The TGeoNode pointing to the physical geometry node.
             *
             * @note
             * Sets up this DetectorElement with the given parent and support.
             * By default, these arguments are both set to null.
             *
             * @note
             * It sets the parent DE on this object, and it will
             * also add this object as a child of the parent, if the
             * parent is not null.
             */
            DetectorElementImpl(DetectorElementImpl* parent = nullptr, TGeoNode* support = nullptr) {
                parent_ = parent;
                if (parent_ != nullptr) {
                    parent_->addChild(this);
                }
                support_ = support;
            }

            virtual ~DetectorElementImpl() {
                // Each DE is responsible for deleting all of its children.
                for (auto child : children_) {
                    delete child;
                }

                // Delete the detector ID.
                if (detID_) {
                    delete detID_;
                }
            }

            TGeoNode* getSupport() {
                return support_;
            }

            void setSupport(TGeoNode* support) {
                support_ = support;
            }

            bool hasSupport() {
                return support_ != nullptr;
            }

            const DetectorElementVector& getChildren() {
                return children_;
            }

            void addChild(DetectorElement* child) {
                children_.push_back(child);
            }

            DetectorElement* getParent() {
                return parent_;
            }

            TGeoVolume* getVolume() {
                if (support_) {
                    return support_->GetVolume();
                } else {
                    return nullptr;
                }
            }

            DetectorID::RawValue getID() {
                return id_;
            }

            DetectorID* getDetectorID() {
                if (detID_) {
                    return detID_;
                } else {
                    if (parent_) {
                        return parent_->getDetectorID();
                    } else {
                        return nullptr;
                    }
                }
            }

            const std::string& getName() {
                return name_;
            }

            void setName(std::string name) {
                name_ = name;
            }

            DetectorElement* findChild(std::string name) {
                DetectorElement* foundChild = nullptr;
                for (auto child : children_) {
                    if (!child->getName().compare(name)) {
                        foundChild = child;
                        break;
                    }
                }
                return foundChild;
            }

        protected:

            TGeoNode* support_{nullptr};
            DetectorElementVector children_;
            DetectorElementImpl* parent_{nullptr};
            DetectorID::RawValue id_{0};
            DetectorID* detID_{nullptr};
            std::string name_;
    };
}

#endif /* DETDESCR_DETECTORELEMENTIMPL_H_ */
