/**
 * @file EcalHexReadout.h
 * @brief Class that translates raw positions of ECal module hits into cells in a hexagonal readout
 * @author Owen Colegro, UCSB
 *  past code sources:
 *           https://cms-hgcal.github.io/TestBeam/HGCSSGeometryConversion_8cc_source.html
 *           modified TH2Poly::Honeycomb routine
 * @author Patterson, UCSB
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef DETDESCR_ECALHEXREADOUT_H_
#define DETDESCR_ECALHEXREADOUT_H_

// LDMX
#include "Framework/Exception.h"
#include "DetDescr/EcalDetectorID.h"

// STL
#include <map>

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
             * @param gap air gap between edges of adjacent ECal modules [mm]
             * @param nCellsWide Total cell count in center horizontal row.
             */
            EcalHexReadout(double moduleMinR, double gap, unsigned nCellsWide,
                    const std::vector<double> &layerZPositions, double ecalFrontZ);

            /**
             * Class destructor.
             */
            virtual ~EcalHexReadout() { }

            /**
             * Get entire real space position for the cell with the input raw ID
             *
             * Inputs x,y,z will be set to the calculated position
             */
            void getCellAbsolutePosition( int rawID , double &x, double &y, double &z ) const {
                
                EcalDetectorID tmpID;
                tmpID.setRawValue( rawID );
                tmpID.unpack();

                int layerID  = tmpID.getLayerID();
                int moduleID = tmpID.getFieldValue( "module_position" );
                int cellID   = tmpID.getCellID();

                XYCoords xy = this->getCellCenterAbsolute( this->combineID( cellID , moduleID ) );
                x = xy.first;
                y = xy.second;
                z = this->getZPosition( layerID );

                return;
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
             * Get the z position from the layer index
             */
            double getZPosition(int layerID) const {
                return ecalFrontZ_ + layerZPositions_.at(layerID);
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
                int bin = ecalMap_.FindBin(x,y)-1; // NB FindBin indices starts from 1, our maps start from 0
                if(bin < 0) {
                    TString error_msg = TString("[EcalHexReadout::getCellIDRelative] Relative coordinates are outside module hexagon!") + 
                                        TString::Format(" Is the gap used by EcalHexReadout (%.2f mm) and the minimum module radius (%.2f mm)",gap_,moduler_) +
                                        TString::Format(" the same as hexagon_gap and Hex_radius in ecal.gdml? Received (x,y) = (%.2f,%.2f).",x,y);
                    EXCEPTION_RAISE( "InvalidArg" , error_msg.Data() );
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
                    EXCEPTION_RAISE( "InvalidCellID" , "Cell " + std::to_string(cellID) + " is not valid." );
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

            /**
             * Get a reference to the TH2Poly used for Cell IDs.
             *
             * @note This is only helpful in the use case
             * where you want to print the cell ID <-> cell position
             * map. DO NOT USE THIS OTHERWISE.
             *
             * @return pointer to member variable ecalMap_
             */
            TH2Poly* getCellPolyMap() const { return &ecalMap_; }

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

            /**
             * Constructs list of trigger groups.
             *
             * The list of trigger groups uses the index as the cell ID and the value as the ID of
             * the trigger group.
             *
             * For the ECal, cells are grouped into 3x3 "squares" and integrated over for the trigger.
             * This function builds a list for finding which trigger group a cell is in.
             */
            void buildTriggerGroup();

            /** 
             * MUST SYNC MINR AND GAP WITH ECAL.GDML. May change cell count here for eg granularity studies.
             * maxR = center-to-corner module hexagon radius, i.e. currently "Hex_radius" in gdml
             * nCellsWide = count of cells in neatly-ordered horizontal center row
             * Cell count calculation:
             *   N = total cell count (in each module)
             *   c = nCellsWide as defined below
             *   Define n through c = 2*n+1
             *   Then N = 1 + 3n(n+1).
             *   E.g. c = 23 gives N = 397.
             */
            unsigned nCellsWide_{0};
            double lengthWide_{0};
            double gap_{1};

            /// Center-to-Corner Radius of cell hexagon [mm]
            double cellr_{0};

            /// Center-to-Corner Radius of module hexagon [mm]
            double moduler_{0};

            /// Center-to-Flat-Side Radius of cell hexagon [mm]
            double cellR_{0};

            /// Center-to-Flat-Side Radius of module hexagon [mm]
            double moduleR_{0};

            /// Number of cells across the center line of a module (must be odd)
            unsigned nCellsWide_{0};

            /// Width of center line across module [mm]
            double lengthWide_{0};

            /// Separation between columns of cells in a module [mm]
            double columnDistance_{0};

            /// Separation between rows of cells in a module [mm]
            double rowDistance_{0};

            /// Front of ECal relative to world geometry [mm]
            double ecalFrontZ_{0};

            /// The layer Z postions are with respect to the front of the ECal [mm]
            std::vector<double> layerZPositions_;

            /// Postion of module centers relative to world geometry (uses module ID as key)
            std::map<int, XYCoords> modulePositionMap_;

            /// Position of cell centers relative to module (uses cell ID as key)
            std::map<int, XYCoords> cellPositionMap_;

            /// Position of cell centers relative to world geometry (uses combined cell and module ID as key)
            std::map<int, XYCoords> cellModulePositionMap_;

            /// Map of cell ID to neighboring cells
            std::map<int, std::vector<int> > NNMap_;

            /// Map of cell ID to neighbors of neighbor cells
            std::map<int, std::vector<int> > NNNMap_;

            /// List of Trigger Group IDs (index is cell ID)
            std::vector<int> triggerGroups_;

            /**
             * Honeycomb Binning from ROOT
             *
             * Needs to be mutable because ROOT doesn't have good const handling
             */
            mutable TH2Poly ecalMap_;
    };

}

#endif
