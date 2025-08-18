#pragma once

#include <fstream>
#include <functional>
#include <iostream>

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/MagneticField/BFieldMapUtils.hpp"
#include "Acts/MagneticField/InterpolatedBFieldMap.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Utilities/AxisFwd.hpp"
#include "Acts/Utilities/Grid.hpp"
#include "Acts/Utilities/Interpolation.hpp"
#include "Acts/Utilities/Result.hpp"

static const double DIPOLE_OFFSET = 400.;  // 400 mm

using InterpolatedMagneticField3 = Acts::InterpolatedBFieldMap<
    Acts::Grid<Acts::Vector3, Acts::Axis<Acts::AxisType::Equidistant>,
               Acts::Axis<Acts::AxisType::Equidistant>,
               Acts::Axis<Acts::AxisType::Equidistant>>>;

using GenericTransformPos = std::function<Acts::Vector3(const Acts::Vector3&)>;
using GenericTransformBField =
    std::function<Acts::Vector3(const Acts::Vector3&, const Acts::Vector3&)>;

/**
 * The default mapping transformation from the tracking space to the bfield
 * space The offset at 400. is the default to place the magnetic field map in
 * the correct location Create the transformation for the position map
 * (z,x,y)
 * -> (x,y,z)
 */

Acts::Vector3 defaultTransformPos(const Acts::Vector3& pos_);

/**
 * The dafault mapping transformation from the field space to the tracking space
 * for the field value Create the transformation for the bfield map (Bx,By,Bz)
 * -> (Bx,By,Bz)
 */

Acts::Vector3 defaultTransformBField(const Acts::Vector3& field,
                                      const Acts::Vector3& /*pos_*/);

void testField(const std::shared_ptr<Acts::MagneticFieldProvider> bfield,
               const Acts::Vector3& eval_pos,
               const Acts::MagneticFieldContext& bctx);

/**
 * The default mapping between local to global bins of the map
 * it transforms the local xyz binning to the global binning
 */

size_t localToGlobalBinXyz(std::array<size_t, 3> bins,
                            std::array<size_t, 3> sizes);

