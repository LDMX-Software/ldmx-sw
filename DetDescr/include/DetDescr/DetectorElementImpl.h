/*
 * DetectorElementImpl.h
 * @brief Implementation of DetectorElement interface
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

    /**
     * @class DetectorElementImpl
     * @brief Implements the DetectorElement interface
     */
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

            /**
             * Class destructor.
             * @note Deletes child DetectorElement objects and the DetectorID.
             */
            virtual ~DetectorElementImpl() {
                // Each DE is responsible for deleting all of its children.
                for (auto child : children_) {
                    delete child;
                }

                // Delete the detector ID.
                if (detectorID_) {
                    delete detectorID_;
                }
            }

            /**
             * Get the geometry node corresponding to this component.
             * @return The geometry node corresponding to this component.
             * @note This can be null if the component is a container only
             * with no corresponding node in the geometry.
             */
            TGeoNode* getSupport() {
                return support_;
            }

            /**
             * Get the list of child DetectorElement objects.
             * @return The list of child DetectorElement objects.
             */
            const DetectorElementVector& getChildren() {
                return children_;
            }

            /**
             * Get the parent DetectorElement containing this one.
             * @return The parent DetectorElement containing this one.
             */
            DetectorElement* getParent() {
                return parent_;
            }

            /**
             * Get the volume of the geometry node.
             * @return The volume of the geometry node.
             */
            TGeoVolume* getVolume() {
                if (support_) {
                    return support_->GetVolume();
                } else {
                    return nullptr;
                }
            }

            /**
             * Get the raw ID assigned to this component.
             * @return The raw ID assigned to this component.
             */
            DetectorID::RawValue getID() {
                return id_;
            }

            /**
             * Get the ID decoder assigned to this component.
             * @return The ID decoder assigned to this component.
             */
            DetectorID* getDetectorID() {
                if (detectorID_) {
                    return detectorID_;
                } else {
                    if (parent_) {
                        return parent_->getDetectorID();
                    } else {
                        return nullptr;
                    }
                }
            }

            /**
             * Get the name of the DetectorElement.
             * @return The name of the DetectorElement.
             */
            const std::string& getName() {
                return name_;
            }

            /**
             * Find the first child with a given name.
             * @param name The first child with a given name.
             * @note This should return one and only one
             * DetectorElement provided the child names
             * are unique.
             */
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

            /**
             * Set the global position of the DetectorElement.
             * @param x The X position.
             * @param y The Y position.
             * @param z The Z position.
             */
            void setGlobalPosition(double x, double y, double z) {
                globalPosition_[0] = x;
                globalPosition_[1] = y;
                globalPosition_[2] = z;
            }

            /**
             * Get the global position of this DetectorElement as vector of doubles.
             * @return The global position of this DetectorElement.
             */
            virtual const std::vector<double>& getGlobalPosition() {
                return globalPosition_;
            }

            void setSupport(TGeoNode* node) {
                support_ = node;
            }

            void setParent(DetectorElementImpl* parent) {
                parent_ = parent;
            }

            /**
             * Add a child DetectorElement to this one.
             * @param The child DetectorElement.
             * @note This does not set the parent link of
             * the child, which must be done separately.
             */
            void addChild(DetectorElement* child) {
                children_.push_back(child);
            }

            /**
             * Initialization hook for adding custom setup code.
             */
            virtual void initialize() {
            }

        protected:

            /** The backing geometry node. */
            TGeoNode* support_{nullptr};

            /** The collection of child DetectorElement objects. */
            DetectorElementVector children_;

            /** The parent DetectorElement. */
            DetectorElementImpl* parent_{nullptr};

            /** The component's raw ID value. */
            DetectorID::RawValue id_{0};

            /** The detector ID decoder. */
            DetectorID* detectorID_{nullptr};

            /** The name of the DetectorElement. */
            std::string name_;

            /** Global position of this DetectorElement. */
            std::vector<double> globalPosition_{0, 0, 0};
    };
}

#endif /* DETDESCR_DETECTORELEMENTIMPL_H_ */
