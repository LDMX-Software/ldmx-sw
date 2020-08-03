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
#include "DetDescr/EcalID.h"

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
    ~EcalHexReadout() { }
    
    /**
     * Get entire real space position for the cell with the input raw ID
     *
     * Inputs x,y,z will be set to the calculated position
     */
    void getCellAbsolutePosition(EcalID ID , double &x, double &y, double &z ) const {
        
        XYCoords xy = this->getCellCenterAbsolute( EcalID(0,ID.module(),ID.cell()) ); 
        x = xy.first;
        y = xy.second;
        z = this->getZPosition( ID.layer() );
        
        return;
    }

        public:

            /**
             * Class constructor.
             * @param moduleMinR The center-to-flat radius of an ECal module [mm]. See comments in src.
             * @param gap air gap between edges of adjacent ECal modules [mm]
             * @param nCellRHeight Total cell radius count across height of module
             */
            EcalHexReadout(double moduleMinR, double gap, unsigned nCellRHeight,
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
    EcalID getCellModuleID(double x, double y) const {
        int moduleID = getModuleID(x,y);
        double relX = x - modulePositionMap_.at(moduleID).first;
        double relY = y - modulePositionMap_.at(moduleID).second;
        int cellID = getCellIDRelative(relX,relY);
        return EcalID(0,moduleID,cellID);
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
    XYCoords getCellCenterAbsolute(EcalID cellModuleID) const {
        return cellModulePositionMap_.at(EcalID(0,cellModuleID.module(),cellModuleID.cell()));
    }
        
    /**
     * @param Return NN IDs, which are combined cellModuleIDs. Normally six. Return by copy.
     */
    std::vector<EcalID> getNN(EcalID cellModuleID) const {
        return NNMap_.at(EcalID(0,cellModuleID.module(),cellModuleID.cell()));
    }

    /**
     * @param Return NNN IDs, which are cellModuleIDs. Normally twelve. Return by copy.
     */
    std::vector<EcalID> getNNN(EcalID cellModuleID) const {
        return NNNMap_.at(EcalID(0,cellModuleID.module(),cellModuleID.cell()));
    }

    /**
     * @param Is probeID in the NN (doesn't include centerID) list of centerID.
     */
    bool isNN(EcalID centerID, EcalID probeID) const {
        EcalID flatCenter(0,centerID.module(),centerID.cell());
        EcalID flatProbe(0,probeID.module(),probeID.cell());
        for (auto ID: getNN(flatCenter)) {
        if (ID == flatProbe) return true;
        }
        return false;
    }

    /**
     * @param Is probeID in the NNN list (doesn't include NN) of centerID
     */
    bool isNNN(EcalID centerID, EcalID probeID) const {
        EcalID flatCenter(0,centerID.module(),centerID.cell());
        EcalID flatProbe(0,probeID.module(),probeID.cell());
        for (auto ID: getNNN(flatCenter)) {
        if (ID == flatProbe) return true;
        }
        return false;
    }
    
    /**
     * Distance to module edge, and whether cell is on edge of module.
     * Use getNN()/getNNN() + isEdgeCell() to expand functionality.
     */
    double distanceToEdge(EcalID id) const;
    bool isEdgeCell(EcalID cellModuleID) const {
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
     */
    const std::map<EcalID, XYCoords>& getCellModulePositionMap() const { return cellModulePositionMap_; }

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
             *
             * Sets modulePositionMap_ using the module IDs for keys and the centers of the module
             * hexagons for values.
             *
             * The module IDs are set in the ecal.gdml file and are
             *  - 0 for center module
             *  - 1 on top (12 o'clock)
             *  - clockwise till 6 at 11 o'clock
             *
             * @param[in] gap separation between module flat-sides
             * @param[in] moduler center-to-flat module radius
             */
            void buildModuleMap();

            /**
             * Constructs the flat-bottomed hexagonal grid (cellID) of corner-down hexagonal cells.
             *
             * Sets ecalMap_ with the defined bins being the ecal cells in coordinates with 
             * respect to the module center. Also sets cellPostionMap_ with the keys
             * being the cell ID and the values being the position of the cell with respect
             * to the module center.
             *
             * ## Strategy
             * Use ROOT's TH2Poly::HoneyComb method to build a large hexagonal grid,
             * then copy the polygons from it which overlap with the module.
             *
             * A vertex between three cells is placed at the origin,
             * then the bottom left corner of the honeycomb and the number of x and y
             * cells across the honeycomb is calculated by continuing to decrement
             * the grid x/y point until the module center-to-flat distance is reached.
             *
             * @param cellr_ the center-to-flat cell radius 
             * @param cellR_ the center-to-corner cell radius
             * @param moduler_ the center-to-flat module radius
             */
            void buildCellMap();

            /**
             * Constructs the positions of all the cells in a layer (cellModuleID) relative to the ecal center
             *
             * This uses the modulePostionMap_ and cellPositionMap_ to calculate the center
             * of all cells relative to the ecal center.
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

            /// verbosity, not configurable but helpful if developing
            int verbose_{2};

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

            /// Gap between module flat sides [mm]
            double gap_;

            /// Center-to-Corner Radius of cell hexagon [mm]
            double cellr_{0};

            /// Center-to-Corner Radius of module hexagon [mm]
            double moduler_{0};

            /// Center-to-Flat-Side Radius of cell hexagon [mm]
            double cellR_{0};

            /// Center-to-Flat-Side Radius of module hexagon [mm]
            double moduleR_{0};

            /**
             * Number of cell center-to-corner radii from the bottom to the top of the module
             *
             * Could be fractional depending on how many fractions of a radii are spanning
             * between the center of the top/bottom cell row and the edge of the module
             */
            double nCellRHeight_{0};

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