inline InterpolatedMagneticField3 rotateFieldMapXYZ(
    const std::function<size_t(std::array<size_t, 3> binsXYZ,
                               std::array<size_t, 3> nBinsXYZ)>&
        localToGlobalBin,
    std::vector<double> xPos, std::vector<double> yPos,
    std::vector<double> zPos, std::vector<Acts::Vector3> bField,
    double lengthUnit, double BFieldUnit, bool firstOctant,
    GenericTransformPos transformPosition,
    GenericTransformBField transformMagneticField) {
  // [1] Create Grid
  // Sort the values
  std::sort(xPos.begin(), xPos.end());
  std::sort(yPos.begin(), yPos.end());
  std::sort(zPos.begin(), zPos.end());

  // Get unique values
  xPos.erase(std::unique(xPos.begin(), xPos.end()), xPos.end());
  yPos.erase(std::unique(yPos.begin(), yPos.end()), yPos.end());
  zPos.erase(std::unique(zPos.begin(), zPos.end()), zPos.end());
  xPos.shrink_to_fit();
  yPos.shrink_to_fit();
  zPos.shrink_to_fit();

  // get the number of bins
  size_t n_bins_x = xPos.size();
  size_t n_bins_y = yPos.size();
  size_t n_bins_z = zPos.size();

  // get the minimum and maximum
  auto min_max_x = std::minmax_element(xPos.begin(), xPos.end());
  auto min_max_y = std::minmax_element(yPos.begin(), yPos.end());
  auto min_max_z = std::minmax_element(zPos.begin(), zPos.end());
  // Create the axis for the grid
  // get minima
  double x_min = *min_max_x.first;
  double y_min = *min_max_y.first;
  double z_min = *min_max_z.first;
  // get maxima
  double x_max = *min_max_x.second;
  double y_max = *min_max_y.second;
  double z_max = *min_max_z.second;
  // calculate maxima (add one last bin, because bin value always corresponds to
  // left boundary)
  double step_z = std::fabs(z_max - z_min) / (n_bins_z - 1);
  double step_y = std::fabs(y_max - y_min) / (n_bins_y - 1);
  double step_x = std::fabs(x_max - x_min) / (n_bins_x - 1);
  x_max += step_x;
  y_max += step_y;
  z_max += step_z;

  // If only the first octant is given
  if (firstOctant) {
    x_min = -*min_max_x.second;
    y_min = -*min_max_y.second;
    z_min = -*min_max_z.second;
    n_bins_x = 2 * n_bins_x - 1;
    n_bins_y = 2 * n_bins_y - 1;
    n_bins_z = 2 * n_bins_z - 1;
  }
  Acts::Axis<Acts::AxisType::Equidistant> x_axis(x_min * lengthUnit,
                                                x_max * lengthUnit, n_bins_x);
  Acts::Axis<Acts::AxisType::Equidistant> y_axis(y_min * lengthUnit,
                                                y_max * lengthUnit, n_bins_y);
  Acts::Axis<Acts::AxisType::Equidistant> z_axis(z_min * lengthUnit,
                                                z_max * lengthUnit, n_bins_z);
  // Create the grid
  using Grid_t =
      Acts::Grid<Acts::Vector3, Acts::Axis<Acts::AxisType::Equidistant>,
                 Acts::Axis<Acts::AxisType::Equidistant>,
                 Acts::Axis<Acts::AxisType::Equidistant>>;
  Grid_t grid(
      std::make_tuple(std::move(x_axis), std::move(y_axis), std::move(z_axis)));

  // [2] Set the bField values
  for (size_t i = 1; i <= n_bins_x; ++i) {
    for (size_t j = 1; j <= n_bins_y; ++j) {
      for (size_t k = 1; k <= n_bins_z; ++k) {
        Grid_t::index_t indices = {{i, j, k}};
        std::array<size_t, 3> n_indices = {
            {xPos.size(), yPos.size(), zPos.size()}};
        if (firstOctant) {
          // std::vectors begin with 0 and we do not want the user needing to
          // take underflow or overflow bins in account this is why we need to
          // subtract by one
          size_t m = std::abs(int(i) - (int(xPos.size())));
          size_t n = std::abs(int(j) - (int(yPos.size())));
          size_t l = std::abs(int(k) - (int(zPos.size())));
          Grid_t::index_t indices_first_octant = {{m, n, l}};

          grid.atLocalBins(indices) =
              bField.at(localToGlobalBin(indices_first_octant, n_indices)) *
              BFieldUnit;

        } else {
          // std::vectors begin with 0 and we do not want the user needing to
          // take underflow or overflow bins in account this is why we need to
          // subtract by one
          grid.atLocalBins(indices) =
              bField.at(localToGlobalBin({{i - 1, j - 1, k - 1}}, n_indices)) *
              BFieldUnit;
        }
      }
    }
  }
  grid.setExteriorBins(Acts::Vector3::Zero());

  // [3] Create the transformation for the position
  // map (z,x,y) -> (x,y,z)

  /*
  auto transformPos = [](const Acts::Vector3& pos_, float offset=400.) {

    Acts::Vector3 rot_pos;
    rot_pos(0)=pos_(1);
    rot_pos(1)=pos_(2);
    rot_pos(2)=pos_(0) + offset;

    return rot_pos;
  };

  */

  // [4] Create the transformation for the bfield
  // map (Bx,By,Bz) -> (Bx,By,Bz)

  // auto transformBField = [](const Acts::Vector3& field,
  //                           const Acts::Vector3& /*pos_*/) {
  //
  //
  //   Acts::Vector3 rot_field;
  //
  //   rot_field(0) = field(2);
  //   rot_field(1) = field(0);
  //   rot_field(2) = field(1);

  //  return rot_field;
  //};

  // [5] Create the mapper and BField Service
  // with the transformations passed from main producer
  return Acts::InterpolatedBFieldMap<Grid_t>(
      {transformPosition, transformMagneticField, std::move(grid)});
}

