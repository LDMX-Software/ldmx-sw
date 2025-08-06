#include "DetDescr/EcalGeometry.h"

namespace ldmx {

static double distance(const std::pair<double, double>& p1,
                       const std::pair<double, double>& p2) {
  return sqrt((p1.first - p2.first) * (p1.first - p2.first) +
              (p1.second - p2.second) * (p1.second - p2.second));
}

[[maybe_unused]] static double distance(
    const std::tuple<double, double, double>& p1,
    const std::tuple<double, double, double>& p2) {
  return sqrt((std::get<0>(p1) - std::get<0>(p2)) *
                  (std::get<0>(p1) - std::get<0>(p2)) +
              (std::get<1>(p1) - std::get<1>(p2)) *
                  (std::get<1>(p1) - std::get<1>(p2)) +
              (std::get<2>(p1) - std::get<2>(p2)) *
                  (std::get<2>(p1) - std::get<2>(p2)));
}

/**
 * Rotation from (x,y) to (p,q)
 */
static void rotate(double& p, double& q) {
  double tmp{p};
  p = -q;
  q = tmp;
}

/**
 * Rotation from (p,q) to (x,y)
 */
static void unrotate(double& p, double& q) {
  double tmp{p};
  p = q;
  q = -tmp;
}

EcalGeometry::EcalGeometry(const framework::config::Parameters& ps)
    : framework::ConditionsObject(EcalGeometry::CONDITIONS_OBJECT_NAME) {
  layerZPositions_ = ps.getParameter<std::vector<double>>("layerZPositions");
  ecalFrontZ_ = ps.getParameter<double>("ecalFrontZ");
  moduler_ = ps.getParameter<double>("moduleMinR");
  gap_ = ps.getParameter<double>("gap");
  nCellRHeight_ = ps.getParameter<double>("nCellRHeight");
  cornersSideUp_ = ps.getParameter<bool>("cornersSideUp");
  layer_shift_x_ = ps.getParameter<double>("layer_shift_x");
  layer_shift_y_ = ps.getParameter<double>("layer_shift_y");
  layer_shift_odd_ = ps.getParameter<bool>("layer_shift_odd");
  layer_shift_odd_bilayer_ = ps.getParameter<bool>("layer_shift_odd_bilayer");
  si_thickness_ = ps.getParameter<double>("si_thickness");

  if (layer_shift_odd_ and layer_shift_odd_bilayer_) {
    EXCEPTION_RAISE("BadConf",
                    "Cannot shift both odd sensitive layers and odd bilayers");
  }

  moduleR_ = moduler_ * (2 / sqrt(3));
  cellR_ = 2 * moduler_ / nCellRHeight_;
  cellr_ = (sqrt(3.) / 2.) * cellR_;

  ldmx_log(debug) << "Building module map with gap " << std::setprecision(2)
                  << gap_ << ", nCellRHeight " << nCellRHeight_
                  << ",  min/max radii of cell " << cellr_ << " / " << cellR_
                  << ", and module " << moduler_ << " / " << moduleR_;

  buildLayerMap();
  buildModuleMap();
  buildCellMap();
  buildCellModuleMap();
  buildNeighborMaps();

  ldmx_log(trace) << "Geometry fully constructed";
}

EcalID EcalGeometry::getID(double x, double y, double z, bool fallible) const {
  int layer_id{-1};
  for (const auto& [lid, layer_xyz] : layer_pos_xy_) {
    // check if the z coordinate is within the layer thickness
    if (std::abs(std::get<2>(layer_xyz) - z) < si_thickness_) {
      layer_id = lid;
      break;
    }
  }
  if (layer_id < 0) {
    if (fallible) {
      return ldmx::EcalID(0);
    } else {
      EXCEPTION_RAISE("BadConf", "z = " + std::to_string(z) +
                                     " mm is not within any"
                                     " of the configured layers.");
    }
  }
  return getID(x, y, layer_id, fallible);
}

EcalID EcalGeometry::getID(double x, double y, int layer_id,
                           bool fallible) const {
  // now assume we know the layer
  // shift to center of layer
  // and convert to flower coordinates
  double p{x - std::get<0>(layer_pos_xy_.at(layer_id))},
      q{y - std::get<1>(layer_pos_xy_.at(layer_id))};

  // deduce module ID
  //    there are only 7 modules so we just loop through them
  //    all and pick out the module ID that we are inside of

  int module_id{-1};
  for (auto const& [mid, module_xy] : module_pos_xy_) {
    double probe_x{p - module_xy.first}, probe_y{q - module_xy.second};
    if (cornersSideUp_) rotate(probe_x, probe_y);
    if (isInside(probe_x / moduleR_, probe_y / moduleR_)) {
      module_id = mid;
      break;
    }
  }

  if (module_id < 0) {
    if (fallible) {
      return ldmx::EcalID(0);
    } else {
      EXCEPTION_RAISE(
          "BadConf",
          TString::Format(
              "Coordinates relative to layer (p,q) = (%.2f, %.2f) mm "
              "derived from world coordinates (%.2f, %.2f) mm with layer = %d "
              "are not inside any module.",
              p, q, x, y, layer_id)
              .Data());
    }
  }

  return getID(x, y, layer_id, module_id);
}

EcalID EcalGeometry::getID(double x, double y, int layer_id, int module_id,
                           bool fallible) const {
  // now assume we know the layer and module
  // shift to center of layer and then center of module
  double p{x - std::get<0>(layer_pos_xy_.at(layer_id)) -
           module_pos_xy_.at(module_id).first},
      q{y - std::get<1>(layer_pos_xy_.at(layer_id)) -
        module_pos_xy_.at(module_id).second};

  // need to rotate
  if (cornersSideUp_) rotate(p, q);

  // deduce cell ID
  int cell_id = cell_id_in_module_.FindBin(p, q) - 1;

  if (cell_id < 0) {
    if (fallible) {
      return ldmx::EcalID(0);
    } else {
      EXCEPTION_RAISE(
          "BadConf",
          TString::Format(
              "Relative cell coordinates (%.2f, %.2f) mm "
              "derived from world coordinates (%.2f, %.2f) mm with layer = %d "
              "and module = %d are outside module hexagon",
              p, q, x, y, layer_id, module_id)
              .Data());
    }
  }

  return EcalID(layer_id, module_id, cell_id);
}

std::tuple<double, double, double> EcalGeometry::getPosition(EcalID id) const {
  return cell_global_pos_.at(id);
}

std::pair<double, double> EcalGeometry::getPositionInModule(int cell_id) const {
  auto pq = cell_pos_in_module_.at(cell_id);

  // going from (p,q) to (x,y) is a unrotate
  if (cornersSideUp_) unrotate(pq.first, pq.second);

  return pq;
}

void EcalGeometry::buildLayerMap() {
  ldmx_log(debug) << "Building layer map with " << layerZPositions_.size()
                  << " layers";
  if (layer_shift_odd_ or layer_shift_odd_bilayer_) {
    if (layer_shift_odd_) {
      ldmx_log(debug) << "Shifting odd layers by (x,y) = (" << layer_shift_x_
                      << ", " << layer_shift_y_ << ") mm";
    } else {
      ldmx_log(debug) << "Shifting odd bilayers by (x,y) = (" << layer_shift_x_
                      << ", " << layer_shift_y_ << ") mm";
    }
  }

  for (std::size_t i_layer{0}; i_layer < layerZPositions_.size(); ++i_layer) {
    // default is centered on z-axis
    double x{0}, y{0}, z{ecalFrontZ_ + layerZPositions_.at(i_layer)};
    if (layer_shift_odd_ and (i_layer % 2 == 1)) {
      x += layer_shift_x_;
      y += layer_shift_y_;
    } else if (layer_shift_odd_bilayer_ and ((i_layer / 2) % 2 == 1)) {
      x += layer_shift_x_;
      y += layer_shift_y_;
    }
    ldmx_log(trace) << "    Layer " << i_layer << " has center at (" << x
                    << ", " << y << ", " << z << ") mm";

    layer_pos_xy_[i_layer] = std::make_tuple(x, y, z);
  }
}

void EcalGeometry::buildModuleMap() {
  static const double C_PI = 3.14159265358979323846;

  // the center module (module_id == 0) has always been (and will always be?)
  //  centered with respect to the layer position
  module_pos_xy_[0] = std::pair<double, double>(0., 0.);

  // for flat-side-up designs (v12 and earlier), the modules are numbered 1 on
  // positive y-axis and then counter-clockwise until 6
  //
  // for corner-side-up designes (v13 and later), the modules are numbered 1 on
  // positive x-axis and then counter-clockwise until 6.
  for (unsigned id = 1; id < 7; id++) {
    // flat-side-up
    double x = (2. * moduler_ + gap_) * sin((id - 1) * (C_PI / 3.));
    double y = (2. * moduler_ + gap_) * cos((id - 1) * (C_PI / 3.));
    if (cornersSideUp_) {
      // re-calculating to make sure centers match GDML
      x = (2. * moduler_ + gap_) * cos((id - 1) * (C_PI / 3.));
      y = -(2. * moduler_ + gap_) * sin((id - 1) * (C_PI / 3.));
    }
    module_pos_xy_[id] = std::pair<double, double>(x, y);
    ldmx_log(trace) << "    Module " << id << " is centered at (x,y) = " << "("
                    << x << ", " << y << ") mm";
  }
}

void EcalGeometry::buildCellMap() {
  /** STRATEGY
   * use ROOT HoneyComb method (build large hex grid, copy polygons that cover
   * module)
   *
   * the sneaky bit is that the center of TH2Poly::HoneyComb is the center of a
   * cell while the center of one of our modules is the corner between three
   * cells. This means some of the hexagons along the edges of the module will
   * _not_ be regular and need to stretched/squashed a bit.
   *
   * REMEMBER:
   * We are in p,q space for the cells_in_module_ map. i.e.
   *
   *   ^
   *   | q axis
   *   __
   *  /  \ -- > p axis
   *  \__/
   *
   */
  TH2Poly gridMap;

  // make hexagonal grid [boundary is rectangle] larger than the module
  double gridMinP = 0., gridMinQ = 0.;  // start at the origin
  int numPCells = 0, numQCells = 0;

  // first x-cell is only a half
  gridMinP -= cellr_;
  numPCells++;
  while (gridMinP > -1 * moduleR_) {
    gridMinP -= 2 * cellr_;  // decrement x by cell center-to-flat diameter
    numPCells++;
  }
  while (gridMinQ > -1 * moduler_) {
    // decrement y by cell center-to-corner radius
    //  alternate between a full corner-to-corner diameter
    //  and a side of a cell (center-to-corner radius)
    if (numQCells % 2 == 0)
      gridMinQ -= 1 * cellR_;
    else
      gridMinQ -= 2 * cellR_;
    numQCells++;
  }
  // only counted one half of the cells
  numPCells *= 2;
  numQCells *= 2;

  gridMap.Honeycomb(gridMinP, gridMinQ, cellR_, numPCells, numQCells);

  ldmx_log(trace) << std::setprecision(2)
                  << "Building buildCellMap with cell rmin: " << cellr_
                  << " cell rmax: " << cellR_ << " (gridMinP,gridMinQ) = ("
                  << gridMinP << "," << gridMinQ << ")"
                  << " (numPCells,numQCells) = (" << numPCells << ","
                  << numQCells << ")";

  // copy cells lying within module boundaries to a module grid
  TListIter next(gridMap.GetBins());  // a TH2Poly is a TList of TH2PolyBin
  TH2PolyBin* polyBin = 0;
  TGraph* poly = 0;  // a polygon returned by TH2Poly is a TGraph
  // cells_in_module_ IDs go from 0 to N-1, not equal to original grid cell ID
  int cell_id = 0;
  while ((polyBin = (TH2PolyBin*)next())) {
    // these bins are coming from the honeycomb
    //  grid so we assume that they are regular
    //  hexagons i.e. has 6 vertices
    poly = (TGraph*)polyBin->GetPolygon();

    // decide whether to copy polygon to new map.
    // use all vertices in case of cut-off edge polygons.
    int numVerticesInside = 0;
    double vertex_p[6], vertex_q[6];  // vertices of the cell
    bool isinside[6];                 // which vertices are inside
    ldmx_log(trace) << "    Cell vertices";
    for (unsigned i = 0; i < 6; i++) {
      poly->GetPoint(i, vertex_p[i], vertex_q[i]);
      ldmx_log(trace) << "      vtx # " << i;
      ldmx_log(trace) << "      vtx p,q " << vertex_p[i] << " " << vertex_q[i];

      isinside[i] = isInside(vertex_p[i] / moduleR_, vertex_q[i] / moduleR_);
      if (isinside[i]) numVerticesInside++;
    }

    if (numVerticesInside > 1) {
      // Include this cell if more than one of its vertices is inside the module
      // hexagon
      double actual_p[8], actual_q[8];
      int num_vertices{0};
      if (numVerticesInside < 6) {
        // This cell is stradling the edge of the module
        // and is NOT cleanly cut by module edge

        ldmx_log(trace) << "    Polygon " << cell_id
                        << " has vertices poking out of module hexagon.";

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
            double edge_origin_p, edge_origin_q;
            double edge_dest_p, edge_dest_q;
            if (vertex_p[i] < -moduleR_ / 2.) {
              // sloped edge on negative-x side
              edge_origin_p = -1. * moduleR_;
              edge_origin_q = 0.;
              edge_dest_p = -0.5 * moduleR_;
              edge_dest_q = moduler_;
            } else if (vertex_p[i] > moduleR_ / 2.) {
              // sloped edge on positive-x side
              edge_origin_p = 0.5 * moduleR_;
              edge_origin_q = moduler_;
              edge_dest_p = moduleR_;
              edge_dest_q = 0.;
            } else {
              // flat edge at top
              edge_origin_p = 0.5 * moduleR_;
              edge_origin_q = moduler_;
              edge_dest_p = -0.5 * moduleR_;
              edge_dest_q = moduler_;
            }
            // flip to bottom half if below x-axis
            if (vertex_q[i] < 0) {
              edge_dest_q *= -1;
              edge_origin_q *= -1;
            }

            // get edge slope vector
            double edge_slope_p = edge_dest_p - edge_origin_p;
            double edge_slope_q = edge_dest_q - edge_origin_q;

            ldmx_log(trace)
                << "Vertex " << i
                << " is inside and adjacent to a vertex outside the module.";
            ldmx_log(trace) << "Working on edge with slope (" << edge_slope_p
                            << "," << edge_slope_q << ")" << " and origin ("
                            << edge_origin_p << "," << edge_origin_q << ")";

            // project vertices adjacent to the vertex outside the module onto
            // the module edge
            double projection_factor =
                ((vertex_p[i] - edge_origin_p) * edge_slope_p +
                 (vertex_q[i] - edge_origin_q) * edge_slope_q) /
                (edge_slope_p * edge_slope_p + edge_slope_q * edge_slope_q);

            double proj_p = edge_origin_p + projection_factor * edge_slope_p;
            double proj_q = edge_origin_q + projection_factor * edge_slope_q;

            if (not isinside[up]) {
              // the next point is outside
              actual_p[num_vertices] = vertex_p[i];
              actual_q[num_vertices] = vertex_q[i];
              actual_p[num_vertices + 1] = proj_p;
              actual_q[num_vertices + 1] = proj_q;
            } else {
              // the previous point was outside
              actual_p[num_vertices] = proj_p;
              actual_q[num_vertices] = proj_q;
              actual_p[num_vertices + 1] = vertex_p[i];
              actual_q[num_vertices + 1] = vertex_q[i];
            }
            num_vertices += 2;

            ldmx_log(trace) << "New Vertex " << i << " : (" << vertex_p[i]
                            << "," << vertex_q[i] << ")";

          } else {
            actual_p[num_vertices] = vertex_p[i];
            actual_q[num_vertices] = vertex_q[i];
            num_vertices++;
          }  // should we project or not
        }  // loop through vertices
      } else {
        // all 6 inside, just copy the vertices over
        num_vertices = 6;
        for (int i = 0; i < 6; i++) {
          actual_p[i] = vertex_p[i];
          actual_q[i] = vertex_q[i];
        }
      }  // if numVerticesInside is less than 5

      // TH2Poly needs to have its own copy of the polygon TGraph
      //  otherwise, we get a seg fault when EcalGeometry is destructed
      //  because the polygon that was copied over from gridMap is deleted at
      //  the end of this function
      cell_id_in_module_.AddBin(num_vertices, actual_p, actual_q);

      /**
       * TODO is this needed?
       */
      double p = (polyBin->GetXMax() + polyBin->GetXMin()) / 2.;
      double q = (polyBin->GetYMax() + polyBin->GetYMin()) / 2.;

      ldmx_log(trace) << "    Copying poly with ID " << polyBin->GetBinNumber()
                      << " and (p,q) (" << std::setprecision(2) << p << "," << q
                      << ")";
      // save cell location as center of ENTIRE hexagon
      cell_pos_in_module_[cell_id] = std::pair<double, double>(p, q);
      ++cell_id;  // incrememnt cell ID
    }  // if num vertices inside is > 1
  }  // loop over larger grid spanning module hexagon
  return;
}

