#include "DetDescr/EcalHexReadout.h"

#include "TGeoPolygon.h"
#include "TGraph.h"
#include "TList.h"
#include "TMultiGraph.h"

#include <assert.h>
#include <iomanip>
#include <iostream>

namespace ldmx {

EcalHexReadout::EcalHexReadout(const framework::config::Parameters& ps)
    : framework::ConditionsObject(EcalHexReadout::CONDITIONS_OBJECT_NAME) {
  layerZPositions_ = ps.getParameter<std::vector<double>>("layerZPositions");
  ecalFrontZ_ = ps.getParameter<double>("ecalFrontZ");
  moduler_ = ps.getParameter<double>("moduleMinR");
  gap_ = ps.getParameter<double>("gap");
  nCellRHeight_ = ps.getParameter<double>("nCellRHeight");
  verbose_ = ps.getParameter<int>("verbose");

  moduleR_ = moduler_ * (2 / sqrt(3));
  cellR_ = 2 * moduler_ / nCellRHeight_;
  cellr_ = (sqrt(3.) / 2.) * cellR_;

  if (verbose_ > 0) {
    std::cout << std::endl
              << "[EcalHexReadout] Verbosity set in header to " << verbose_
              << std::endl;
    std::cout << "     Building module map with gap " << std::setprecision(2)
              << gap_ << ", nCellRHeight " << nCellRHeight_
              << ",  min/max radii of cell " << cellr_ << " / " << cellR_
              << ", and module " << moduler_ << " / " << moduleR_ << std::endl;
  }

  buildModuleMap();
  buildCellMap();
  buildCellModuleMap();
  buildNeighborMaps();

  if (verbose_ > 0) {
    std::cout << std::endl;
  }
}

void EcalHexReadout::buildModuleMap() {
  if (verbose_ > 0) {
    std::cout
        << std::endl
        << "[buildModuleMap] Building module position map for module min r of "
        << moduler_ << "    and gap of " << gap_ << std::endl;
  }

  // module IDs are 0 for ecal center, 1 at 12 o'clock, and clockwise till 6 at
  // 11 o'clock.
  double C_PI = 3.14159265358979323846;  // or TMath::Pi(), #define, atan(), ...
  modulePositionMap_[0] = std::pair<double, double>(0., 0.);
  for (unsigned id = 1; id < 7; id++) {
    double x = (2. * moduler_ + gap_) * sin((id - 1) * (C_PI / 3.));
    double y = (2. * moduler_ + gap_) * cos((id - 1) * (C_PI / 3.));
    modulePositionMap_[id] = std::pair<double, double>(x, y);
    if (verbose_ > 2)
      std::cout << TString::Format("   id %d is at (%.2f, %.2f)", id, x, y)
                << std::endl;
  }
  if (verbose_ > 0) std::cout << std::endl;
}

void EcalHexReadout::buildCellMap() {
  /** STRATEGY
   * use native ROOT HoneyComb method to build large hexagonal grid.
   * then copy from it the polygons which cover a module.
   */
  TH2Poly gridMap;

  // make hexagonal grid [boundary is rectangle] larger than the module
  double gridMinX = 0., gridMinY = 0.;  // start at the origin
  int numXCells = 0, numYCells = 0;

  // first x-cell is only a half
  gridMinX -= cellr_;
  numXCells++;
  while (gridMinX > -1 * moduleR_) {
    gridMinX -= 2 * cellr_;  // decrement x by cell center-to-flat diameter
    numXCells++;
  }
  while (gridMinY > -1 * moduler_) {
    // decrement y by cell center-to-corner radius
    //  alternate between a full corner-to-corner diameter
    //  and a side of a cell (center-to-corner radius)
    if (numYCells % 2 == 0)
      gridMinY -= 1 * cellR_;
    else
      gridMinY -= 2 * cellR_;
    numYCells++;
  }
  // only counted one half of the cells
  numXCells *= 2;
  numYCells *= 2;

  gridMap.Honeycomb(gridMinX, gridMinY, cellR_, numXCells, numYCells);

  if (verbose_ > 0) {
    std::cout << std::endl;
    std::cout << std::setprecision(2) << "[buildCellMap] cell rmin: " << cellr_
              << " cell rmax: " << cellR_ << " (gridMinX,gridMinY) = ("
              << gridMinX << "," << gridMinY << ")"
              << " (numXCells,numYCells) = (" << numXCells << "," << numYCells
              << ")" << std::endl;
  }

  // copy cells lying within module boundaries to a module grid
  TListIter next(gridMap.GetBins());  // a TH2Poly is a TList of TH2PolyBin
  TH2PolyBin* polyBin = 0;
  TGraph* poly = 0;   // a polygon returned by TH2Poly is a TGraph
  int ecalMapID = 0;  // ecalMap cell IDs go from 0 to N-1, not equal to
                      // original grid cell ID.
  while ((polyBin = (TH2PolyBin*)next())) {
    // these bins are coming from the honeycomb
    //  grid so we assume that they are regular
    //  hexagons i.e. has 6 vertices
    poly = (TGraph*)polyBin->GetPolygon();

    // decide whether to copy polygon to new map.
    // use all vertices in case of cut-off edge polygons.
    int numVerticesInside = 0;
    double vertex_x[6], vertex_y[6];  // vertices of the cell
    bool isinside[6];                 // which vertices are inside
    if (verbose_ > 2) std::cout << "[buildCellMap] Cell vertices" << std::endl;
    for (unsigned i = 0; i < 6; i++) {
      poly->GetPoint(i, vertex_x[i], vertex_y[i]);
      if (verbose_ > 2) {
        std::cout << "     vtx # " << i << std::endl;
        std::cout << "     vtx x,y " << vertex_x[i] << " " << vertex_y[i]
                  << std::endl;
      }
      isinside[i] = isInside(vertex_x[i] / moduleR_, vertex_y[i] / moduleR_);
      if (isinside[i]) numVerticesInside++;
    }

    if (numVerticesInside > 1) {
      // Include this cell if more than one of its vertices is inside the module
      // hexagon
      double actual_x[8], actual_y[8];
      int num_vertices{0};
      if (numVerticesInside < 6) {
        // This cell is stradling the edge of the module
        // and is NOT cleanly cut by module edge

        if (verbose_ > 1) {
          std::cout << "[buildCellMap] Polygon " << ecalMapID
                    << " has vertices poking out of module hexagon."
                    << std::endl;
        }

        // loop through vertices
        for (int i = 0; i < 6; i++) {
          int up = i == 5 ? 0 : i + 1;
          int dn = i == 0 ? 5 : i - 1;
          if (isinside[i] and (not isinside[up] or not isinside[dn])) {
            // this vertex is inside the module hexagon and is adjacent to a
            // vertex outside
            // ==> project this vertex onto the nearest edge of the module
            // hexagon

            // determine which side of hexagon we should project onto
            double edge_origin_x, edge_origin_y;
            double edge_dest_x, edge_dest_y;
            if (vertex_x[i] < -moduleR_ / 2.) {
              // sloped edge on negative-x side
              edge_origin_x = -1. * moduleR_;
              edge_origin_y = 0.;
              edge_dest_x = -0.5 * moduleR_;
              edge_dest_y = moduler_;
            } else if (vertex_x[i] > moduleR_ / 2.) {
              // sloped edge on positive-x side
              edge_origin_x = 0.5 * moduleR_;
              edge_origin_y = moduler_;
              edge_dest_x = moduleR_;
              edge_dest_y = 0.;
            } else {
              // flat edge at top
              edge_origin_x = 0.5 * moduleR_;
              edge_origin_y = moduler_;
              edge_dest_x = -0.5 * moduleR_;
              edge_dest_y = moduler_;
            }

            // flip to bottom half if below x-axis
            if (vertex_y[i] < 0) {
              edge_dest_y *= -1;
              edge_origin_y *= -1;
            }

            // get edge slope vector
            double edge_slope_x = edge_dest_x - edge_origin_x;
            double edge_slope_y = edge_dest_y - edge_origin_y;

            if (verbose_ > 2) {
              std::cout
                  << "Vertex " << i
                  << " is inside and adjacent to a vertex outside the module."
                  << std::endl;
              std::cout << "Working on edge with slope (" << edge_slope_x << ","
                        << edge_slope_y << ")"
                        << " and origin (" << edge_origin_x << ","
                        << edge_origin_y << ")" << std::endl;
            }

            // project vertices adjacent to the vertex outside the module onto
            // the module edge
            double projection_factor =
                ((vertex_x[i] - edge_origin_x) * edge_slope_x +
                 (vertex_y[i] - edge_origin_y) * edge_slope_y) /
                (edge_slope_x * edge_slope_x + edge_slope_y * edge_slope_y);

            double proj_x = edge_origin_x + projection_factor * edge_slope_x;
            double proj_y = edge_origin_y + projection_factor * edge_slope_y;

            if (not isinside[up]) {
              // the next point is outside
              actual_x[num_vertices] = vertex_x[i];
              actual_y[num_vertices] = vertex_y[i];
              actual_x[num_vertices + 1] = proj_x;
              actual_y[num_vertices + 1] = proj_y;
            } else {
              // the previous point was outside
              actual_x[num_vertices] = proj_x;
              actual_y[num_vertices] = proj_y;
              actual_x[num_vertices + 1] = vertex_x[i];
              actual_y[num_vertices + 1] = vertex_y[i];
            }
            num_vertices += 2;

            if (verbose_ > 2) {
              std::cout << "New Vertex " << i << " : (" << vertex_x[i] << ","
                        << vertex_y[i] << ")" << std::endl;
            }
          } else {
            actual_x[num_vertices] = vertex_x[i];
            actual_y[num_vertices] = vertex_y[i];
            num_vertices++;
          }  // should we project or not
        }    // loop through vertices
      } else {
        // all 6 inside, just copy the vertices over
        num_vertices = 6;
        for (int i = 0; i < 6; i++) {
          actual_x[i] = vertex_x[i];
          actual_y[i] = vertex_y[i];
        }
      }  // if numVerticesInside is less than 5

      // ecalMap_ needs to have its own copy of the polygon TGraph
      //  otherwise, we get a seg fault when EcalHexReadout is destructed
      //  because the polygon that was copied over from gridMap is deleted at
      //  the end of this function
      ecalMap_.AddBin(num_vertices, actual_x, actual_y);

      double x = (polyBin->GetXMax() + polyBin->GetXMin()) / 2.;
      double y = (polyBin->GetYMax() + polyBin->GetYMin()) / 2.;
      if (verbose_ > 1) {
        std::cout << "[buildCellMap] Copying poly with ID "
                  << polyBin->GetBinNumber() << " and (x,y) ("
                  << std::setprecision(2) << x << "," << y << ")" << std::endl;
      }
      // save cell location as center of ENTIRE hexagon
      cellPositionMap_[ecalMapID] = std::pair<double, double>(x, y);
      ecalMapID++;  // incrememnt cell ID
    }               // if num vertices inside is > 1
  }                 // loop over larger grid spanning module hexagon

  if (verbose_ > 0) std::cout << std::endl;
  return;
}

void EcalHexReadout::buildCellModuleMap() {
  if (verbose_ > 0)
    std::cout << std::endl
              << "[buildCellModuleMap] Building cellModule position map"
              << std::endl;
  for (auto const& module : modulePositionMap_) {
    int moduleID = module.first;
    double moduleX = module.second.first;
    double moduleY = module.second.second;
    for (auto const& cell : cellPositionMap_) {
      int cellID = cell.first;
      double cellX = cell.second.first;
      double cellY = cell.second.second;
      double x = cellX + moduleX;
      double y = cellY + moduleY;
      cellModulePositionMap_[EcalID(0, moduleID, cellID)] =
          std::pair<double, double>(x, y);
    }
  }
  if (verbose_ > 0)
    std::cout << "  contained " << cellModulePositionMap_.size() << " entries. "
              << std::endl;
}

void EcalHexReadout::buildNeighborMaps() {
  /** STRATEGY
   * Neighbors may include from other modules. All this is precomputed. So we
   * can be wasteful here. Gaps may be nonzero, so we simply apply an anulus
   * requirement (r < point <= r+dr) using total x,y positions relative to the
   * ecal center (cell+module positions). This makes the routine portable to
   * future cell layouts. Note that the module centers already take into account
   * a nonzero gap. The number of neighbors is not simple because: edges, and
   * that module edges have cutoff cells. (NN) Center within [1*cellr_,
   * 3*cellr_] (NNN) Center within [3*cellr_, 4.5*cellr_] Chosen b/c in ideal
   * case, centers are at 2*cell_ (NN), and at 3*cellR_=3.46*cellr_ and 4*cellr_
   * (NNN).
   */

  NNMap_.clear();
  NNNMap_.clear();
  for (auto const& centerChannel : cellModulePositionMap_) {
    EcalID centerID = centerChannel.first;
    double centerX = centerChannel.second.first;
    double centerY = centerChannel.second.second;
    for (auto const& probeChannel : cellModulePositionMap_) {
      EcalID probeID = probeChannel.first;
      double probeX = probeChannel.second.first;
      double probeY = probeChannel.second.second;
      double dist = sqrt((probeX - centerX) * (probeX - centerX) +
                         (probeY - centerY) * (probeY - centerY));
      if (dist > 1 * cellr_ && dist <= 3. * cellr_) {
        NNMap_[centerID].push_back(probeID);
      } else if (dist > 3. * cellr_ && dist <= 4.5 * cellr_) {
        NNNMap_[centerID].push_back(probeID);
      }
    }
    if (verbose_ > 1)
      std::cout << TString::Format("Found %d NN and %d NNN for cellModuleID ",
                                   int(NNMap_[centerID].size()),
                                   int(NNNMap_[centerID].size()))
                << centerID
                << TString::Format(" with x,y (%.2f,%.2f)", centerX, centerY)
                << std::endl;
  }
  if (verbose_ > 2) {
    double specialX =
        0.5 * moduleR_ - 0.5 * cellr_;  // center of cell which is upper-right
                                        // corner of center module
    double specialY = moduler_ - 0.5 * cellR_;
    EcalID specialCellModuleID = getCellModuleID(specialX, specialY);
    std::cout << "The neighbors of the bin in the upper-right corner of the "
                 "center module, with cellModuleID "
              << specialCellModuleID << " include " << std::endl;
    for (auto centerNN : NNMap_.at(specialCellModuleID)) {
      std::cout << " NN " << centerNN
                << TString::Format(" (x,y) (%.2f, %.2f)",
                                   getCellCenterAbsolute(centerNN).first,
                                   getCellCenterAbsolute(centerNN).second)
                << std::endl;
    }
    for (auto centerNNN : NNNMap_.at(specialCellModuleID)) {
      std::cout << " NNN " << centerNNN
                << TString::Format(" (x,y) (%.2f, %.2f)",
                                   getCellCenterAbsolute(centerNNN).first,
                                   getCellCenterAbsolute(centerNNN).second)
                << std::endl;
    }
    std::cout << TString::Format(
                     "This bin is a distance of %.2f away from a module edge. "
                     "Decision isEdge %d.",
                     distanceToEdge(specialCellModuleID),
                     isEdgeCell(specialCellModuleID))
              << std::endl;
  }
  if (verbose_ > 0) std::cout << std::endl;
  return;
}

double EcalHexReadout::distanceToEdge(EcalID cellModuleID) const {
  // https://math.stackexchange.com/questions/1210572/find-the-distance-to-the-edge-of-a-hexagon
  int cellID = cellModuleID.cell();
  std::pair<double, double> cellLocation = getCellCenterRelative(cellID);
  double x = fabs(cellLocation.first);  // bring to first quadrant
  double y = fabs(cellLocation.second);
  double r = sqrt(x * x + y * y);
  double theta = (r > 1E-3) ? fabs(std::atan(y / x)) : 0;
  if (x < moduleR_ / 2.)
    return (moduler_ - y);  // closest line is straight vertical to top edge
  double dist =
      sqrt(3.) * moduleR_ / (std::sin(theta) + sqrt(3.) * std::cos(theta));
  return dist;
}

bool EcalHexReadout::isInside(double normX, double normY) const {
  if (verbose_ > 2)
    std::cout << TString::Format(
                     "[isInside] Checking if normXY=(%.2f,%.2f) is inside.",
                     normX, normY)
              << std::endl;
  normX = fabs(normX), normY = fabs(normY);
  double xvec = -1, yvec = -1. / sqrt(3);
  double xref = 0.5, yref = sqrt(3) / 2.;
  if ((normX > 1.) || (normY > yref)) {
    if (verbose_ > 2)
      std::cout << "[isInside]   they are outside quadrant." << std::endl;
    return false;
  }
  double dotProd = (xvec * (normX - xref) + yvec * (normY - yref));
  if (verbose_ > 2)
    std::cout << TString::Format(
                     "[isInside] they are inside quadrant. Dot product (>0 is "
                     "inside): %.2f ",
                     dotProd)
              << std::endl;
  return (dotProd > 0.);
}

}  // namespace ldmx
