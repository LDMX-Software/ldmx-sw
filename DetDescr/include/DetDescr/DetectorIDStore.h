#ifndef DETDESCR_DETECTORIDSTORE_H_
#define DETDESCR_DETECTORIDSTORE_H_

// LDMX
#include "DetectorID.h"

namespace detdescr {

class DetectorIDStore {

    public:

        typedef std::map<std::string, DetectorID*> DetectorIDMap;

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

        DetectorIDMap ids;
};

}

#endif
