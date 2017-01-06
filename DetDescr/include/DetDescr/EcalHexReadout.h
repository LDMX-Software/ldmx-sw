/**
 * @file EcalHexReadout.h
 * @brief Class that translates raw positions of ECal hits into cells in a hexagonal readout
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

/**
 * @class EcalHexReadout
 * @brief Implementation of ECal hexagonal cell readout
 *
 * @note
 * This class implements a hexagonal cell readout across all modules in the ECal.
 * It can provide a cell ID given a position and can also translate a cell ID back
 * into a position of the cell centroid.
 */
class EcalHexReadout {

    public:

        /**
         * Class constructor.
         * @param width The width of the grid [mm].
         * @param side The length of the hexagon's side [mm].
         */
        EcalHexReadout(const double width = 1000, const double side = 4.59360);

        /**
         * Class destructor.
         */
        virtual ~EcalHexReadout() {
            delete ecalMap;
        }

        /**
         * Get a cell ID from an XY position.
         * @param x The X position.
         * @param y The Y position.
         */
        inline int getCellId(float x, float y) {
            return ecalMap->FindBin(x, y);
        }

        /**
         * Get an XY position from a cell ID.
         * @param cellID The cell ID.
         * @return The XY position of the cell.
         */
        inline std::pair<float, float> getCellCentroidXYPair(int cellId) {
            std::pair<std::map<int, XYCoords>::iterator, bool> isInserted;
            isInserted = cellIdtoCoords.insert(std::pair<int,XYCoords>(cellId,std::pair<float,float>(0, 0)));
            if (isInserted.second == true) {
                throw std::runtime_error("Error: cell " + std::to_string(cellId) + " is not valid");
            }
            return isInserted.first->second;
        }

        /**
         * @todo Document this function.
         */
        inline bool isInShowerInnerRing(int centroidId, int cellId) {
            bool matched = false;
            for (auto ringId : getInnerRingCellIds(centroidId)) {
                if (cellId == ringId)
                    matched = true;
            }
            return matched;
        }

        /**
         * @todo Document this function.
         */
        inline bool isInShowerOuterRing(int centroidId, int cellId) {
            bool matched = false;
            for (auto ringId : getOuterRingCellIds(centroidId)) {
                if (cellId == ringId)
                    matched = true;
            }
            return matched;
        }

    private:

        /**
         * @todo Document this function.
         */
        inline std::vector<int> getInnerRingCellIds(int cellId) {
            return {cellId - 1, cellId - 1 - 76, cellId - 1 + 77,
                cellId + 1, cellId + 1 - 77, cellId + 1 + 76};
        }

        /**
         * @todo Document this function.
         */
        inline std::vector<int> getOuterRingCellIds(int cellId) {
            return {cellId - 2 * 76, cellId - 2 * 76- 1, cellId - 2 * 76 - 2,
                cellId + 2 * 78, cellId + 2 * 78 - 1, cellId + 2 * 78 - 2,
                cellId + 2, cellId + 2 - 77, cellId + 2 + 76,
                cellId - 2, cellId - 2 - 76, cellId - 2 + 77};
        }

        /**
         * @todo Document this function.
         */
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
