/**
 * @file DetectorIDStore.h
 * @brief Class that provides a global store of detector IDs
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_DETECTORIDSTORE_H_
#define DETDESCR_DETECTORIDSTORE_H_

// LDMX
#include "DetectorID.h"

namespace ldmx {

    /**
     * @class DetectorIDStore
     * @brief A global store for accessing DetectorID objects
     */
    class DetectorIDStore {

        public:

            /**
             * Type mapping detector IDs to their names.
             */
            typedef std::map<std::string, DetectorID*> DetectorIDMap;

            /**
             * Get the global instance of the store.
             * @return The global instance of the store.
             */
            static DetectorIDStore* getInstance() {
                static DetectorIDStore INSTANCE;
                return &INSTANCE;
            }

            /**
             * Get a detector ID by name.
             * @return The detector ID with the name or <i>nullptr</i> if does not exist.
             */
            DetectorID* getID(const std::string& name) {
                return ids[name];
            }

            /**
             * Add a detector ID by name.
             * @param name The name of the detector ID.
             * @param id The detector ID.
             */
            void addID(const std::string& name, DetectorID* id) {
                ids[name] = id;
            }

        private:

            /**
             * The map of names to detector IDs.
             */
            DetectorIDMap ids;
    };

}

#endif
