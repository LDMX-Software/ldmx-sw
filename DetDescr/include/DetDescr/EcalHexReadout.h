/**
 * @file EcalHexReadout.h
 * @brief Class that translates raw positions of ECal module hits into cells in a hexagonal readout
 * @author Owen Colegro, UCSB
 *  past code sources:
 *           https://cms-hgcal.github.io/TestBeam/HGCSSGeometryConversion_8cc_source.html
 *           modified TH2Poly::Honeycomb routine
 * @author Patterson, UCSB
 */

#ifndef DETDESCR_ECALHEXREADOUT_H_
#define DETDESCR_ECALHEXREADOUT_H_

// STL
#include <map>
#include <iostream>

// ROOT
#include "TH2Poly.h"

namespace ldmx {

    typedef std::pair<double,double> XYCoords;

    /**
     * @class EcalHexReadout
     * @brief Implementation of ECal hexagonal cell readout
     *
     * @note
     * This class defines an integer ID for each cell in a module, convertible with 2D position.
     * 
     */
    class EcalHexReadout {

        public:

            /**
             * Class constructor.
             * @param moduleMinR The center-to-flat radius of an ECal module [mm]. See comments in src.
             * @param nCellsWide Total cell count in center horizontal row.
             */
            EcalHexReadout(double moduleMinR = defaultMinR, double gap = defaultGap_, unsigned nCellsWide = defaultNCellsWide);

            /**
             * Class destructor.
             */
            virtual ~EcalHexReadout() {
                delete ecalMap_;
            }

            /**
             * Combine cell and module IDs into a per-layer ID
             */
            int combineID(int cellID, int moduleID) const {
              return 10*cellID+moduleID;
            }

            /**
             * Separate cell and module IDs from a combined cellModuleID
             */
            std::pair<int,int> separateID(int cellModuleID) const {
              return std::pair<int,int>(cellModuleID/10, cellModuleID % 10);
            }

            /**
             * Get a module center position relative to the ecal center [mm]
             */
            XYCoords getModuleCenter(int moduleID) const {
                return modulePositionMap_.at(moduleID);
            }

            /**
             * Get a module ID from an XY position relative to the ecal center [mm]
             */
            int getModuleID(double x, double y) const {
                int bestID = -1;
                double bestDist = 1E6;
                for(auto const& module : modulePositionMap_) {  
                    int mID   = module.first;
                    double mX = module.second.first;
                    double mY = module.second.second;
                    double dist = sqrt( (x-mX)*(x-mX) + (y-mY)*(y-mY) );
                    if(dist < moduler_) return mID;
                    if(dist < bestDist) { bestID = mID; bestDist = dist; }
                }
                return bestID;
            }

            /**
             * Get a cell ID from an XY position relative to module center. 
             * This is where invalid (x,y) from external calls will end up failing and need error handling.
             * @param x Any X position [mm]
             * @param y Any Y position [mm]
             */
            int getCellIDRelative(double x, double y) const {
                if(!isInside(x/moduleR_, y/moduleR_)) return -1; // protection for (REAL) cases where ecalMap_ may have parts of bins outside technical geometry
                int bin = ecalMap_->FindBin(x,y)-1; // NB FindBin indices starts from 1, our maps start from 0
                if(bin < 0) {
                    TString error_msg = TString("[EcalHexReadout::getCellIDRelative] Relative coordinates are outside module hexagon!") + 
                                        TString::Format(" Is the gap used by EcalHexReadout (%.2f mm) and the minimum module radius (%.2f mm)",gap_,moduler_) +
                                        TString::Format(" the same as hexagon_gap and Hex_radius in ecal.gdml? Received (x,y) = (%.2f,%.2f).",x,y);
                    throw std::invalid_argument(error_msg.Data());
                }
                return bin;
            }

            /**
             * Get a combined cellModule ID from an XY position relative to ecal center.
             * Error is ID < 0 (see TH2Poly for meanings).
             * @param x Any X position [mm]
             * @param y Any Y position [mm]
             * @param moduleID The module copy number (0 through 6)
             */
            int getCellModuleID(double x, double y) const {
                int moduleID = getModuleID(x,y);
                double relX = x - getModuleCenter(moduleID).first;
                double relY = y - getModuleCenter(moduleID).second;
                int cellID = getCellIDRelative(relX,relY);
                if(cellID < 0){
                    std::cout << TString::Format("[EcalHexReadout::getCellModuleID] Someone gave me an out-of-bounds point (%f,%f)",x,y) << std::endl;
                    return -1;
                }
                int cellModuleID = combineID(cellID,moduleID);
                return cellModuleID;
            }

            /**
             * Get a cell center XY position relative to module center from a cell ID.
             * @param cellID The cell ID.
             * @return The XY position of the center of the cell. Error is exception.
             */
            XYCoords getCellCenterRelative(int cellID) const {
                // this map search is probably just as fine as the TList search for the cell in ecalMap.
                //   wonder why TH2Poly->GetBin(ID) doesn't exist. plus the map is useful by itself.
                auto search = cellPositionMap_.find(cellID);
                if(search == cellPositionMap_.end()) {
                    throw std::runtime_error("Error: cell " + std::to_string(cellID) + " is not valid");
                }
                return search->second;
            }

