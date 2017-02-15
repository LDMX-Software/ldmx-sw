/*
 * DetectorElement.h
 * @brief Interface defining an identifiable component in the detector hierarchy
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORELEMENT_H_
#define DETDESCR_DETECTORELEMENT_H_

// ROOT
#include "TGeoNode.h"

// LDMX
#include "DetDescr/DetectorID.h"

#include <vector>

namespace ldmx {

    /**
     * @class DetectorElement
     * @brief Interface for accessing a component in the detector component hierarchy
     * @note DetectorElement objects should be given globally unique names and IDs
     * when they are initialized.
     */
    class DetectorElement {

        public:

            /**
             * Vector of DetectorElement.
             */
            typedef std::vector<DetectorElement*> DetectorElementVector;

            /**
             * Class destructor.
             */
            virtual ~DetectorElement() {;}

            /**
             * Get the geometry node corresponding to this component.
             * @return The geometry node corresponding to this component.
             * @note This can be null if the component is a container only
             * with no corresponding node in the geometry.
             */
            virtual TGeoNode* getSupport() = 0;

            /**
             * Get the list of child DetectorElement objects.
             * @return The list of child DetectorElement objects.
             */
            virtual const DetectorElementVector& getChildren() = 0;

            /**
             * Get the parent DetectorElement containing this one.
             * @return The parent DetectorElement containing this one.
             */
            virtual DetectorElement* getParent() = 0;

            /**
             * Get the volume of the geometry node.
             * @return The volume of the geometry node.
             */
            virtual TGeoVolume* getVolume() = 0;

            /**
             * Get the raw ID assigned to this component.
             * @return The raw ID assigned to this component.
             */
            virtual DetectorID::RawValue getID() = 0;

            /**
             * Get the ID decoder assigned to this component.
             * @return The ID decoder assigned to this component.
             */
            virtual DetectorID* getDetectorID() = 0;

            /**
             * Get the name of the DetectorElement.
             * @return The name of the DetectorElement.
             */
            virtual const std::string& getName() = 0;

            /**
             * Find the first child with a given name.
             * @param name The first child with a given name.
             */
            virtual DetectorElement* findChild(std::string name) = 0;

            /**
             * Get the global position of the DetectorElement.
             * @return The global position of the DetectorElement.
             */
            virtual const std::vector<double>& getGlobalPosition() = 0;
    };

    /**
     * @class DetectorElementVisitor
     * @brief Interface for visiting DetectorElement nodes
     */
    class DetectorElementVisitor {

        public:

            /**
             * Class destructor.
             */
            virtual ~DetectorElementVisitor() {;}

            /**
             * Function to be implemented by the visitor.
             * @param de The DetectorElement to visit.
             */
            virtual void visit(DetectorElement* de) = 0;

            /**
             * Walk a DetectorElement tree in level order.
             * @param de The top DetectorElement.
             * @param visitor The visitor to activate at each DetectorElement.
             */
            static void walk(DetectorElement* de, DetectorElementVisitor* visitor);
    };
}

#endif /* DETDESCR_DETECTORELEMENT_H_ */
