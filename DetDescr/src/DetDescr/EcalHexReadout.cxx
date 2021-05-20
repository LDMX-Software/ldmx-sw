#include "DetDescr/EcalHexReadout.h"

#include "TGeoPolygon.h"
#include "TGraph.h"
#include "TList.h"
#include "TMath.h"
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
  cellR_ = 2 * moduler_ / 35.;
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

  // module IDs are 0 for ecal center, 1 at right, and counterclockwise till 6
  double C_PI = 3.14159265358979323846;  // or TMath::Pi(), #define, atan(), ...
    
    modulePositionMap_[0] = std::pair<double, double>(0., 0.);
      for (unsigned id = 1; id < 7; id++) {
          double x = (2. * moduler_ + gap_) * cos((id-1) * (C_PI / 3.));
          double y = - (2. * moduler_ + gap_) * sin((id-1) * (C_PI / 3.));
        modulePositionMap_[id] = std::pair<double, double>(x, y);
        if (verbose_ > 2)
          std::cout << TString::Format("   id %d is at (%.2f, %.2f)", id, x, y)
                    << std::endl;
      }
    
  if (verbose_ > 0) std::cout << std::endl;
}

void EcalHexReadout::buildCellMap() {
  /** STRATEGY
   * Hard code
   */
    Double_t a = cellR_;  // cell long radius (or hexagon flat side length)
    Double_t xCenter = 0.;
    Double_t yCenter = 0.;
    Double_t numberOfCellsFlatSide = 12;
    Double_t x[6], y[6];

    //(xLoop,yLoop)-coordinate of the first starting vertex in the "for" loops
    //(xTemp,yTemp)-the changing starting vertex as loops through columns/rows
    Double_t xLoop, yLoop, xTemp, yTemp;
    int yColNum;
    int ecalMapID = 0;

    // The hexagon grid divides into three sections: two trapezoids
    // Cell map starts from top left

    // Section 1: Left trapezoid (13cell short base, 24cell long base, 12cell
    // lateral side
    xLoop = xCenter - a * 18.0;
    yLoop = yCenter + a * 18.0 / TMath::Sqrt(3);
    for (int xCounter = 0; xCounter < numberOfCellsFlatSide;
         xCounter++) {  // lateral side->12cells
      yTemp = yLoop;    // resets the temporary variable
      yColNum =
          numberOfCellsFlatSide + xCounter + 1;  // number of cells in each column
      for (int yCounter = 0; yCounter < yColNum; yCounter++) {
        // Create a cell: go clockwise around the hexagon vertices starting from
        // the left-most one
        x[0] = xLoop;
        y[0] = yTemp;
        x[1] = x[0] + a / 2.0;
        y[1] = y[0] + a * TMath::Sqrt(3) / 2.0;
        x[2] = x[1] + a;
        y[2] = y[1];
        x[3] = x[2] + a / 2.0;
        y[3] = y[0];
        x[4] = x[2];
        y[4] = y[3] - a * TMath::Sqrt(3) / 2.0;
        x[5] = x[1];
        y[5] = y[4];
        ecalMap_.AddBin(6, x, y);
        double cellx = (x[0] + x[3]) / 2;
        double celly = (y[1] + y[4]) / 2;
        cellPositionMap_[ecalMapID] = std::pair<double, double>(cellx, celly);
        ecalMapID++;
        // Go down
        yTemp -= a * TMath::Sqrt(3);
      }
      // Increment the starting position from column to column
      yLoop += a * TMath::Sqrt(3) / 2.0;
      xLoop += 1.5 * a;
    }

    // Section 2: Right trapezoid (12cell short base, 23cell long base, 12cell
    // lateral side
    xLoop = xCenter;
    yLoop = yCenter + a * 11.0 * TMath::Sqrt(3);
    for (int xCounter = 0; xCounter < numberOfCellsFlatSide; xCounter++) {
      yTemp = yLoop;
      yColNum = 2 * numberOfCellsFlatSide - xCounter -
                1;  // number of cells in each column
      for (int yCounter = 0; yCounter < yColNum; yCounter++) {
        // Create a cell: go clockwise around the hexagon vertices starting from
        // the left-most one
        x[0] = xLoop;
        y[0] = yTemp;
        x[1] = x[0] + a / 2.0;
        y[1] = y[0] + a * TMath::Sqrt(3) / 2.0;
        x[2] = x[1] + a;
        y[2] = y[1];
        x[3] = x[2] + a / 2.0;
        y[3] = y[0];
        x[4] = x[2];
        y[4] = y[3] - a * TMath::Sqrt(3) / 2.0;
        x[5] = x[1];
        y[5] = y[4];
        ecalMap_.AddBin(6, x, y);
        double cellx = (x[0] + x[3]) / 2;
        double celly = (y[1] + y[4]) / 2;
        cellPositionMap_[ecalMapID] = std::pair<double, double>(cellx, celly);
        ecalMapID++;
        // Go down
        yTemp -= a * TMath::Sqrt(3);
      }
      // Increment the starting position from column to column
      yLoop -= a * TMath::Sqrt(3) / 2.0;
      xLoop += 1.5 * a;
    }

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
