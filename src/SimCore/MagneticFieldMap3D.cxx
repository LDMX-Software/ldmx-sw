#include "SimCore/MagneticFieldMap3D.h"
#include "Framework/Exception/Exception.h"

// STL
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

// Geant4
#include "G4SystemOfUnits.hh"
#include "globals.hh"

using namespace std;

namespace simcore {

MagneticFieldMap3D::MagneticFieldMap3D(const char* filename, double xOffset,
                                       double yOffset, double zOffset)
    : nx_(0),
      ny_(0),
      nz_(0),
      xOffset_(xOffset),
      yOffset_(yOffset),
      zOffset_(zOffset),
      invertX_(false),
      invertY_(false),
      invertZ_(false) {
  ifstream file(filename);  // Open the file for reading.

  // Throw an error if file does not exist.
  if (!file.good()) {
    EXCEPTION_RAISE("FileDNE", "The field map file '" + std::string(filename) +
                                   "' does not exist!");
  }

  G4cout << "-----------------------------------------------------------"
         << G4endl;
  G4cout << "    Magnetic Field Map 3D" << G4endl;
  G4cout << "-----------------------------------------------------------"
         << G4endl << G4endl;

  G4cout << "Reading the field grid from " << filename << " ... " << endl;
  G4cout << "  Offsets: " << xOffset << " " << yOffset << " " << zOffset
         << G4endl;

  // Ignore first blank line
  char buffer[256];
  file.getline(buffer, 256);

  // Read table dimensions
  file >> nx_ >> ny_ >> nz_;  // Note dodgy order

  G4cout << "  Number of values: " << nx_ << " " << ny_ << " " << nz_ << G4endl;

  // Set up storage space for table
  xField_.resize(nx_);
  yField_.resize(nx_);
  zField_.resize(nx_);
  int ix, iy, iz;
  for (ix = 0; ix < nx_; ix++) {
    xField_[ix].resize(ny_);
    yField_[ix].resize(ny_);
    zField_[ix].resize(ny_);
    for (iy = 0; iy < ny_; iy++) {
      xField_[ix][iy].resize(nz_);
      yField_[ix][iy].resize(nz_);
      zField_[ix][iy].resize(nz_);
    }
  }

  // Ignore other header information
  // The first line whose second character is '0' is considered to
  // be the last line of the header.
  do {
    file.getline(buffer, 256);
  } while (buffer[1] != '0');

  // Read in the data
  double xval, yval, zval, bx, by, bz;
  for (ix = 0; ix < nx_; ix++) {
    for (iy = 0; iy < ny_; iy++) {
      for (iz = 0; iz < nz_; iz++) {
        file >> xval >> yval >> zval >> bx >> by >> bz;
        if (ix == 0 && iy == 0 && iz == 0) {
          minx_ = xval;
          miny_ = yval;
          minz_ = zval;
        }
        xField_[ix][iy][iz] = bx;
        yField_[ix][iy][iz] = by;
        zField_[ix][iy][iz] = bz;
      }
    }
  }
  file.close();

  maxx_ = xval;
  maxy_ = yval;
  maxz_ = zval;

  G4cout << "  ... done reading " << G4endl << G4endl;
  G4cout << "Read values of field from file " << filename << G4endl;
  G4cout << "  Assumed the order: x, y, z, Bx, By, Bz" << G4endl;
  G4cout << "  Min values: " << minx_ << " " << miny_ << " " << minz_ << " mm "
         << G4endl;
  G4cout << "  Max values: " << maxx_ << " " << maxy_ << " " << maxz_ << " mm "
         << G4endl;
  G4cout << "  Field offsets: " << xOffset_ << " " << yOffset_ << " "
         << zOffset_ << " mm " << G4endl << G4endl;

  // Should really check that the limits are not the wrong way around.
  if (maxx_ < minx_) {
    swap(maxx_, minx_);
    invertX_ = true;
  }
  if (maxy_ < miny_) {
    swap(maxy_, miny_);
    invertY_ = true;
  }
  if (maxz_ < minz_) {
    swap(maxz_, minz_);
    invertZ_ = true;
  }

  G4cout << "After reordering if necessary" << G4endl;
  G4cout << "  Min values: " << minx_ << " " << miny_ << " " << minz_ << " mm "
         << G4endl;
  G4cout << "  Max values: " << maxx_ << " " << maxy_ << " " << maxz_ << " mm "
         << G4endl;
  ;

  dx_ = maxx_ - minx_;
  dy_ = maxy_ - miny_;
  dz_ = maxz_ - minz_;

  G4cout << "  Range of values: " << dx_ << " " << dy_ << " " << dz_ << " mm"
         << G4endl << G4endl;
  G4cout << "Done loading field map" << G4endl << G4endl;
  G4cout << "-----------------------------------------------------------"
         << G4endl << G4endl;
}

void MagneticFieldMap3D::GetFieldValue(const double point[4],
                                       double* bfield) const {
  double x = point[0] - xOffset_;
  double y = point[1] - yOffset_;
  double z = point[2] - zOffset_;
  double eps = 1E-6;

  // Check that the point is within the defined region
  if (x >= minx_ && x < maxx_ - eps && y >= miny_ && y < maxy_ - eps &&
      z >= minz_ && z < maxz_ - eps) {
    // Position of given point within region, normalized to the range
    // [0,1]
    double xfraction = (x - minx_) / dx_;
    double yfraction = (y - miny_) / dy_;
    double zfraction = (z - minz_) / dz_;

    if (invertX_) {
      xfraction = 1 - xfraction;
    }
    if (invertY_) {
      yfraction = 1 - yfraction;
    }
    if (invertZ_) {
      zfraction = 1 - zfraction;
    }

    // Need addresses of these to pass to modf below.
    // modf uses its second argument as an OUTPUT argument.
    double xdindex, ydindex, zdindex;

    // Position of the point within the cuboid defined by the
    // nearest surrounding tabulated points
    double xlocal = (std::modf(xfraction * (nx_ - 1), &xdindex));
    double ylocal = (std::modf(yfraction * (ny_ - 1), &ydindex));
    double zlocal = (std::modf(zfraction * (nz_ - 1), &zdindex));

    // The indices of the nearest tabulated point whose coordinates
    // are all less than those of the given point
    int xindex = static_cast<int>(xdindex);
    int yindex = static_cast<int>(ydindex);
    int zindex = static_cast<int>(zdindex);

#ifdef DEBUG_INTERPOLATING_FIELD
    G4cout << "Local x,y,z: " << xlocal << " " << ylocal << " " << zlocal
           << G4endl;
    G4cout << "Index x,y,z: " << xindex << " " << yindex << " " << zindex
           << G4endl;
    double valx0z0, mulx0z0, valx1z0, mulx1z0;
    double valx0z1, mulx0z1, valx1z1, mulx1z1;
    valx0z0 = table[xindex][0][zindex];
    mulx0z0 = (1 - xlocal) * (1 - zlocal);
    valx1z0 = table[xindex + 1][0][zindex];
    mulx1z0 = xlocal * (1 - zlocal);
    valx0z1 = table[xindex][0][zindex + 1];
    mulx0z1 = (1 - xlocal) * zlocal;
    valx1z1 = table[xindex + 1][0][zindex + 1];
    mulx1z1 = xlocal * zlocal;
#endif

    // Full 3-dimensional version
    bfield[0] =
        xField_[xindex][yindex][zindex] * (1 - xlocal) * (1 - ylocal) *
            (1 - zlocal) +
        xField_[xindex][yindex][zindex + 1] * (1 - xlocal) * (1 - ylocal) *
            zlocal +
        xField_[xindex][yindex + 1][zindex] * (1 - xlocal) * ylocal *
            (1 - zlocal) +
        xField_[xindex][yindex + 1][zindex + 1] * (1 - xlocal) * ylocal *
            zlocal +
        xField_[xindex + 1][yindex][zindex] * xlocal * (1 - ylocal) *
            (1 - zlocal) +
        xField_[xindex + 1][yindex][zindex + 1] * xlocal * (1 - ylocal) *
            zlocal +
        xField_[xindex + 1][yindex + 1][zindex] * xlocal * ylocal *
            (1 - zlocal) +
        xField_[xindex + 1][yindex + 1][zindex + 1] * xlocal * ylocal * zlocal;
    bfield[1] =
        yField_[xindex][yindex][zindex] * (1 - xlocal) * (1 - ylocal) *
            (1 - zlocal) +
        yField_[xindex][yindex][zindex + 1] * (1 - xlocal) * (1 - ylocal) *
            zlocal +
        yField_[xindex][yindex + 1][zindex] * (1 - xlocal) * ylocal *
            (1 - zlocal) +
        yField_[xindex][yindex + 1][zindex + 1] * (1 - xlocal) * ylocal *
            zlocal +
        yField_[xindex + 1][yindex][zindex] * xlocal * (1 - ylocal) *
            (1 - zlocal) +
        yField_[xindex + 1][yindex][zindex + 1] * xlocal * (1 - ylocal) *
            zlocal +
        yField_[xindex + 1][yindex + 1][zindex] * xlocal * ylocal *
            (1 - zlocal) +
        yField_[xindex + 1][yindex + 1][zindex + 1] * xlocal * ylocal * zlocal;
    bfield[2] =
        zField_[xindex][yindex][zindex] * (1 - xlocal) * (1 - ylocal) *
            (1 - zlocal) +
        zField_[xindex][yindex][zindex + 1] * (1 - xlocal) * (1 - ylocal) *
            zlocal +
        zField_[xindex][yindex + 1][zindex] * (1 - xlocal) * ylocal *
            (1 - zlocal) +
        zField_[xindex][yindex + 1][zindex + 1] * (1 - xlocal) * ylocal *
            zlocal +
        zField_[xindex + 1][yindex][zindex] * xlocal * (1 - ylocal) *
            (1 - zlocal) +
        zField_[xindex + 1][yindex][zindex + 1] * xlocal * (1 - ylocal) *
            zlocal +
        zField_[xindex + 1][yindex + 1][zindex] * xlocal * ylocal *
            (1 - zlocal) +
        zField_[xindex + 1][yindex + 1][zindex + 1] * xlocal * ylocal * zlocal;

  } else {
    bfield[0] = 0.0;
    bfield[1] = 0.0;
    bfield[2] = 0.0;
  }
}

}  // namespace simcore
