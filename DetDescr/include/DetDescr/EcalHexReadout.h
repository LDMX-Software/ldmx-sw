/**
 * @file EcalHexReadout.h
 * @brief Class that translates raw positions of ECal module hits into cells in
 * a hexagonal readout
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
#include "DetDescr/EcalID.h"
#include "Framework/ConditionsObject.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

// STL
#include <map>

// ROOT
#include "TH2Poly.h"

namespace ecal {
class EcalGeometryProvider;
}

namespace ldmx {

/**
 * @class EcalHexReadout
 * @brief Implementation of ECal hexagonal cell readout
 *
 * This is the object that does the extra geometry *not* implemented in the
 * gdml. In order to save time during the simulation, the individual cells in
 * the readout hexagons are not constructed individually in Geant4. This means
 * we have to have a translation between position and cell ID which is
 * accomplished by this class.
 *
 * ## ORIENTATION ASSUMPTIONS:
 * - modules are oriented flat side down.
 * - cells are oriented corner side down.
 *
 * ## SOME GEOMETRY:
 * - hexagons have two radii:
 *   - r (half of flat-to-flat width) and R (half of corner-to-corner width).
 *   - r = (sqrt(3)/2)R and s = R, where s is the length of an edge.
 * - for seven ecal modules oriented flat-side-down, maximum x and y extents
 * are:
 *   - deltaY = 6r' + 2g = 3sqrt(3)R' + 2g
 *   - deltaX = 4R' + s' + 2g/cos(30 deg) = 5R' + 4g/sqrt(3)
 *   - where g is uniform gap width between modules, and primed variables
 * correspond to modules.
 *
 * ## THIS GRID:
 * - column-to-column distance in a grid such as ours is 2r = sqrt(3)R.
 * - row-to-row distance is 1.5R (easy to observe that twice that distance = 3R)
 *
 * The cell radius is calculated from the total number of center-to-corner cell
 * radii that span the module height. This count can have fractional counts to
 * account for the fractions of cell radii at the module edges.
 */
class EcalHexReadout : public framework::ConditionsObject {
 public:
  static constexpr const char* CONDITIONS_OBJECT_NAME{"EcalHexReadout"};

  /**
   * Class destructor.
   *
   * Does nothing because the stl containers clean up automatically.
   */
  virtual ~EcalHexReadout() {}

  /**
   * Get entire real space position for the cell with the input raw ID
   *
   * Inputs x,y,z will be set to the calculated position
   *
   * @sa getCellCenterAbsolute and getZPosition
   *
   * @param[in] id EcalID for the cell we want the position of
   * @param[out] x set to x-coordinate of cell center
   * @param[out] y set to y-coordinate of cell center
   * @param[out] z set to z-coordinate of cell center
   */
  void getCellAbsolutePosition(EcalID id, double& x, double& y,
                               double& z) const {
    std::pair<double, double> xy =
        this->getCellCenterAbsolute(EcalID(0, id.module(), id.cell()));
    x = xy.first;
    y = xy.second;
    z = getZPosition(id.layer());

    return;
  }

  /**
   * Get the z-coordinate given the layer id
   *
   * @param[in] layer int layer id
   * @return z-coordinate of the input sensitive layer
   */
  double getZPosition(int layer) const {
    return ecalFrontZ_ + layerZPositions_.at(layer);
  }

  /**
   * Get the number of layers in the Ecal Geometry
   *
   * @returns number fo layers in geometry
   */
  int getNumLayers() const { return layerZPositions_.size(); }

  /**
   * Get a module center position relative to the ecal center [mm]
   *
   * @param[in] moduleID id of the module
   * @return (x,y) coordinate pair for center of module
   */
  std::pair<double, double> getModuleCenter(int moduleID) const {
    return modulePositionMap_.at(moduleID);
  }

  /**
   * Get the number of modules in the Ecal flower
   *
   * @returns number of modules
   */
  int getNumModulesPerLayer() const { return modulePositionMap_.size(); }

  /**
   * Get a module ID from an XY position relative to the ecal center [mm]
   *
   * @param[in] x x-coordinate for test position
   * @param[in] y y-coordinate for test position
   * @return module ID that coordinate (x,y) is in
   */
  int getModuleID(double x, double y) const {
    int bestID = -1;
    double bestDist = 1E6;
    for (auto const& module : modulePositionMap_) {
      int mID = module.first;
      double mX = module.second.first;
      double mY = module.second.second;
      double dist = sqrt((x - mX) * (x - mX) + (y - mY) * (y - mY));
      if (dist < moduler_) return mID;
      if (dist < bestDist) {
        bestID = mID;
        bestDist = dist;
      }
    }
    return bestID;
  }

