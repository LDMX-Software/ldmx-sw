#ifndef DETDESCR_DETECTORIDSTORE_H_
#define DETDESCR_DETECTORIDSTORE_H_ 1

// LDMX
#include "DetectorID.h"

class DetectorIDStore {

    public:

        typedef std::map<std::string, DetectorID*> DetectorIdMap;

        static DetectorIDStore* getInstance() {
            static DetectorIDStore INSTANCE;
            return &INSTANCE;
        }

        DetectorID* getID(const std::string& name) {
            return ids[name];
        }

        void addID(const std::string& name, DetectorID* id) {
            ids[name] = id;
        }

    private:

        DetectorIdMap ids;
};

#endif
