/*
 * @file DetectorElement.h
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
#include <iostream>

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

            virtual void initialize() = 0;
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

    /**
     * @brief
     * Template method for creating a DetectorElement, used as a function pointer
     * by the DetectorElementFactory.
     */
    template<typename T> DetectorElement* createType() {
        return new T;
    }

    /**
     * @brief
     * Factory for creating DetectorElement objects by their type name
     * (e.g. "Ecal", "RecoilTracker", etc.)
     */
    class DetectorElementFactory {

        public:

            /**
             * Type for DetectorElement creation function.
             */
            typedef DetectorElement* (*CreatorFunc)();

            /**
             * Type for mapping DetectorElement names to creation functions.
             */
            typedef std::map<std::string, CreatorFunc> FuncMap;

            /**
             * Map of names to creation functions.
             */
            FuncMap map_;

            /**
             * Get a static instance of the DetectorElementFactory.
             */
            static DetectorElementFactory* instance() {
                static DetectorElementFactory fac;
                return &fac;
            }

            /**
             * Create an instance of a DetectorElement by its type name.
             */
            DetectorElement* create(std::string name) {
                DetectorElementFactory::FuncMap::iterator it = map_.find(name);
                if (it != map_.end()) {
                    return it->second();
                } else {
                    std::cerr << "No DetectorElement found with type <" << name << ">" << std::endl;
                    throw std::runtime_error("Failed to find DetectorElement type.");
                }
            }

        public:

            /**
             * Add a new DetectorElement type by registering a function pointer
             * for creating it.
             */
            template<typename T>
            short add(const char* name) {
                CreatorFunc func = &createType<T>;
                map_[name] = func;
                return 0;
            }
    };

    /**
     * Macro for initializing a DetectorElement which is just a placeholder var to
     * be used by statically initializing it.
     */
    #define DE_INIT(NAME) \
    static short NAME##Init;

    /**
     * Macro for adding a DetectorElement by registering its creation function.
     */
    #define DE_ADD(NAME) \
    static short NAME##Init = DetectorElementFactory::instance()->add<NAME>(#NAME);
}

#endif /* DETDESCR_DETECTORELEMENT_H_ */