  /**
   * Get a cell ID from an XY position relative to module center.
   *
   * This is where invalid (x,y) from external calls will end up failing and
   * need error handling.
   *
   * @note One option for assigning dead material is to do it here.
   * If the input (x,y) relative to the module is close enough to the edge (for
   * example), we could return a special cell ID (-1 for example) that signals
   * that this hit is in a dead region.
   *
   * @param x Any X position [mm]
   * @param y Any Y position [mm]
   * @return local cell ID to the module
   */
  int getCellIDRelative(double x, double y) const {
    int bin = ecalMap_.FindBin(x, y) -
              1;  // NB FindBin indices starts from 1, our maps start from 0
    if (bin < 0) {
      TString error_msg = TString(
                              "[EcalHexReadout::getCellIDRelative] Relative "
                              "coordinates are outside module hexagon!") +
                          TString::Format(
                              " Is the gap used by EcalHexReadout (%.2f mm) "
                              "and the minimum module radius (%.2f mm)",
                              gap_, moduler_) +
                          TString::Format(
                              " the same as hexagon_gap and Hex_radius in "
                              "ecal.gdml? Received (x,y) = (%.2f,%.2f).",
                              x, y);
      EXCEPTION_RAISE("InvalidArg", error_msg.Data());
    }
    return bin;
  }

  /**
   * Get the number of cells in an ecal module
   *
   * @returns number of cells in the ecal module
   */
  int getNumCellsPerModule() const { return ecalMap_.GetNumberOfBins(); }

  /**
   * Get a cell center XY position relative to module center from a cell ID.
   *
   * @throw Exception if invalid cell ID is input.
   *
   * @param cellID The cell ID.
   * @return The (x,y) position of the center of the cell.
   */
  std::pair<double, double> getCellCenterRelative(int cellID) const {
    // this map search is probably just as fine as the TList search for the cell
    // in ecalMap.
    //   wonder why TH2Poly->GetBin(ID) doesn't exist. plus the map is useful by
    //   itself.
    auto search = cellPositionMap_.find(cellID);
    if (search == cellPositionMap_.end()) {
      EXCEPTION_RAISE("InvalidCellID",
                      "Cell " + std::to_string(cellID) + " is not valid.");
    }
    return search->second;
  }

  /**
   * Get a combined cellModule ID from an XY position relative to ecal center.
   *
   * @sa getCellIDRelative and getModuleID
   *
   * @param x Any X position [mm]
   * @param y Any Y position [mm]
   * @return an EcalID that has the correct module and cell information while
   * layer is set to zero
   */
  EcalID getCellModuleID(double x, double y) const {
    int moduleID = getModuleID(x, y);
    double relX = x - modulePositionMap_.at(moduleID).first;
    double relY = y - modulePositionMap_.at(moduleID).second;
    int cellID = getCellIDRelative(relX, relY);
    return EcalID(0, moduleID, cellID);
  }

  /**
   * Get a cell center XY position relative to ecal center from a combined
   * cellModuleID.
   *
   * @throw std::out_of_range if EcalID isn't created with valid cell or module
   * IDs.
   *
   * @param cellModuleID EcalID where all we care about is module and cell
   * @return The XY position of the center of the cell.
   */
  std::pair<double, double> getCellCenterAbsolute(EcalID cellModuleID) const {
    return cellModulePositionMap_.at(
        EcalID(0, cellModuleID.module(), cellModuleID.cell()));
  }

  /**
   * Distance to module edge, and whether cell is on edge of module.
   *
   * @TODO Use getNN()/getNNN() + isEdgeCell() to expand functionality.
   *
   * @param[in] cellModuleID EcalId where all we care about is module and cell
   * @return distance to edge of the module
   */
  double distanceToEdge(EcalID cellModuleID) const;

  /**
   * Check if input cell is on the edge of a module.
   *
   * @sa distanceToEdge
   *
   * @param[in] cellModuleID EcalId where all we care about is module and cell
   * return true if distance to edge is less than max cell radius
   */
  bool isEdgeCell(EcalID cellModuleID) const {
    return (distanceToEdge(cellModuleID) < cellR_);
  }

