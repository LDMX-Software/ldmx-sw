#ifndef DETDESCR_DETECTORIDSTORE_H_
#define DETDESCR_DETECTORIDSTORE_H_ 1

// LDMX
#include "DetDescr/DetectorId.h"

class DetectorIdStore {

    public:

        typedef std::map<std::string, DetectorId*> DetectorIdMap;

        static DetectorIdStore* getInstance() {
            static DetectorIdStore INSTANCE;
            return &INSTANCE;
        }

        DetectorId* getId(const std::string& name) {
            return ids[name];
        }

        void addId(const std::string& name, DetectorId* id) {
            ids[name] = id;
        }

    private:

        DetectorIdMap ids;
};

#endif
