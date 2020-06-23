/**
 * @file VisAttributesStore.h
 * @brief Class that provides a global visualization attributes store
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_VISATTRIBUTESSTORE_H_
#define SIMAPPLICATION_VISATTRIBUTESSTORE_H_

// Geant4
#include "G4VisAttributes.hh"

namespace ldmx {

    /**
     * @class VisAttributesStore
     * @brief Global store of <i>G4VisAttributes</i> created from GDML data
     */
    class VisAttributesStore {

        public:

            /**
             * Map of name to vis attributes.
             */
            typedef std::map<std::string, G4VisAttributes*> VisAttributesMap;

            /**
             * Get the global instance of the store.
             * @return The vis attributes store.
             */
            static VisAttributesStore* getInstance() {
                static VisAttributesStore INSTANCE;
                return &INSTANCE;
            }

            
            /**
             * Destructor
             *
             * Cleans up G4VisAttributes
             */
            ~VisAttributesStore() {
                std::cout << "~VisAttributesStore" << std::endl;
                for ( auto &nameAtt : visAttributesMap_ ) {
                    delete nameAtt.second;
                }
                visAttributesMap_.clear();
            }

            /**
             * Get vis attributes by name.
             * @param name The name of the vis attributes.
             * @return The vis attributes or <i>nullptr</i> if does not exist.
             */
            G4VisAttributes* getVisAttributes(const std::string& name) {
                return visAttributesMap_.at(name);
            }

            /**
             * Register a vis attributes by name.
             * @param name The name of the vis attributes.
             * @param visAttributes The vis attributes to register.
             */
            void addVisAttributes(const std::string& name, G4VisAttributes* visAttributes) {
                visAttributesMap_[name] = visAttributes;
            }

        private:

            /**
             * The map of names to vis attributes.
             */
            VisAttributesMap visAttributesMap_;
    };

}

#endif