  /**
   * Determines if point (x,y), already normed to max hexagon radius, lies
   * within a hexagon. Corners are (1,0) and (0.5,sqrt(3)/2). Uses "<", not
   * "<=".
   *
   * @param[in] normX X-coordinate relative to module hexagon divided by maximum
   * hexagon radius
   * @param[in] normY Y-coordinate relative to module hexagon divided by maximum
   * hexagon radius
   * @return true if (normX,normY) is within the hexagon centered at the origin
   * with maximum radius 1.
   */
  bool isInside(double normX, double normY) const;

  /**
   * Get the Nearest Neighbors of the input ID
   *
   * @param id id to get
   * @return list of EcalID that are the inputs nearest neighbors
   */
  std::vector<EcalID> getNN(EcalID id) const {
    auto list = NNMap_.at(EcalID(0, id.module(), id.cell()));
    for (auto& flat : list) id = EcalID(id.layer(), flat.module(), flat.cell());
    return list;
  }

  /**
   * Check if the probe id is one of the nearest neightbors of the centroid id
   *
   * @param probe id to check if it is nearest neighbor
   * @param centroid id that is center of neighbors
   * @return true if probe ID is a nearest neighbor of the centroid
   */
  bool isNN(EcalID centroid, EcalID probe) const {
    for (auto& id : getNN(centroid)) {
      if (id == probe) return true;
    }
    return false;
  }

  /**
   * Get the Next-to-Nearest Neighbors of the input ID
   *
   * @param id id to get
   * @return list of EcalID that are the inputs next-to-nearest neighbors
   */
  std::vector<EcalID> getNNN(EcalID id) const {
    auto list = NNNMap_.at(EcalID(0, id.module(), id.cell()));
    for (auto& flat : list) id = EcalID(id.layer(), flat.module(), flat.cell());
    return list;
  }

  /**
   * Check if the probe id is one of the next-to-nearest neightbors of the
   * centroid id
   *
   * @param probe id to check if it is a next-to-nearest neighbor
   * @param centroid id that is center of neighbors
   * @return true if probe ID is a next-to-nearest neighbor of the centroid
   */
  bool isNNN(EcalID centroid, EcalID probe) const {
    for (auto& id : getNNN(centroid)) {
      if (id == probe) return true;
    }
    return false;
  }

  /**
   * Get the center-to-flat radius of the module hexagons
   *
   * @return module min radius [mm]
   */
  double getModuleMinR() const { return moduler_; }

  /**
   * Get the center-to-corner radius of the module hexagons
   *
   * @return module max radius [mm]
   */
  double getModuleMaxR() const { return moduleR_; }

  /**
   * Get the center-to-flat radius of the cell hexagons
   *
   * @return cell min radius [mm]
   */
  double getCellMinR() const { return cellr_; }

  /**
   * Get the center-to-corner radius of the cell hexagons
   *
   * @return cell max radius [mm]
   */
  double getCellMaxR() const { return cellR_; }

  /**
   * Get a const reference to the cell center position map.
   */
  const std::map<int, std::pair<double, double>>& getCellPositionMap() const {
    return cellPositionMap_;
  }

  /**
   * Get a const reference to the full cell-module position map.
   */
  const std::map<EcalID, std::pair<double, double>>& getCellModulePositionMap()
      const {
    return cellModulePositionMap_;
  }

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

  static EcalHexReadout* debugMake(const framework::config::Parameters& p) {
    return new EcalHexReadout(p);
  }

 private:
  /**
   * Class constructor, for use only by the provider
   *
   * @param ps Parameters to configure the EcalHexReadout
   */
  EcalHexReadout(const framework::config::Parameters& ps);
  friend class ecal::EcalGeometryProvider;

  /**
   * Constructs the positions of the seven modules (moduleID) relative to the
   * ecal center
   *
   * Sets modulePositionMap_ using the module IDs for keys and the centers of
   * the module hexagons for values.
   *
   * The module IDs are set in the ecal.gdml file and are replicated here.
   *  - 0 for center module
   *  - 1 on top (12 o'clock)
   *  - clockwise till 6 at 11 o'clock
   *
   * @param[in] gap_ separation between module flat-sides
   * @param[in] moduler_ center-to-flat module radius
   * @param[out] modulePositionMap_ map of module IDs to module centers relative
   * to ecal
   */
  void buildModuleMap();

