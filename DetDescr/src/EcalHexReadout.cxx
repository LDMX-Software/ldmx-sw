#include "DetDescr/EcalHexReadout.h"

#include "TList.h"
#include "TGeoPolygon.h"
#include "TGraph.h"
#include "TMultiGraph.h"

#include <assert.h>
#include <iostream>
#include <iomanip>

namespace ldmx {

    EcalHexReadout::EcalHexReadout(double moduleMinR, double gap, unsigned nCellRHeight,
            const std::vector<double> &layerZPositions, double ecalFrontZ) 
        : layerZPositions_(layerZPositions), ecalFrontZ_(ecalFrontZ) {

        // ORIENTATION ASSUMPTIONS:
        //   modules are oriented flat side down. cells are oriented corner side down.
        // SOME GEOMETRY:
        //   hexagons have two radii:
        //     r (half of flat-to-flat width) and R (half of corner-to-corner width).
        //     r = (sqrt(3)/2)R and s = R, where s is the length of an edge.
        //   for seven ecal modules oriented flat-side-down, maximum x and y extents are:
        //     deltaY = 6r' + 2g = 3sqrt(3)R' + 2g
        //     deltaX = 4R' + s' + 2g/cos(30 deg) = 5R' + 4g/sqrt(3)
        //     where g is uniform gap width between modules, and primed variables correspond to modules.
        // THIS GRID:
        //   column-to-column distance in a grid such as ours is 2r = sqrt(3)R.
        //   row-to-row distance is 1.5R (easy to observe that twice that distance = 3R)
        //
        // The cell radius is calculated from the total number of center-to-corner cell radii
        // that span the module height. This count can have fractional counts to account
        // for the fractions of cell radii at the module edges.

        moduleR_      = moduleMinR*(2/sqrt(3));
        moduler_      = moduleMinR;
        nCellRHeight_ = nCellRHeight;
        cellR_        = 2*moduler_/nCellRHeight;
        cellr_        = (sqrt(3.)/2.)*cellR_;
        gap_          = gap;
        if(verbose_>0){
            std::cout << std::endl << "[EcalHexReadout] Verbosity set in header to " << verbose_ << std::endl;
            std::cout << TString::Format("Building module map with gap %.2f, nCellRHeight %.2f ",gap_,nCellRHeight_) << std::endl;
            std::cout << TString::Format("  min/max radii of cell %.2f %.2f and module %.2f %.2f",cellr_,cellR_,moduler_,moduleR_) << std::endl;
        }

        buildModuleMap();
        buildCellMap();
        buildCellModuleMap();
        buildNeighborMaps();
        if(verbose_>0){ std::cout << std::endl; }
    }

    void EcalHexReadout::buildModuleMap(){
        if(verbose_>0) {
            std::cout 
                << std::endl 
                << "[buildModuleMap] Building module position map for module min r of " << moduler_
                << "    and gap of " << gap_
                << std::endl;
        }

        // module IDs are 0 for ecal center, 1 at 12 o'clock, and clockwise till 6 at 11 o'clock.
        double C_PI = 3.14159265358979323846; // or TMath::Pi(), #define, atan(), ...
        modulePositionMap_[0] = std::pair<double,double>(0.,0.);
        for(unsigned id = 1 ; id < 7 ; id++){
            double x = (2.*moduler_+gap_)*sin( (id-1)*(C_PI/3.) );
            double y = (2.*moduler_+gap_)*cos( (id-1)*(C_PI/3.) );
            modulePositionMap_[id] = std::pair<double,double>(x,y);
            if(verbose_>2) std::cout << TString::Format("   id %d is at (%.2f, %.2f)",id,x,y) << std::endl;
        }
        if(verbose_>0) std::cout << std::endl;
    }

