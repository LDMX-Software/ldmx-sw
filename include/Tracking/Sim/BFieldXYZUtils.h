#include "Acts/MagneticField/InterpolatedBFieldMap.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Utilities/detail/AxisFwd.hpp"
#include "Acts/MagneticField/BFieldMapUtils.hpp"
#include "Acts/Utilities/Interpolation.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/Utilities/detail/Grid.hpp"


using InterpolatedMagneticField3 =
  Acts::InterpolatedBFieldMap<Acts::detail::Grid<
  Acts::Vector3, Acts::detail::EquidistantAxis,
  Acts::detail::EquidistantAxis, Acts::detail::EquidistantAxis>>;

// map (x,y,z) -> (r,z)
//auto transformPos = [](const Vector3& pos) {
//  return Vector2(perp(pos), pos.z());
//};

// map (Bx,By,Bz) -> (Bx,By,Bz)
//auto transformBField = [](const Vector3& field, const Vector3&) {
//  return field;
//};

//auto localToGlobalBin_xyz = (std::array<size_t, 3> binsXYZ,
//			     std::array<size_t, 3> nBinsXYZ) {
//  return (binsXYZ.at(0) * (nBinsXYZ.at(1) * nBinsXYZ.at(2)) +
//	  binsXYZ.at(1) * nBinsXYZ.at(2) + binsXYZ.at(2));
//};

Acts::InterpolatedBFieldMap<Acts::detail::Grid<
Acts::Vector3, Acts::detail::EquidistantAxis, Acts::detail::EquidistantAxis,
  Acts::detail::EquidistantAxis>>
    rotateFieldMapXYZ(const std::function<size_t(std::array<size_t, 3> binsXYZ,
                                                 std::array<size_t, 3> nBinsXYZ)>&
                      localToGlobalBin,
                  std::vector<double> xPos, std::vector<double> yPos,
                  std::vector<double> zPos, std::vector<Acts::Vector3> bField,
                  double lengthUnit, double BFieldUnit, bool firstOctant) {
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
  size_t nBinsX = xPos.size();
  size_t nBinsY = yPos.size();
  size_t nBinsZ = zPos.size();

  // get the minimum and maximum
  auto minMaxX = std::minmax_element(xPos.begin(), xPos.end());
  auto minMaxY = std::minmax_element(yPos.begin(), yPos.end());
  auto minMaxZ = std::minmax_element(zPos.begin(), zPos.end());
  // Create the axis for the grid
  // get minima
  double xMin = *minMaxX.first;
  double yMin = *minMaxY.first;
  double zMin = *minMaxZ.first;
  // get maxima
  double xMax = *minMaxX.second;
  double yMax = *minMaxY.second;
  double zMax = *minMaxZ.second;
  // calculate maxima (add one last bin, because bin value always corresponds to
  // left boundary)
  double stepZ = std::fabs(zMax - zMin) / (nBinsZ - 1);
  double stepY = std::fabs(yMax - yMin) / (nBinsY - 1);
  double stepX = std::fabs(xMax - xMin) / (nBinsX - 1);
  xMax += stepX;
  yMax += stepY;
  zMax += stepZ;

  // If only the first octant is given
  if (firstOctant) {
    xMin = -*minMaxX.second;
    yMin = -*minMaxY.second;
    zMin = -*minMaxZ.second;
    nBinsX = 2 * nBinsX - 1;
    nBinsY = 2 * nBinsY - 1;
    nBinsZ = 2 * nBinsZ - 1;
  }
  Acts::detail::EquidistantAxis xAxis(xMin * lengthUnit, xMax * lengthUnit,
                                      nBinsX);
  Acts::detail::EquidistantAxis yAxis(yMin * lengthUnit, yMax * lengthUnit,
                                      nBinsY);
  Acts::detail::EquidistantAxis zAxis(zMin * lengthUnit, zMax * lengthUnit,
                                      nBinsZ);
  // Create the grid
  using Grid_t =
      Acts::detail::Grid<Acts::Vector3, Acts::detail::EquidistantAxis,
                         Acts::detail::EquidistantAxis,
                         Acts::detail::EquidistantAxis>;
  Grid_t grid(
      std::make_tuple(std::move(xAxis), std::move(yAxis), std::move(zAxis)));

  // [2] Set the bField values
  for (size_t i = 1; i <= nBinsX; ++i) {
    for (size_t j = 1; j <= nBinsY; ++j) {
      for (size_t k = 1; k <= nBinsZ; ++k) {
        Grid_t::index_t indices = {{i, j, k}};
        std::array<size_t, 3> nIndices = {
            {xPos.size(), yPos.size(), zPos.size()}};
        if (firstOctant) {
          // std::vectors begin with 0 and we do not want the user needing to
          // take underflow or overflow bins in account this is why we need to
          // subtract by one
          size_t m = std::abs(int(i) - (int(xPos.size())));
          size_t n = std::abs(int(j) - (int(yPos.size())));
          size_t l = std::abs(int(k) - (int(zPos.size())));
          Grid_t::index_t indicesFirstOctant = {{m, n, l}};

          grid.atLocalBins(indices) =
              bField.at(localToGlobalBin(indicesFirstOctant, nIndices)) *
              BFieldUnit;

        } else {
          // std::vectors begin with 0 and we do not want the user needing to
          // take underflow or overflow bins in account this is why we need to
          // subtract by one
          grid.atLocalBins(indices) =
              bField.at(localToGlobalBin({{i - 1, j - 1, k - 1}}, nIndices)) *
              BFieldUnit;
        }
      }
    }
  }
  grid.setExteriorBins(Acts::Vector3::Zero());

  // [3] Create the transformation for the position
  // map (z,x,y) -> (x,y,z)
  // TODO:: Remove this hardcoded value!
  auto transformPos = [](const Acts::Vector3& pos, float offset=400.) {
    
    Acts::Vector3 rot_pos;
    rot_pos(0)=pos(1);
    rot_pos(1)=pos(2);
    rot_pos(2)=pos(0) + offset;
    
    //std::cout<<"PF::Check:: transforming Pos"<<std::endl;
    //std::cout<<pos<<std::endl;
    //std::cout<<"TO"<<std::endl;
    //std::cout<<rot_pos<<std::endl;
    
    return rot_pos;
  };

  // [4] Create the transformation for the bfield
  // map (Bx,By,Bz) -> (Bx,By,Bz)
  auto transformBField = [](const Acts::Vector3& field,
                            const Acts::Vector3& /*pos*/) {

    
    Acts::Vector3 rot_field;
    
    rot_field(0) = field(2);
    rot_field(1) = field(0);
    rot_field(2) = field(1);

    //std::cout<<"PF::Check:: transforming"<<std::endl;
    //std::cout<<field<<std::endl;
    //std::cout<<"TO"<<std::endl;
    //std::cout<<rot_field<<std::endl;
    
    return rot_field;
  };

  // [5] Create the mapper & BField Service
  // create field mapping
  return Acts::InterpolatedBFieldMap<Grid_t>(
      {transformPos, transformBField, std::move(grid)});
}