  /**
   * Constructs the flat-bottomed hexagonal grid (cellID) of corner-down
   * hexagonal cells.
   *
   * Sets ecalMap_ with the defined bins being the ecal cells in coordinates
   * with respect to the module center. Also sets cellPostionMap_ with the keys
   * being the cell ID and the values being the position of the cell with
   * respect to the module center.
   *
   * ## Strategy
   * Use ROOT's TH2Poly::HoneyComb method to build a large hexagonal grid,
   * then copy the polygons from it which overlap with the module with
   * more than one vertex.
   *
   * A vertex between three cells is placed at the origin,
   * then the bottom left corner of the honeycomb and the number of x and y
   * cells across the honeycomb is calculated by continuing to decrement
   * the grid x/y point until the module center-to-flat distance is reached.
   *
   * The hexagons that only have one vertex outside the module hexagon leave
   * a small space un-covered by the tiling, so the vertices adjacent to the
   * external vertex are projected onto the module edge.
   *
   * @param[in] cellr_ the center-to-flat cell radius
   * @param[in] cellR_ the center-to-corner cell radius
   * @param[in] moduler_ the center-to-flat module radius
   * @param[in] moduleR_ the center-to-flat module radius
   * @param[out] ecalMap_ TH2Poly with local cell ID to local cell position
   * mapping
   * @param[out] cellPostionMap_ map of local cell ID to cell center position
   * relative to module
   */
  void buildCellMap();

  /**
   * Constructs the positions of all the cells in a layer relative to the ecal
   * center.
   *
   * This uses the modulePostionMap_ and cellPositionMap_ to calculate the
   * center of all cells relative to the ecal center.
   *
   * @param[in] modulePositionMap_ map of module IDs to module centers relative
   * to ecal
   * @param[in] cellPositionMap_ map of cell IDs to cell centers relative to
   * module
   * @param[out]
   */
  void buildCellModuleMap();

  /**
   * Construts NNMap and NNNMap
   *
   * Since this only occurs once during processing, we can be wasteful.
   * We do a nested loop over the entire cellular position map and calculate
   * neighbors by seeing which cells are within multiples of the cellular radius
   * of each other.
   *
   * @param[in] cellModulePostionMap_ map of cells to cell centers relative to
   * ecal
   * @param[out] NNMap_ map of cell IDs to list of cell IDs that are its nearest
   * neighbors
   * @param[out] NNNMap_ map of cell IDs to list of cell IDs that are its
   * next-to-nearest neighbors
   */
  void buildNeighborMaps();

  /**
   * Constructs list of trigger groups.
   *
   * The list of trigger groups uses the index as the cell ID and the value as
   * the ID of the trigger group.
   *
   * For the ECal, cells are grouped into 3x3 "squares" and integrated over for
   * the trigger. This function builds a list for finding which trigger group a
   * cell is in.
   */
  void buildTriggerGroup();

 private:
  /// verbosity, not configurable but helpful if developing
  int verbose_{2};

  /// Gap between module flat sides [mm]
  double gap_;

  /// Center-to-Flat Radius of cell hexagon [mm]
  double cellr_{0};

  /// Center-to-Flat Radius of module hexagon [mm]
  double moduler_{0};

  /// Center-to-Corner Radius of cell hexagon [mm]
  double cellR_{0};

  /// Center-to-Corner Radius of module hexagon [mm]
  double moduleR_{0};

  /**
   * Number of cell center-to-corner radii (one side of the cell)
   * from the bottom to the top of the module
   *
   * Could be fractional depending on how many fractions of a radii are spanning
   * between the center of the top/bottom cell row and the edge of the module
   */
  double nCellRHeight_{0};

  /// Front of ECal relative to world geometry [mm]
  double ecalFrontZ_{0};

  /// The layer Z postions are with respect to the front of the ECal [mm]
  std::vector<double> layerZPositions_;

  /// Postion of module centers relative to world geometry (uses module ID as
  /// key)
  std::map<int, std::pair<double, double>> modulePositionMap_;

  /// Position of cell centers relative to module (uses cell ID as key)
  std::map<int, std::pair<double, double>> cellPositionMap_;

  /// Position of cell centers relative to world geometry (uses ID with real
  /// cell and module and layer as zero for key)
  std::map<EcalID, std::pair<double, double>> cellModulePositionMap_;

  /// Map of cell ID to neighboring cells (uses ID with real cell and module and
  /// layer as zero for key)
  std::map<EcalID, std::vector<EcalID>> NNMap_;

  /// Map of cell ID to neighbors of neighbor cells (uses ID with real cell and
  /// module and layer as zero for key)
  std::map<EcalID, std::vector<EcalID>> NNNMap_;

  /// List of Trigger Group IDs (index is cell ID)
  std::vector<int> triggerGroups_;

  /**
   * Honeycomb Binning from ROOT
   *
   * Needs to be mutable because ROOT doesn't have good const handling
   */
  mutable TH2Poly ecalMap_;
};

}  // namespace ldmx

#endif