void EcalGeometry::buildCellModuleMap() {
  ldmx_log(trace) << "Building cellModule position map";
  /// construct map of cell centers relative to layer center
  for (auto const& [module_id, module_xy] : module_pos_xy_) {
    for (auto const& [cell_id, cell_pq] : cell_pos_in_module_) {
      double cell_x{cell_pq.first}, cell_y{cell_pq.second};
      // convert from (p,q) to (x,y) space
      // when the corners are not up, x = p and y = q
      // so no transformation needs to be done
      if (cornersSideUp_) unrotate(cell_x, cell_y);

      // calculate cell's pq relative to entire layer center
      auto cell_rel_to_layer =
          std::make_pair(module_xy.first + cell_x, module_xy.second + cell_y);

      // now add the layer-center values to get the global position
      // of the cell
      cell_pos_in_layer_[EcalID(0, module_id, cell_id)] = cell_rel_to_layer;
    }
  }

  /// construct map of global cell centers relative to target center
  for (auto const& [layer_id, layer_xyz] : layer_pos_xy_) {
    for (auto const& [flat_id, rel_to_layer] : cell_pos_in_layer_) {
      // now add the layer-center values to get the global position
      // of the cell
      cell_global_pos_[EcalID(layer_id, flat_id.module(), flat_id.cell())] =
          std::make_tuple(rel_to_layer.first + std::get<0>(layer_xyz),
                          rel_to_layer.second + std::get<1>(layer_xyz),
                          std::get<2>(layer_xyz));
    }
  }

  ldmx_log(trace) << "Cell module map contains " << cell_global_pos_.size()
                  << " entries. ";
  return;
}