//This is a copy of 
//https://github.com/acts-project/acts/blob/main/Examples/Detectors/MagneticField/src/FieldMapTextIo.cpp
//with additional rotateAxes flag to rotate the axes and field to be in the tracking (ACTS) Frame


inline InterpolatedMagneticField3 makeMagneticFieldMapXyzFromText(std::function<size_t(std::array<size_t, 3> binsXYZ,
										       std::array<size_t, 3> nBinsXYZ)> localToGlobalBin,
								  const std::string& fieldMapFile, Acts::ActsScalar lengthUnit,
								  Acts::ActsScalar BFieldUnit, bool firstOctant, bool rotateAxes) {
  /// [1] Read in field map file
  // Grid position points in x, y and z
  std::vector<double> xPos;
  std::vector<double> yPos;
  std::vector<double> zPos;
  // components of magnetic field on grid points
  std::vector<Acts::Vector3> bField;
  
  constexpr size_t kDefaultSize = 1 << 15;
  // reserve estimated size
  xPos.reserve(kDefaultSize);
  yPos.reserve(kDefaultSize);
  zPos.reserve(kDefaultSize);
  bField.reserve(kDefaultSize);
  // [1] Read in file and fill values
  std::ifstream map_file(fieldMapFile.c_str(), std::ios::in);
  std::string line;
  double x = 0., y = 0., z = 0.;
  double bx = 0., by = 0., bz = 0.;
  while (std::getline(map_file, line)) {
    if (line.empty() || line[0] == '%' || line[0] == '#' || line[0] == ' ' ||
        line.find_first_not_of(' ') == std::string::npos)
      continue;
    
    std::istringstream tmp(line);
    tmp >> x >> y >> z >> bx >> by >> bz;
    
    xPos.push_back(x);
    yPos.push_back(y);
    zPos.push_back(z);
    bField.push_back(Acts::Vector3(bx, by, bz));
  }
  map_file.close();
  xPos.shrink_to_fit();
  yPos.shrink_to_fit();
  zPos.shrink_to_fit();
  bField.shrink_to_fit();

  if (rotateAxes) {
    return rotateFieldMapXYZ(localToGlobalBin, xPos, yPos, zPos, bField,
                             lengthUnit, BFieldUnit, firstOctant);
  }
  else
    return Acts::fieldMapXYZ(localToGlobalBin, xPos, yPos, zPos, bField,
                             lengthUnit, BFieldUnit, firstOctant);
  
}

