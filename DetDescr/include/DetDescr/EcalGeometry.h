/**
 * @file EcalGeometry.h
 * @brief Class that translates raw positions of ECal module hits into cells in
 * a hexagonal readout
 * @author Owen Colegro, UCSB
 *  past code sources:
 *           https://cms-hgcal.github.io/TestBeam/HGCSSGeometryConversion_8cc_source.html
 *           modified TH2Poly::Honeycomb routine
 * @author Patterson, UCSB
 * @author Tom Eichlersmith, University of Minnesota
 * @author Hongyin Liu, UCSB
 */

#ifndef DETDESCR_ECALGEOMETRY_H_
#define DETDESCR_ECALGEOMETRY_H_

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
 * Translation between real-space positions and cell IDs within
 * the ECal.
 *
 * This is the object that does the extra geometry *not* implemented in the
 * gdml. In order to save time during the simulation, the individual cells in
 * the readout hexagons are not constructed individually in Geant4. This means
 * we have to have a translation between position and cell ID which is
 * accomplished by this class.
 *
 * Moreover, downstream processors sometimes need access to the cell position
 * when they only know the cell ID. This class can also do this conversion.
 *
 * ## ORIENTATION ASSUMPTIONS:
 * - modules and cells have opposite orientation. i.e. if modules are corner-side
 *   down, then cells are flat-side down.
 *
 * ## SOME GEOMETRY:
 * - hexagons have two radii:
 *   - r (half of flat-to-flat width) and R (half of corner-to-corner width).
 *   - r = (sqrt(3)/2)R and s = R, where s is the length of an edge.
 * - for seven ecal modules oriented flat-side-down, 
 *   maximum x and y extents are:
 *   - deltaY = 6r' + 2g = 3sqrt(3)R' + 2g
 *   - deltaX = 4R' + s' + 2g/cos(30 deg) = 5R' + 4g/sqrt(3)
 *   - where g is uniform gap width between modules, 
 *     and primed variables correspond to modules.
 *
 * Since the whole hexagon flower has changed orientation since previous versions
 * of the detector (which we wish to still support), I am defining an extra set of 
 * axes with respect to the flower itself. Below I have drawn the ECal hexagon i
 * flower and defined two axes: p (through the "pointy sides") and q (through the 
 * "flat sides"). In some versions of the ECal flower, p == x and q == y while
 * in others p == -y and q == x.
 *
 *          ^
 *          | q axis
 *         __
 *      __/1 \__
 *     /6 \__/2 \
 *     \__/0 \__/ __ p axis __>
 *     /5 \__/3 \
 *     \__/4 \__/
 *        \__/
 *
 * Now we can define a process for determining the cellular IDs.
 * - Define a mapping of cell IDs within a module (center of module is the origin)
 *   - Use TH2Poly to do the Hexagon tiling in p,q space
 * - Define center of modules with respect to center of layer in p,q space
 *   - currently this is assumed to be the same within all layers BUT will
 *     depend on geometry parameters like the gap between modules
 * - Define center of layers (INCLUDING x,y position)
 *   - some versions of the geometry shift layers off the z axis to 
 *     cover the gaps between modules
 *   - Will need to handle the conversion between x,y and p,q axes
 *
 * ## THIS GRID:
 * - column-to-column distance in a grid such as ours is 2r = sqrt(3)R.
 * - row-to-row distance is 1.5R (easy to observe that twice that distance = 3R)
 *
 * The cell radius is calculated from the total number of center-to-corner cell
 * radii that span the module height. This count can have fractional counts to
 * account for the fractions of cell radii at the module edges.
 */
class EcalGeometry : public framework::ConditionsObject {
 public:
  static constexpr const char* CONDITIONS_OBJECT_NAME{"EcalGeometry"};

  /**
   * Class destructor.
   *
   * Does nothing because the member variables clean up
   * themselves.
   */
  virtual ~EcalGeometry() = default;