// This is a copy of
// https://github.com/acts-project/acts/blob/main/Examples/Detectors/MagneticField/src/FieldMapTextIo.cpp
// with additional rotateAxes flag to rotate the axes and field to be in the
// tracking (ACTS) Frame

inline InterpolatedMagneticField3 makeMagneticFieldMapXyzFromText(
    std::function<size_t(std::array<size_t, 3> binsXYZ,
                         std::array<size_t, 3> nBinsXYZ)>
        localToGlobalBin,
    GenericTransformPos transformPosition,
    GenericTransformBField transformMagneticField,
    const std::string& fieldMapFile, Acts::ActsScalar lengthUnit,
    Acts::ActsScalar BFieldUnit, bool firstOctant, bool rotateAxes) {
  /// [1] Read in field map file
  // Grid position points in x, y and z
  std::vector<double> x_pos;
  std::vector<double> y_pos;
  std::vector<double> z_pos;
  // components of magnetic field on grid points
  std::vector<Acts::Vector3> b_field;

  constexpr size_t k_default_size = 1 << 15;
  // reserve estimated size
  x_pos.reserve(k_default_size);
  y_pos.reserve(k_default_size);
  z_pos.reserve(k_default_size);
  b_field.reserve(k_default_size);
  // [1] Read in file and fill values
  std::ifstream map_file(fieldMapFile.c_str(), std::ios::in);
  std::string line;
  double pos_x = 0., pos_y = 0., pos_z = 0.;
  double bx = 0., by = 0., bz = 0.;

  bool header_found = false;

  while (std::getline(map_file, line)) {
    if (line.empty() || line[0] == '%' || line[0] == '#' || line[0] == ' ' ||
        line.find_first_not_of(' ') == std::string::npos || !header_found) {
      if (line.find("Header") != std::string::npos) header_found = true;
      continue;
    }
    std::istringstream tmp(line);
    tmp >> pos_x >> pos_y >> pos_z >> bx >> by >> bz;

    x_pos.push_back(pos_x);
    y_pos.push_back(pos_y);
    z_pos.push_back(pos_z);
    b_field.push_back(Acts::Vector3(bx, by, bz));
  }
  map_file.close();

  if (!header_found) {
    std::cout << "MAP LOADING ERROR:: line containing the word 'Header' not "
                 "found in the BMap."
              << std::endl;
  }

  x_pos.shrink_to_fit();
  y_pos.shrink_to_fit();
  z_pos.shrink_to_fit();
  b_field.shrink_to_fit();

  if (rotateAxes) {
    return rotateFieldMapXYZ(localToGlobalBin, x_pos, y_pos, z_pos, b_field,
                             lengthUnit, BFieldUnit, firstOctant,
                             transformPosition, transformMagneticField);
  } else
    return Acts::fieldMapXYZ(localToGlobalBin, x_pos, y_pos, z_pos, b_field,
                             lengthUnit, BFieldUnit, firstOctant);
}

inline InterpolatedMagneticField3 loadDefaultBField(
    const std::string& fieldMapFile, GenericTransformPos transformPosition,
    GenericTransformBField transformMagneticField) {
  // std::function<Acts::Vector3(const Acts::Vector3&, float)>
  // transformPosition, std::function<Acts::Vector3(const Acts::Vector3&,const
  // Acts::Vector3&)> transformMagneticField

  return makeMagneticFieldMapXyzFromText(
      std::move(localToGlobalBinXyz), transformPosition,
      transformMagneticField, fieldMapFile,
      1. * Acts::UnitConstants::mm,    // default scale for axes length
      1000. * Acts::UnitConstants::T,  // The map is in kT, so scale it to T
      false,                           // not symmetrical
      true                             // rotate the axes to tracking frame
  );
}

// R =
//
// 0   0   1
// 1   0   0
// 0   1   0
