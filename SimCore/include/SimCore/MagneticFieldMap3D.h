/**
 * @file MagneticFieldMap3D.h
 * @brief Class for defining a global 3D magnetic field
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Norman Graf, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_MAGNETICFIELDMAP3D_H_
#define SIMCORE_MAGNETICFIELDMAP3D_H_

// Geant4
#include "Framework/Logger.h"
#include "G4MagneticField.hh"

// STL
#include <vector>
using std::vector;

namespace simcore {

/**
 * @brief
 * A 3D B-field map defined as a grid of points with associated B-field values.
 *
 * @note
 * Values are interpolated to obtain B-field information at points in between.
 *
 * @details
 * The header format of the input data file is as follows:
 *
 * 1) Blank line
 *
 * 2) int int int
 *
 * Number of X, Y, and Z grid points (N_x, N_y, N_z)
 *
 * 3-8) int String
 *
 * A description of fields in the lines of the map.  In our case it is
 * literally:
 *
 * 1 X
 * 2 Y
 * 3 Z
 * 4 BX
 * 5 BY
 * 6 BZ
 *
 * 9) int String
 *
 * Header terminator
 *
 * 10+) float float float float float float
 *
 * N_x*N_y*N_z+9
 *
 * x_ y_ z_ B_x B_y B_z
 *
 * Original PurgMagTabulatedField3D code developed by: S.Larsson and J.
 * Generowicz.
 */

class MagneticFieldMap3D : public G4MagneticField {
 public:
  /**
   * Class constructor.
   * @param[in] filename The name of the file defining the B-field grid.
   * @param[in] xOffset, yOffset, zOffset The offset of the grid's coordinate
   * system.
   */
  MagneticFieldMap3D(const char* filename, double xOffset, double yOffset,
                     double zOffset);

  /**
   * Implementation of primary virtual method from G4MagneticField interface.
   * @param[in]  point  The point in 3D space.
   * @param[out] bfield The output B-field data at the point.
   */
  void GetFieldValue(const double point[4], double* bfield) const;

 private:
  /*
   * Storage space for the table.
   */
  vector<vector<vector<double>>> x_field_;
  vector<vector<vector<double>>> y_field_;
  vector<vector<vector<double>>> z_field_;

  /*
   * The dimensions of the table.
   */
  int nx_, ny_, nz_;

  /*
   * The physical limits of the defined region.
   */
  double minx_, maxx_, miny_, maxy_, minz_, maxz_;

  /*
   * The physical extent of the defined region.
   */
  double dx_, dy_, dz_;

  /*
   * Offsets if field map is not in global coordinates
   */
  double x_offset_;
  double y_offset_;
  double z_offset_;

  /*
   * Flags for inverting dimensions.
   */
  bool invert_x_, invert_y_, invert_z_;

  /**
   * Enable logging for this class.
   */
  enableLogging("MagneticFieldMap3D")
};

}  // namespace simcore
// namespace

#endif