  /**
   * Get a cell's ID number from its position
   *
   * @param[in] x global x position [mm]
   * @param[in] y global y position [mm]
   * @param[in] z global z position [mm]
   * @return EcalID of the cell
   */
  EcalID getID(double x, double y, double z) const;

  /**
   * Get a cell's ID from its x,y global position and layer number
   *
   * This is faster as long as we trust that the layer positions between
   * GDML and the configured parameters of this class match.
   *
   * @param[in] x global x position [mm]
   * @param[in] y global y position [mm]
   * @param[in] layer_id integer ID of the layer the hit is in
   * @return EcalID of the cell
   */
  EcalID getID(double x, double y, int layer_id) const;

  /**
   * Get a cell's ID from its x,y global position and layer/module numbers
   * as deduced from GDML copy numbers.
   *
   * This is the fastest option but we need to carefully validated that the
   * layer and module positions between the GDML and the configured parameters
   * of this class match.
   *
   * @param[in] x global x position [mm]
   * @param[in] y global y position [mm]
   * @param[in] layer_id integer ID of the layer the hit is in
   * @param[in] module_id integer ID of the module the hit is in
   * @return EcalID of the cell
   */
  EcalID getID(double x, double y, int layer_id, int module_id) const;

  /**
   * Get a cell's position from its ID number
   *
   * std::tuple is useful here because you can use C++17's pattern
   * matching to use code like the following
   * ```cpp
   * auto [x,y,z] = geometry.getPosition(id);
   * ```
   */
  std::tuple<double,double,double> getPosition(EcalID id) const;

  /**
   * Get a cell's position within a module
   */
  std::pair<double,double> getPositionInModule(int cell_id) const;

  /**
   * Get the z-coordinate given the layer id
   *
   * @param[in] layer int layer id
   * @return z-coordinate of the input sensitive layer
   */
  double getZPosition(int layer) const {
    return std::get<2>(layer_pos_xy_.at(layer));
  }

  /**
   * Get the number of layers in the Ecal Geometry
   *
   * @returns number fo layers in geometry
   */
  int getNumLayers() const { return layer_pos_xy_.size(); }

  /**
   * Get the number of modules in the Ecal flower
   *
   * @note This number is hard-coded to reflect the fact
   * that it is also hard-coded in the buildModuleMap function.
   * If a future geometry adds more modules (perhaps "triangles"
   * to fill in rectangular block in the HCal that the ECal is
   * in), then buildModuleMap and this function will need to
   * be modified.
   *
   * @returns number of modules
   */
  int getNumModulesPerLayer() const { return 7; }

  /**
   * Get the number of cells in each module of the Ecal Geometry
   *
   * @note This assumes that all modules are the full high-density
   * hexagons from CMS (no triangles!)
   */
  int getNumCellsPerModule() const { return cell_id_in_module_.GetNumberOfBins(); }

