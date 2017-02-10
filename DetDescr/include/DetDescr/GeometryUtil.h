/*
 * GeometryUtil.h
 * @brief Miscellaneous static geometry utility functions
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_GEOMETRYUTIL_H_
#define DETDESCR_GEOMETRYUTIL_H_

#include "TGeoNode.h"

namespace ldmx {

    class GeometryUtil {

        private:

            GeometryUtil() {;}

        public:

            /**
             * Find the first daughter which has a name that starts with the prefix.
             * @param prefix The name prefix.
             * @param node The geometry node.
             * @return The matching node which has a name that starts with the prefix.
             */
            static TGeoNode* findFirstDauNameStartsWith(std::string prefix, TGeoNode* node) {
                int nDau = node->GetNdaughters();
                TGeoNode* dau = nullptr;
                for (int iDau = 0; iDau < nDau; iDau++) {
                    std::string dauName = node->GetDaughter(iDau)->GetName();
                    if (dauName.compare(0, prefix.length(), prefix) == 0) {
                        dau = node->GetDaughter(iDau);
                        break;
                    }
                }
                return dau;
            }

            /**
             * Get a list of all daughter nodes that start with the prefix.
             * @param prefix The name prefix.
             * @param node The geometry node.
             * @return A list of matching nodes which start with the prefix.
             */
            static std::vector<TGeoNode*> findDauNameStartsWith(std::string prefix, TGeoNode* node) {
                std::vector<TGeoNode*> dauVec;
                TGeoNode* dau = nullptr;
                int nDau = node->GetNdaughters();
                for (int iDau = 0; iDau < nDau; iDau++) {
                    std::string dauName = node->GetDaughter(iDau)->GetName();
                    if (dauName.compare(0, prefix.length(), prefix) == 0) {
                        dauVec.push_back(node->GetDaughter(iDau));
                    }
                }
                return dauVec;
            }

    };

}

#endif /* DETDESCR_GEOMETRYUTIL_H_ */