    void EcalHexReadout::buildCellMap() {
        /** STRATEGY
         * use native ROOT HoneyComb method to build large hexagonal grid.
         * then copy from it the polygons which cover a module.
         */
        TH2Poly gridMap;

        // make hexagonal grid [boundary is rectangle] larger than the module
        double gridMinX = 0., gridMinY = 0.; //start at the origin
        int numXCells=0, numYCells=0;
        while ( gridMinX > -1*moduleR_ ) {
            gridMinX -= 2*cellr_; //decrement x by cell center-to-flat diameter
            numXCells++;
        }
        while ( gridMinY > -1*moduler_ ) {
            if (numYCells%2==0) gridMinY -= cellR_; //decrement y by cell center-to-corner diameter
            else                gridMinY -= 2*cellR_;
            numYCells++;
        }
        //only counted one half of the cells
        numXCells *= 2;
        numYCells *= 2;

        gridMap.Honeycomb(gridMinX , gridMinY, cellR_, numXCells , numYCells);

        if(verbose_>0){
            std::cout << std::endl;
            std::cout << std::setprecision(2)
                << "[buildCellMap] cell rmin: " << cellr_ << " cell rmax: " << cellR_
                << " (gridMinX,gridMinY) = (" << gridMinX << "," << gridMinY << ")"
                << " (numXCells,numYCells) = (" << numXCells << "," << numYCells << ")" << std::endl;
        }

        // copy cells lying within module boundaries to a module grid
        std::vector<int> cellIdCopied(gridMap.GetNumberOfBins());
        TListIter next(gridMap.GetBins()); // a TH2Poly is a TList of TH2PolyBin
        TH2PolyBin *polyBin = 0;
        TGraph * poly = 0; // a polygon returned by TH2Poly is a TGraph
        int ecalMapID = 0; // ecalMap cell IDs go from 0 to N-1, not equal to original grid cell ID.
        while( (polyBin = (TH2PolyBin*)next()) ){
            poly = (TGraph*)polyBin->GetPolygon();

            // decide whether to copy polygon to new map.
            // use all vertices, eg in case of cut-off edge polygons.
            int numVerticesInside = 0;
            if(verbose_>2) std::cout << "[buildCellMap] Cell vertices" << std::endl;
            double vertex_x=0, vertex_y=0;
            for(unsigned i = 0 ; i < poly->GetN() ; i++){
                poly->GetPoint(i,vertex_x,vertex_y);
                if (verbose_>2) {
                    std::cout << "     vtx # " << i << std::endl;
                    std::cout << "     vtx x,y " << vertex_x << " " << vertex_y << std::endl;
                }
                if ( isInside(vertex_x/moduleR_, vertex_y/moduleR_) ) numVerticesInside++;
            }

            bool addPoly = numVerticesInside > 1;
            if(addPoly){
                int id = polyBin->GetBinNumber();
                double x = (polyBin->GetXMax() + polyBin->GetXMin()) / 2.;
                double y = (polyBin->GetYMax() + polyBin->GetYMin()) / 2.;
                if(verbose_>1) std::cout << TString::Format("[buildCellMap] Copying poly with ID %d and (x,y) (%.2f,%.2f)", id, x, y) << std::endl;
                bool isCopied = (std::find(std::begin(cellIdCopied), std::end(cellIdCopied), id) != cellIdCopied.end());
                if(verbose_>1 && isCopied) std::cout << "    cell was used already! not copying." << std::endl;
                if(!isCopied){
                    //ecalMap_ needs to have its own copy of the polygon TGraph
                    //  otherwise, we get a seg fault when EcalHexReadout is destructed
                    //  because the polygon that was copied over from gridMap is deleted at the end of this function
                    if ( numVerticesInside < 6 ) {
                        //TODO some fanciness with the edge
                        ecalMap_.AddBin( poly->GetN() , poly->GetX() , poly->GetY() );
                    } else {
                        //all vertices inside ==> cell fully in the module
                        ecalMap_.AddBin( poly->GetN() , poly->GetX() , poly->GetY() );
                    }
                    cellPositionMap_[ecalMapID] = std::pair<double,double>(x,y);
                    ecalMapID++;
                    cellIdCopied.push_back(id);
                }
            }
        }

        if(verbose_>0) std::cout << std::endl;
        return;
    }

    void EcalHexReadout::buildCellModuleMap(){
        if(verbose_>0) std::cout << std::endl << "[buildCellModuleMap] Building cellModule position map" << std::endl;
        for(auto const& module : modulePositionMap_) {
            int moduleID = module.first;
            double moduleX = module.second.first;
            double moduleY = module.second.second;
            for(auto const& cell : cellPositionMap_) {
                int cellID = cell.first;
                double cellX = cell.second.first;
                double cellY = cell.second.second;
                double x = cellX+moduleX;
                double y = cellY+moduleY;
                int cellModuleID = combineID(cellID,moduleID);
                cellModulePositionMap_[cellModuleID] = std::pair<double,double>(x,y);
            }
        }
        if(verbose_>0) std::cout << "  contained " << cellModulePositionMap_.size() << " entries. " << std::endl;
    }

