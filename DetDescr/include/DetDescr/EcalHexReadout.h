/**
 * @file EcalHexReadout.h
 * @brief Translates raw positions of ECal hits into cells in a hexagonal readout.
 * @author Owen Colegro, UCSB
 */

#ifndef DETDESCR_ECALHEXREADOUT_H_
#define DETDESCR_ECALHEXREADOUT_H_

// STL
#include <map>
#include <iostream>

// ROOT
#include "TH2Poly.h"

namespace detdescr {

class EcalHexReadout {

    public:

        EcalHexReadout(const double width = 1000, const double side = 4.59360);

        virtual ~EcalHexReadout() {
            delete ecalMap;
        }

        inline int getCellId(float x, float y) {
            return ecalMap->FindBin(x, y);
        }

        inline std::pair<float, float> getCellCentroidXYPair(int cellId) {
            std::pair<std::map<int, XYCoords>::iterator, bool> isInserted;
            isInserted = cellIdtoCoords.insert(std::pair<int,XYCoords>(cellId,std::pair<float,float>(0, 0)));
            if (isInserted.second == true) {
                throw std::runtime_error("Error: cell " + std::to_string(cellId) + " is not valid");
            }
            return isInserted.first->second;
        }

        inline bool isInShowerInnerRing(int centroidId, int cellId) {
            bool matched = false;
            for (auto ringId : getInnerRingCellIds(centroidId)) {
                if (cellId == ringId)
                    matched = true;
            }
            return matched;
        }

        inline bool isInShowerOuterRing(int centroidId, int cellId) {
            bool matched = false;
            for (auto ringId : getOuterRingCellIds(centroidId)) {
                if (cellId == ringId)
                    matched = true;
            }
            return matched;
        }

    private:

        inline std::vector<int> getInnerRingCellIds(int cellId) {
            return {cellId - 1, cellId - 1 - 76, cellId - 1 + 77,
                cellId + 1, cellId + 1 - 77, cellId + 1 + 76};
        }

        inline std::vector<int> getOuterRingCellIds(int cellId) {
            return {cellId - 2 * 76, cellId - 2 * 76- 1, cellId - 2 * 76 - 2,
                cellId + 2 * 78, cellId + 2 * 78 - 1, cellId + 2 * 78 - 2,
                cellId + 2, cellId + 2 - 77, cellId + 2 + 76,
                cellId - 2, cellId - 2 - 76, cellId - 2 + 77};
        }

        void buildMap(Double_t xstart,
                Double_t ystart, /* Map starting points */
                Double_t a,  /* side length */
                Int_t k,     /* # hexagons in a column */
                Int_t s);

    private:
        TH2Poly* ecalMap;
        typedef std::pair<float,float> XYCoords;
        std::map<int, XYCoords> cellIdtoCoords;
};

}

#endif
