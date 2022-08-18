#include "DetDescr/EcalGeometry.h"

#include "TGeoPolygon.h"
#include "TGraph.h"
#include "TList.h"
#include "TMath.h"
#include "TMultiGraph.h"

#include <assert.h>
#include <iomanip>
#include <iostream>

namespace ldmx {

EcalGeometry::EcalGeometry(const framework::config::Parameters& ps)
    : framework::ConditionsObject(EcalGeometry::CONDITIONS_OBJECT_NAME) {
  layerZPositions_ = ps.getParameter<std::vector<double>>("layerZPositions");
  ecalFrontZ_ = ps.getParameter<double>("ecalFrontZ");
  moduler_ = ps.getParameter<double>("moduleMinR");
  gap_ = ps.getParameter<double>("gap");
  nCellRHeight_ = ps.getParameter<double>("nCellRHeight");
  verbose_ = ps.getParameter<int>("verbose");
  cornersSideUp_ = ps.getParameter<bool>("cornersSideUp");
  layer_shift_x_ = ps.getParameter<double>("layer_shift_x");
  layer_shift_y_ = ps.getParameter<double>("layer_shift_y");
  layer_shift_odd_ = ps.getParameter<bool>("layer_shift_odd");
  layer_shift_odd_bilayer_ = ps.getParameter<bool>("layer_shift_odd_bilayer");

  if (layer_shift_odd_ and layer_shift_odd_bilayer_) {
    EXCEPTION_RAISE("BadConf",
        "Cannot shift both odd sensitive layers and odd bilayers");
  }
  
  moduleR_ = moduler_ * (2 / sqrt(3));
  cellR_ = 2 * moduler_ / nCellRHeight_;
  cellr_ = (sqrt(3.) / 2.) * cellR_;

  if (verbose_ > 0) {
    std::cout << std::endl
              << "[EcalGeometry] Verbosity set in header to " << verbose_
              << std::endl;
    std::cout << "     Building module map with gap " << std::setprecision(2)
              << gap_ << ", nCellRHeight " << nCellRHeight_
              << ",  min/max radii of cell " << cellr_ << " / " << cellR_
              << ", and module " << moduler_ << " / " << moduleR_ << std::endl;
  }

  buildLayerMap();
  buildModuleMap();
  buildCellMap();
  buildCellModuleMap();
  buildNeighborMaps();

  if (verbose_ > 0) {
    std::cout << std::endl;
  }
}

EcalID EcalGeometry::getID(double x, double y, double z) const {
  static const double tolerance = 1.; // thickness of Si
  int layer_id{-1};
  for (const auto& [lid, layzer_xyz] : layer_pos_xy_) {
    if (abs(std::get<2>(layer_xyz)-z) < tolerance) {
      layer_id = lid;
      break;
    }
  }
  if (layer_id < 0) {
    EXCEPTION_RAISE("BadConf",
        "z = "+std::to_string(z)+" mm is not within any"
        " of the configured layers.");
  }
  // now assume we know the layer
  // shift to center of layer
  // and convert to flower coordinates
  double p{x - std::get<0>(layer_pos_xy_.at(layer_id)}, 
         q{y - std::get<1>(layer_pos_xy_.at(layer_id)};
  if (cornersSideUp_) {
    // need to rotate
    double tmp{p};
    p = q;
    q = -tmp;
  }

  // deduce module ID
  //  the hexagon center we are closest to is the module we are in
  //  additionally we can shorten this loop because if we are within 
  //  a small radius of the hexagon center, there is no way we are in
  //  another hexagon

  int module_id{-1};
  double closest = 1e6;
  for (auto const& [mid, module_pq] : module_pos_pq_) {
    double dist = sqrt((p - module_pq.first)*(p-module_pq.first) +
        (q - module_pq.second)*(q-module_pq.second));
    if (dist < moduler_) {
      module_id = mid;
      break;
    }
    if (dist < closest) {
      module_id = mid;
      closests = dist;
    }
  }

  if (module_id < 0) {
    EXCEPTION_RAISE("BadConf",
        "Unable to deduce module relative to layer");
  }

  // shift to center of deduced module
  p -= module_pos_pq_.at(module_id).first;
  q -= module_pos_pq_.at(module_id).second;

  // deduce cell ID
  int cell_id = cell_id_in_module_.FindBin(p,q) - 1;
  
  if (cell_id < 0) {
    EXCEPTION_RAISE("BadConf",
        "Relative cell coordinates are outside module hexagon");
  }

  return EcalID(layer_id,module_id,cell_id);
}

std::tuple<double,double,double> getPosition(EcalID id) const {
  return cell_global_pos_.at(id);
}

void EcalGeometry::buildLayerMap() {
  if (verbose_ > 0) {
    std::cout << "[EcalGeometry::buildLayerMap] "
      << " Building layer map with " 
      << layerZPositions_.size() << " layers" << std::endl;;
    if (layer_shift_odd_ or layer_shift_odd_bilayer_) {
      std::cout << "  shifting odd ";
      if (layer_shift_odd_) std::cout << "layers";
      else std::cout << "bilayers";
      std::cout << " by (x,y) = (" 
        << layer_shift_x_ << ", " << layer_shift_y_ << ") mm"
        << std::endl;
    } else {
      std::cout << "  without any shifting" << std::endl;
    }
  }
  for (std::size_t i_layer{0}; i_layer < layerZPositions_.size(); ++i_layer) {
    // default is centered on z-axis
    double x{0},y{0},z{ecalFrontZ_+layerZPositions_.at(i_layer)};
    if (layer_shift_odd_ and (i_layer % 2 == 1)) {
      x += layer_shift_x_;
      y += layer_shift_y_;
    } else if (layer_shift_odd_bilayer_ and ((i_layer_/2) % 2 == 1)) {
      x += layer_shift_x_;
      y += layer_shift_y_;
    }
    if (verbose_ > 2) {
      std::cout << "    Layer " << i_layer
        << " has center at (" << x << ", " << y << ", " << z << ") mm"
        << std::endl;
    }
    layer_pos_xy_[i_layer] = std::make_tuple(x,y,z);  
  } 
}

void EcalGeometry::buildModuleMap() {
  if (verbose_ > 0) {
    std::cout << "[EcalGeometry::buildModuleMap] "
      << "Building module position map for module min r of "
      << moduler_ << " and gap of " << gap_ << std::endl;
  }

  double C_PI = 3.14159265358979323846;  // or TMath::Pi(), #define, atan(), ...
  // module IDs are 0 for ecal center, 1 at right, and counterclockwise till 6 at 11 o'clock
  module_pos_pq_[0] = std::pair<double, double>(0., 0.);
  for (unsigned id = 1; id < 7; id++) {
    double p = (2. * moduler_ + gap_) * sin((id - 1) * (C_PI / 3.));
    double q = (2. * moduler_ + gap_) * cos((id - 1) * (C_PI / 3.));
    module_pos_pq_[id] = std::pair<double, double>(p, q);
    if (verbose_ > 2)
      std::cout << "    Module " << id << " is centered at (p,q) = "
        << "(" << p << ", " << q << ") mm" << std::endl;
  }
  if (verbose_ > 0) std::cout << std::endl;
}

void EcalGeometry::buildCellMap() {
  /** STRATEGY
   * use ROOT HoneyComb method (build large hex grid, copy polygons that cover module)
   * 
   * the sneaky bit is that the center of TH2Poly::HoneyComb is the center of a cell
   * while the center of one of our modules is the corner between three cells. 
   * This means some of the hexagons along the edges of the module will _not_ be regular
   * and need to stretched/squashed a bit.
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
    if (numQCells % 2 == 0) gridMinQ -= 1 * cellR_;
    else gridMinQ -= 2 * cellR_;
    numQCells++;
  }
  // only counted one half of the cells
  numPCells *= 2;
  numQCells *= 2;
  
  gridMap.Honeycomb(gridMinP, gridMinQ, cellR_, numPCells, numQCells);
  
  if (verbose_ > 0) {
    std::cout << std::endl;
    std::cout << std::setprecision(2) << "[buildCellMap] cell rmin: " << cellr_
      << " cell rmax: " << cellR_ << " (gridMinP,gridMinQ) = ("
      << gridMinP << "," << gridMinQ << ")"
      << " (numPCells,numQCells) = (" << numPCells << "," << numQCells
      << ")" << std::endl;
  }
        
  // copy cells lying within module boundaries to a module grid
  TListIter next(gridMap.GetBins());  // a TH2Poly is a TList of TH2PolyBin
  TH2PolyBin* polyBin = 0;
  TGraph* poly = 0;   // a polygon returned by TH2Poly is a TGraph
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
    if (verbose_ > 2) std::cout << "    Cell vertices" << std::endl;
    for (unsigned i = 0; i < 6; i++) {
      poly->GetPoint(i, vertex_p[i], vertex_q[i]);
      if (verbose_ > 2) {
        std::cout << "      vtx # " << i << std::endl;
        std::cout << "      vtx p,q " << vertex_p[i] << " " << vertex_q[i]
          << std::endl;
      }
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
      
        if (verbose_ > 1) {
          std::cout << "    Polygon " << ecalMapID
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
            
            if (verbose_ > 2) {
                  std::cout << "Vertex " << i
                    << " is inside and adjacent to a vertex outside the module."
                    << std::endl;
                  std::cout << "Working on edge with slope (" << edge_slope_p << ","
                            << edge_slope_q << ")"
                            << " and origin (" << edge_origin_p << ","
                            << edge_origin_q << ")" << std::endl;
            }
            
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
            
            if (verbose_ > 2) {
              std::cout << "New Vertex " << i << " : (" << vertex_p[i] << ","
                << vertex_q[i] << ")" << std::endl;
            }
          } else {
            actual_p[num_vertices] = vertex_p[i];
            actual_q[num_vertices] = vertex_q[i];
            num_vertices++;
          }  // should we project or not
        }    // loop through vertices
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
      if (verbose_ > 1) {
        std::cout << "    Copying poly with ID "
            << polyBin->GetBinNumber() << " and (p,q) ("
            << std::setprecision(2) << p << "," << q << ")" << std::endl;
      }
      // save cell location as center of ENTIRE hexagon
      cell_pos_in_module_[cell_id] = std::pair<double, double>(p, q);
      ++cell_id; // incrememnt cell ID
    } // if num vertices inside is > 1
  }   // loop over larger grid spanning module hexagon
  
  if (verbose_ > 0) std::cout << std::endl;
  return;
}

void EcalGeometry::buildCellModuleMap() {
  if (verbose_ > 0)
    std::cout << std::endl
              << "[EcalGeometry::buildCellModuleMap] Building cellModule position map"
              << std::endl;
  for (auto const& [layer_id, layer_xyz] : layer_pos_xy_) {
    for (auto const& [module_id, module_pq] : module_pos_pq_) {
      for (auto const& [cell_id, cell_pq] : cell_pos_in_module_) {
        // calculate cell's pq relative to entire layer center
        double cell_rel_to_layer = std::make_pair(
            module_pq.first + cell_pq.first,
            module_pq.second + cell_pq.second
            );
        // now convert from (p,q) to (x,y) space
        if (cornersSideUp_) {
          /**
           * 90deg rotation
           *  x = q
           *  y = -p
           * Basically a swap action to be able to reinterpret
           * first,second as x,y instead of p,q
           */
          double x;
          x = cell_rel_to_layer.second;
          cell_rel_to_layer.second = -1.*cell_rel_to_layer.first;
          cell_rel_to_layer.first = x;
        }
        // when the corners are not up, x = p and y = q
        // so no transformation needs to be done

        // now add the layer-center values to get the global position
        // of the cell
        cell_global_pos_[EcalID(layer_id, module_id, cell_id)]
          = std::make_tuple(
              cell_rel_to_layer.first + std::get<0>(layer_xyz),
              cell_rel_to_layer.second + std::get<1>(layer_xyz),
              std::get<2>(layer_xyz));
      }
    }
  }
  if (verbose_ > 0)
    std::cout << "  contained " << cell_global_pos_.size() << " entries. "
              << std::endl;
}

void EcalGeometry::buildNeighborMaps() {
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

double EcalGeometry::distanceToEdge(EcalID cellModuleID) const {
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

bool EcalGeometry::isInside(double normX, double normY) const {
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
