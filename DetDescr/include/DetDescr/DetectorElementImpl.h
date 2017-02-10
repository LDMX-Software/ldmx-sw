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
             *
             * @note
             * Sets up this DetectorElement with the given parent (can be null)
             * and support (can also be null).
             *
             * @note
             * It sets the parent DE on this object, and it will
             * also add this object as a child of the parent, if the
             * parent is not null.
             */
            DetectorElementImpl(DetectorElementImpl* parent, TGeoNode* support = nullptr) {
                parent_ = parent;
                if (parent_ != nullptr) {
                    parent_->addChild(this);
                }
                support_ = support;
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
                return detID_;
            }

            const std::string& getName() {
                return name_;
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