    void EcalHexReadout::buildNeighborMaps(){
        /** STRATEGY
         * Neighbors may include from other modules. All this is precomputed. So we can be wasteful here.
         * Gaps may be nonzero, so we simply apply an anulus requirement (r < point <= r+dr) using total x,y positions
         * relative to the ecal center (cell+module positions). This makes the routine portable to future cell layouts.
         * Note that the module centers already take into account a nonzero gap.
         * The number of neighbors is not simple because: edges, and that module edges have cutoff cells.
         *   (NN) Center within [1*cellr_, 3*cellr_]
         *   (NNN) Center within [3*cellr_, 4.5*cellr_]
         *   Chosen b/c in ideal case, centers are at 2*cell_ (NN), and at 3*cellR_=3.46*cellr_ and 4*cellr_ (NNN).
         */

        NNMap_.clear();
        NNNMap_.clear();
        for(auto const& centerChannel : cellModulePositionMap_) {
            EcalID centerID = centerChannel.first;
            double centerX = centerChannel.second.first;
            double centerY = centerChannel.second.second;
            for(auto const& probeChannel : cellModulePositionMap_) {
                EcalID probeID = probeChannel.first;
                double probeX = probeChannel.second.first;
                double probeY = probeChannel.second.second;
                double dist = sqrt( (probeX-centerX)*(probeX-centerX) + (probeY-centerY)*(probeY-centerY) );
                if(      dist > 1*cellr_  && dist <= 3.*cellr_)  { NNMap_[centerID].push_back(probeID); }
                else if( dist > 3.*cellr_ && dist <= 4.5*cellr_) {NNNMap_[centerID].push_back(probeID); }
            }
            if(verbose_>1) std::cout << TString::Format("Found %d NN and %d NNN for cellModuleID ",int(NNMap_[centerID].size()), int(NNNMap_[centerID].size()))
				     << centerID << TString::Format(" with x,y (%.2f,%.2f)", centerX, centerY) << std::endl;
        }
        if(verbose_>2){
            double specialX = 0.5*moduleR_ - 0.5*cellr_; // center of cell which is upper-right corner of center module
            double specialY = moduler_ - 0.5*cellR_;
	    EcalID specialCellModuleID = getCellModuleID(specialX,specialY);
            std::cout << "The neighbors of the bin in the upper-right corner of the center module, with cellModuleID " 
                      << specialCellModuleID << " include " << std::endl;
            for(auto centerNN : NNMap_.at(specialCellModuleID)){
	      std::cout << " NN " << centerNN
			<< TString::Format(" (x,y) (%.2f, %.2f)",getCellCenterAbsolute(centerNN).first,getCellCenterAbsolute(centerNN).second) << std::endl;
            }
            for(auto centerNNN : NNNMap_.at(specialCellModuleID)){
	      std::cout << " NNN " << centerNNN
			<< TString::Format(" (x,y) (%.2f, %.2f)",getCellCenterAbsolute(centerNNN).first,getCellCenterAbsolute(centerNNN).second) << std::endl;
            }
            std::cout << TString::Format("This bin is a distance of %.2f away from a module edge. Decision isEdge %d.",
                         distanceToEdge(specialCellModuleID),isEdgeCell(specialCellModuleID)) << std::endl;
        }
        if(verbose_>0) std::cout << std::endl;
        return;
    }

    double EcalHexReadout::distanceToEdge(EcalID cellModuleID) const {
        // https://math.stackexchange.com/questions/1210572/find-the-distance-to-the-edge-of-a-hexagon
	int cellID = cellModuleID.cell();
        XYCoords cellLocation = getCellCenterRelative(cellID);
        double x = fabs(cellLocation.first); // bring to first quadrant
        double y = fabs(cellLocation.second);
        double r = sqrt(x*x+y*y);
        double theta = (r > 1E-3) ? fabs(std::atan(y/x)) : 0;
        if(x < moduleR_/2.) return (moduler_ - y); // closest line is straight vertical to top edge
        double dist = sqrt(3.)*moduleR_/(std::sin(theta) + sqrt(3.)*std::cos(theta));
        return dist;
    }

    bool EcalHexReadout::isInside(double normX, double normY) const {
        if(verbose_>2) std::cout << TString::Format("[isInside] Checking if normXY=(%.2f,%.2f) is inside.",normX,normY) << std::endl;
        normX = fabs(normX), normY = fabs(normY);
        double xvec = -1,  yvec = -1./sqrt(3);
        double xref = 0.5, yref = sqrt(3)/2.;
        if( (normX > 1.) || (normY > yref) ){
            if(verbose_>2) std::cout << "[isInside]   they are outside quadrant." << std::endl;
            return false;
        }
        double dotProd = (xvec*(normX-xref) + yvec*(normY-yref));
        if(verbose_>2) std::cout << TString::Format("[isInside] they are inside quadrant. Dot product (>0 is inside): %.2f ", dotProd) << std::endl;
        return (dotProd > 0.);
    }

//    void EcalHexReadout::buildTriggerGroups() {
//
//        /*
//         * Assumptions
//         *  - Scanning from right to left, bottom to top of module hexagon (behavior default to TH2Poly)
//         *  - Ignoring cell at exact center of module (putting it into it's own trigger group)
//         */
//
//        //calculate total number of cells in the hexagon
//        int n = (nCellsWide_-1)/2;
//        int totalNumCells = 1 + 3n(n+1);
//        
//        triggerGroups_.resize(totalNumCells);
//
//        //loop through all cells, assigning them to a trigger group
//        int rowNum{0}, collNum{0};
//        for ( unsigned int cellID = 0; cellID < triggerGroups_.size(); cellID++ ) {
//            
//        }
//    }

}
