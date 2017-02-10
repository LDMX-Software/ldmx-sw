/*
 * DetectorElementImpl.h
 * @brief Implementation of DetectorElement class
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORELEMENTIMPL_H_
#define DETDESCR_DETECTORELEMENTIMPL_H_

namespace ldmx {

    class DetectorElementImpl : public DetectorElement {

        public:

            void setup(DetectorDataService* svc) {
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

        private:

            TGeoNode* support_;
            DetectorElementVector children_;
            DetectorElement* parent_;
            DetectorID id_;
            DetectorID* detID_;
    };
}



#endif /* DETDESCR_DETECTORELEMENTIMPL_H_ */