void EcalGeometry::buildNeighborMaps() {
  /** STRATEGY
   *
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
  ldmx_log(trace) << "Building Nearest and Next-Nearest Neighbor maps";

  NNMap_.clear();
  NNNMap_.clear();
  for (auto const& [center_id, center_xyz] : cell_pos_in_layer_) {
    for (auto const& [probe_id, probe_xyz] : cell_pos_in_layer_) {
      /// do distance calculation
      double dist = distance(probe_xyz, center_xyz);
      if (dist > 1 * cellr_ && dist <= 3. * cellr_) {
        NNMap_[center_id].push_back(probe_id);
      } else if (dist > 3. * cellr_ && dist <= 4.5 * cellr_) {
        NNNMap_[center_id].push_back(probe_id);
      }
    }
    // Keeping this commented out until the SimIDs string method is implemented
    // ldmx_log(debug) << "  Found " << NNMap_[center_id].size() << " NN and "
    // << NNNMap_[center_id].size() << " NNN for cell " << center_id;
  }
  return;
}

double EcalGeometry::distanceToEdge(EcalID id) const {
  // https://math.stackexchange.com/questions/1210572/find-the-distance-to-the-edge-of-a-hexagon
  std::pair<double, double> cellLocation = cell_pos_in_module_.at(id.cell());
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

bool EcalGeometry::isInside(double normX, double normY) const {
  ldmx_log(trace) << std::fixed << std::setprecision(2)
                  << "Checking if normXY=(" << normX << "," << normY
                  << ") is inside.";
  normX = fabs(normX), normY = fabs(normY);
  double xvec = -1, yvec = -1. / sqrt(3);
  double xref = 0.5, yref = sqrt(3) / 2.;
  if ((normX > 1.) || (normY > yref)) {
    ldmx_log(trace) << "They are outside quadrant.";
    return false;
  }
  double dotProd = (xvec * (normX - xref) + yvec * (normY - yref));
  ldmx_log(trace) << std::fixed << std::setprecision(2)
                  << "They are inside quadrant. Dot product (>0 is inside): "
                  << dotProd;
  return (dotProd > 0.);
}

}  // namespace ldmx