            /**
             * Get a cell center XY position relative to ecal center from a combined cellModuleID.
             * @param cellModuleID The combined cellModuleID.
             * @return The XY position of the center of the cell. Error is exception.
             */
            XYCoords getCellCenterAbsolute(int cellModuleID) const {
                return cellModulePositionMap_.at(cellModuleID);
            }

            /**
             * @param Return NN IDs, which are combined cellModuleIDs. Normally six. Return by copy.
             *   NB cellModuleIDs are: 10*cellID+moduleID
             */
            std::vector<int> getNN(int cellModuleID) const {
              return NNMap_.at(cellModuleID);
            }

            /**
             * @param Return NNN IDs, which are cellModuleIDs. Normally twelve. Return by copy.
             *   NB cellModuleIDs are: 10*cellID+moduleID
             */
            std::vector<int> getNNN(int cellModuleID) const {
              return NNNMap_.at(cellModuleID);
            }

            /**
             * @param Is probeID in the NN (doesn't include centerID) list of centerID.
             *   NB cellModuleIDs are: 10*cellID+moduleID
             */
            bool isNN(int centerID, int probeID) const {
                for (auto ID: getNN(centerID)) {
                    if (ID == probeID) return true;
                }
                return false;
            }

            /**
             * @param Is probeID in the NNN list (doesn't include NN) of centerID
             *   NB cellModuleIDs are: 10*cellID+moduleID
             */
            bool isNNN(int centerID, int probeID) const {
                for (auto ID: getNNN(centerID)) {
                    if (ID == probeID) return true;
                }
                return false;
            }

            /**
             * Distance to module edge, and whether cell is on edge of module.
             * Use getNN()/getNNN() + isEdgeCell() to expand functionality.
             */
            double distanceToEdge(int cellModuleID) const;
            bool isEdgeCell(int cellModuleID) const {
                return (distanceToEdge(cellModuleID) < cellR_);
            }

            /**
             * Return entire cellID - cell center position map with read access
             */
            const std::map<int, XYCoords>& getCellPositionMap() const { return cellPositionMap_; }

            /**
             * Return entire moduleID - module center position map with read access
             */
            const std::map<int, XYCoords>& getModulePositionMap() const { return modulePositionMap_; }

            /**
             * Return entire cellModuleID - cell center position map with read access
             *   NB cellModuleIDs are: 10*cellID+moduleID
             */
            const std::map<int, XYCoords>& getCellModulePositionMap() const { return cellModulePositionMap_; }

            /**
             * Returns cell min and max radii
             */
            std::vector<double> getCellMinMaxRadii() const { return {cellr_,cellR_}; }

            /**
             * Returns module min and max radii
             */
            std::vector<double> getModuleMinMaxRadii() const { return {moduler_,moduleR_}; }

            /**
             * Stand-alone. Determines if point (x,y), already normed to max hexagon radius, lies
             * within a hexagon. Corners are (1,0) and (0.5,sqrt(3)/2). Uses "<", not "<=".
             */
            bool isInside(double normX, double normY) const;

        private:

            /**
             * Constructs the positions of the seven modules (moduleID) relative to the ecal center
             */
            void buildModuleMap();

            /**
             * Constructs the flat-bottomed hexagonal grid (cellID) of corner-down hexagonal cells. This
             * results in the center horizontal row being neatly ordered, sharing vertical edges,
             * and being the longest axis of the grid.
             * @param lengthWide Total width of center horizontal row.
             * @param nCellsWide Total cell count in center horizontal row.
             */
            void buildCellMap();

            /**
             * Constructs the positions of all the cells in a layer (cellModuleID) relative to the ecal center
             */
            void buildCellModuleMap();

            /**
             * Construts NNMap and NNNMap
             */
            void buildNeighborMaps();

            int verbose_{0}; // 0 to 3

            unsigned nCellsWide_{0};
            double lengthWide_{0};
            double gap_{0};
            double cellr_{0};
            double moduler_{0};
            double cellR_{0};
            double moduleR_{0};
            double columnDistance_{0};
            double rowDistance_{0};

            std::map<int, XYCoords> modulePositionMap_;
            std::map<int, XYCoords> cellPositionMap_;
            std::map<int, XYCoords> cellModulePositionMap_;
            std::map<int, std::vector<int> > NNMap_;
            std::map<int, std::vector<int> > NNNMap_;

            /** 
             * MUST SYNC MINR AND GAP WITH ECAL.GDML. May change cell count here for eg granularity studies.
             * minR = center-to-flat module hexagon radius, i.e. currently "Hex_radius" in gdml
             * nCellsWide = count of cells in neatly-ordered horizontal center row
             * Cell count calculation:
             *   N = total cell count (in each module)
             *   c = nCellsWide as defined below
             *   Define n through c = 2*n+1
             *   Then N = 1 + 3n(n+1).
             *   E.g. c = 23 gives N = 397.
             */
            static constexpr double defaultMinR{85.};
            static constexpr double defaultGap_{0.};
            static constexpr unsigned defaultNCellsWide{23};

            TH2Poly* ecalMap_;
            TH2Poly* gridMap_;
    };

}

#endif