  /**
   * Get the Nearest Neighbors of the input ID
   *
   * @param id id to get
   * @return list of EcalID that are the inputs nearest neighbors
   */
  std::vector<EcalID> getNN(EcalID id) const {
    auto list = NNMap_.at(EcalID(0, id.module(), id.cell()));
    for (auto& flat : list) flat = EcalID(id.layer(), flat.module(), flat.cell());
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
    for (auto& flat : list) flat = EcalID(id.layer(), flat.module(), flat.cell());
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
   * Get a reference to the TH2Poly used for Cell IDs.
   *
   * @note This is only helpful in the use case
   * where you want to print the cell ID <-> cell position
   * map. DO NOT USE THIS OTHERWISE.
   *
   * @return pointer to member variable cell_id_in_module_
   */
  TH2Poly* getCellPolyMap() const { 
    for (auto const& [cell_id, cell_center] : cell_pos_in_module_) {
      cell_id_in_module_.Fill(cell_center.first, cell_center.second, cell_id);
    }
    return &cell_id_in_module_; 
  }

  static EcalGeometry* debugMake(const framework::config::Parameters& p) {
    return new EcalGeometry(p);
  }

 private:
  /**
   * Class constructor, for use only by the provider
   *
   * @param ps Parameters to configure the EcalGeometry
   */
  EcalGeometry(const framework::config::Parameters& ps);
  friend class ecal::EcalGeometryProvider;

  /**
   * Constructs the positions of the layers in world coordinates
   */
  void buildLayerMap();

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
   * @note We require two cells to be in the same layer in order to be nearest 
   * neighbors or next-nearest neighbors.
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
   * Distance to module edge, and whether cell is on edge of module.
   *
   * @TODO Use getNN()/getNNN() + isEdgeCell() to expand functionality.
   *
   * @param[in] cellModuleID EcalId where all we care about is module and cell
   * @return distance to edge of the module
   */
  double distanceToEdge(EcalID id) const;

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
   * @note This function is in p,q space so any rotations need to be performed
   * before calling this function.
   *
   * @param[in] normX X-coordinate relative to module hexagon divided by maximum
   * hexagon radius
   * @param[in] normY Y-coordinate relative to module hexagon divided by maximum
   * hexagon radius
   * @return true if (normX,normY) is within the hexagon centered at the origin
   * with maximum radius 1.
   */
  bool isInside(double normX, double normY) const;

 private:
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
   * indicator of geometry orientation 
   * if true, flower shape's corners side (ie: side with two modules) is at the top
   */
  bool cornersSideUp_;

  /**
   * shift of layers in the x-direction [mm]
   */
  double layer_shift_x_;

  /**
   * shift of layers in the y-direction [mm]
   */
  double layer_shift_y_;

  /**
   * shift odd layers
   *
   * odd layers are the high-z layer in each bi-layer
   *
   * i.e. We will shift if layer_id_ % 2 == 1
   */
  bool layer_shift_odd_;

  /**
   * shift odd bi layers
   *
   * NOT IMPLEMENTED IN GDML
   *
   * This shifts the bi-layer grouping of two sensitive
   * layers together.
   *
   * i.e. We will shift if (layer_id_ / 2) % 2 == 1
   *
   * where it is integer division.
   */
  bool layer_shift_odd_bilayer_;

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

 private:
  /// verbosity
  int verbose_;

  /**
   * Position of layer centers in world coordinates
   * (uses layer ID as key)
   */
  std::map<int, std::tuple<double,double,double>> layer_pos_xy_;

  /**
   * Postion of module centers relative to the center of the layer
   * in world coordinates
   *
   * (uses module ID as key)
   */
  std::map<int, std::pair<double, double>> module_pos_xy_;

  /**
   * Position of cell centers relative to center of module in
   * p,q space.
   *
   * use cell ID as key
   */
  std::map<int, std::pair<double, double>> cell_pos_in_module_;

  /**
   * Position of cell centers relative to center of layer in world
   * coordinates.
   *
   * @note Layer shifts are NOT included in this map since they depend
   * on the layer number!!
   *
   * Uses EcalID with layer set to zero as key.
   */
  std::map<EcalID, std::pair<double,double>> cell_pos_in_layer_;

  /**
   * Position of cell centers relative to world geometry 
   *
   * This is where we convert p,q (flower) space into x,y (world) space
   * by calculating the z-location as well as including rotations and
   * shifts when converting from p,q to x,y.
   *
   * The key is the full EcalID and the value is the x,y,z tuple.
   */
  std::map<EcalID, std::tuple<double,double, double>> cell_global_pos_;

  /**
   * Map of cell ID to neighboring cells 
   *
   * The EcalID's in this map all have layer ID set to zero.
   */
  std::map<EcalID, std::vector<EcalID>> NNMap_;

  /**
   * Map of cell ID to neighbors of neighbor cells
   *
   * The EcalID's in this map all have layer ID set to zero.
   */
  std::map<EcalID, std::vector<EcalID>> NNNMap_;

  /**
   * Honeycomb Binning from ROOT
   *
   * Needs to be mutable because ROOT doesn't have good const handling
   *
   * Lookup a cell ID using its position relative to the center of
   * the module in p,q space.
   */
  mutable TH2Poly cell_id_in_module_;
};

}  // namespace ldmx

#endif
