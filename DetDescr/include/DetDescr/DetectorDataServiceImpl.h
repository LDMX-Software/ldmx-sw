/*
 * @file DetectorServiceImpl.h
 * @brief Class implementing the DetectorDataService
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORSERVICEIMPL_H_
#define DETDESCR_DETECTORSERVICEIMPL_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"
#include "DetDescr/DetectorDataService.h"

#include <map>
#include <fstream>
#include <iostream>

namespace ldmx {

    /**
     * @class DetectorElementCache
     * @brief Map of IDs and geometry nodes to DetectorElement objects
     */
    class DetectorElementCache {

        public:

            void set(int id, DetectorElement* de) {
                if (get(id)) {
                    // FIXME: Should use base Exception class here.
                    std::cerr << "ERROR: The id for '" << de->getName() << "' is already assigned to '" << this->get(id)->getName() << "'." << std::endl;
                    throw std::runtime_error("The id is already used in the DE cache.");
                }
                idMap_[id] = de;
            }

            void set(TGeoNode* node, DetectorElement* de) {
                if (get(node)) {
                    // FIXME: Should use base Exception class here.
                    throw std::runtime_error("The node is already used in the DE cache.");
                }
                nodeMap_[node] = de;
            }

            DetectorElement* get(int id) {
                if (idMap_.find(id) != idMap_.end()) {
                    return idMap_[id];
                } else {
                    return nullptr;
                }
            }

            DetectorElement* get(TGeoNode* node) {
                if (nodeMap_.find(node) != nodeMap_.end()) {
                    return nodeMap_[node];
                } else {
                    return nullptr;
                }
            }

            bool contains(TGeoNode* node) {
                return nodeMap_.find(node) != nodeMap_.end();
            }

            bool contains(int id) {
                return idMap_.find(id) != idMap_.end();
            }

        private:
            std::map<int, DetectorElement*> idMap_;
            std::map<TGeoNode*, DetectorElement*> nodeMap_;
    };

    /**
     * @class DetectorDataServiceImpl
     * @brief Implements the DetectorDataService for accessing detector data at runtime
     */
    class DetectorDataServiceImpl : public DetectorDataService {

        public:

            /**
             * Class destructor.
             *
             * @note
             * Deletes the DetectorElement hierarchy and the ROOT geometry manager.
             */
            virtual ~DetectorDataServiceImpl() {
                delete deTop_;
                delete geoManager_;
            }

            /**
             * Class constructor.
             *
             * @note
             * Builds the cache of names to local aliases, which are file system paths of
             * full detector GDML files.
             */
            DetectorDataServiceImpl() {
                fac_ = DetectorElementFactory::instance();
                setupLocalAliases();
            }

            /**
             * Get the TGeoManager that was setup when loading the detector.
             * This will return null until initialize() is called.
             * @return The current geometry manager or null if not setup.
             */
            TGeoManager* getGeometryManager() {
                return geoManager_;
            }

            /**
             * Get the top DetectorElement, pointing to the "world volume."
             */
            DetectorElement* getTopDetectorElement() {
                return deTop_;
            }

            /**
             * Initialize the ROOT geometry system from the current detector name.
             * This will load the GDML geometry and setup the DetectorElement
             * hierarchy.
             */
            void initialize();

            /**
             * Set the name of the detector to be loaded when initialize() is called.
             * If the detector name is never set then an error will be thrown
             * during initialization.
             * @param detectorName The name of the detector.
             */
            void setDetectorName(std::string detectorName) {
                detectorName_ = detectorName;
            }

            /**
             * Get the name of the current detector.
             * @return The name of the current detector.
             */
            virtual const std::string& getDetectorName() {
                return detectorName_;
            }

            /**
             * Add an alias mapping a detector name to a physical location
             * such as a filesystem path.
             * @param detectorName The name of the detector.
             * @param alias The alias of the detector (i.e. file path).
             * @throw runtime_error If the alias is not a valid file path.
             */
            void addAlias(std::string detectorName, std::string alias) {
                std::ifstream f(alias.c_str());
                if (f.good()) {
                    aliasMap_[detectorName] = alias;
                } else {
                    throw std::runtime_error("The alias is not a valid file: " + alias);
                }
                //std::cout << "[ DetectorDataServiceImpl ] : Added alias " << detectorName << " => " << alias << std::endl;
            }

            /**
             * Get a DetectorElement by its ID.
             * @return The DetectorElement with the id or nullptr if none exists.
             */
            DetectorElement* getDetectorElement(int id) {
                return deCache_.get(id);
            }

            /**
             * Get a DetectorElement by its assigned node.
             * @return The DetectorElement with the matching node or null if none exists.
             *
             * @note If the node is not explicitly assigned to a DetectorElement, a search
             * will be performed up the geometry hierarchy until a DetectorElement is
             * found that maps to a parent node or the top is reached.
             */
            DetectorElement* findDetectorElement(TGeoNode* node);

            /**
             * Locate a DetectorElement leaf from a global position.
             * @return The DetectorElement containing the position.
             */
            DetectorElement* locateDetectorElement(std::vector<double>& globalPosition);

            /**
             * Get the detector aliases mapping names to files.
             * @return The detector alias map.
             */
            const DetectorAliasMap& getDetectorAliases() {
                return aliasMap_;
            }

        private:

            /**
             * Build the DetectorElement hierarchy.
             */
            void buildDetectorElements();

            /**
             * Build a cache of detector aliases from names to their locations on
             * disk, using the LDMXSW_DIR environment variable to locate the main
             * detector data directory in the installation area.
             */
            void setupLocalAliases();

        private:

            /** Name of the current detector. */
            std::string detectorName_;

            /** Map of names to detector locations (GDML files). */
            DetectorDataService::DetectorAliasMap aliasMap_;

            /** The currently active geometry manager. */
            TGeoManager* geoManager_{nullptr};

            /** The top DetectorElement providing access to the hierarchy. */
            DetectorElementImpl* deTop_{nullptr};

            /** Map of IDs and nodes to DetectorElement objects. */
            DetectorElementCache deCache_;

            DetectorElementFactory* fac_;
    };

    /**
     * @class GlobalPositionCacheBuilder
     * @brief Caches the global positions of every DetectorElement in the hierarchy
     */
    class GlobalPositionCacheBuilder : public DetectorElementVisitor {

        public:

            /**
             * Class constructor.
             * @param nav The TGeoNavigator for navigating the detector geometry.
             */
            GlobalPositionCacheBuilder(TGeoNavigator* nav) : nav_(nav) {;}

            /**
             * Visit the given DetectorElement and set its global position.
             * @param de The DetectorElement to visit.
             */
            void visit(DetectorElement* de);

        private:

            /** The geometry navigator. */
            TGeoNavigator* nav_;
    };

    /**
     * @class DetectorElementCacheBuilder
     * @brief Sets the mapping of IDs and nodes to DetectorElement objects
     */
    class DetectorElementCacheBuilder : public DetectorElementVisitor {

        public:

            /**
             * Class constructor.
             * @param cache The DetectorElementCache to update.
             */
            DetectorElementCacheBuilder(DetectorElementCache* cache) : cache_(cache) {
            }

            /**
             * Visit the given DetectorElement and cache the lookup to its ID and node.
             * @param de The DetectorElement to visit.
             */
            void visit(DetectorElement* de) {
                if (de->getSupport()) {
                    cache_->set(de->getSupport(), de);
                }
                if (de->getID()) {
                    if (!leafOnly_ || de->getChildren().size() == 0) {
                        std::cout << "Caching ID <" << de->getID() << "> for '" << de->getName() << "'." << std::endl;
                        cache_->set(de->getID(), de);
                    }
                }
            }

            /**
             * Set flag for caching IDs only for leaf volumes (e.g. to avoid duplication).
             * @param leafOnly True to cache only leaf nodes in the geometry.
             */
            void setLeafOnly(bool leafOnly) {
                leafOnly_ = leafOnly;
            }

        private:

            /** Pointer to cache that will be updated. */
            DetectorElementCache* cache_;

            /** Flag for caching IDs only for leaf nodes. */
            bool leafOnly_{false};
    };
}

#endif /* DETDESCR_DETECTORSERVICEIMPL_H_ */
