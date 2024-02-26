/**
 * @file BeamElectronTruth.h
 * @brief Class that represents the truth information about
 * beam electron at the target. Can be used for incoming/recoil
 * electron.
 * @author Lene Kristian Bryngemark, Lund University
 */

#ifndef RECON_EVENT_BEAMELECTRONTRUTH_H_
#define RECON_EVENT_BEAMELECTRONTRUTH_H_

// ROOT
#include "TObject.h"  //ClassDef

// STL
#include <iostream>

namespace ldmx {

/**
 * @class BeamElectronTruth
 * @brief Represents the truth information on beam electrons at the target
 */
class BeamElectronTruth {
 public:
  /**
   * Class constructor.
   */
  BeamElectronTruth() = default;

  /**
   * Class destructor.
   */
  virtual ~BeamElectronTruth(){};

  /**
   * Print a description of this object.
   */
  void Print() const;

  /**
   * Clear the data in the object.
   */
  void Clear();

  /**
   * Set x coordinate of the found beam electron.
   * @param x The x coordinate of the found beam electron.
   */
  void setX(double x) { x_ = x; }

  /**
   * Set y coordinate of the found beam electron.
   * @param y The y coordinate of the found beam electron.
   */
  void setY(double y) { y_ = y; }

  /**
   * Set z coordinate of the found beam electron.
   * @param z The z coordinate of the found beam electron.
   */
  void setZ(double z) { z_ = z; }

  /**
   * Set all three spatial coordinates  at once
   * @param x The x coordinate of the found beam electron.
    * @param y The y coordinate of the found beam electron.
   * @param z The z coordinate of the found beam electron.

   */
  void setXYZ(double x, double y, double z) {
    x_ = x;
    y_ = y;
    z_ = z;
  }

  /**
   * Get x coordinate of the beam electron.
   * @return x The x coordinate of the found beam electron.
   */
  double getX() { return x_; }

  /**
   * Get y coordinate of the beam electron.
   * @return y The y coordinate of the found beam electron.
   */
  double getY() { return y_; }

  /**
   * Get z coordinate of the beam electron.
   * @return z The z coordinate of the found beam electron.
   */
  double getZ() { return z_; }

  // binned coordinates, according to some detector granularity. here, assume TS

  /**
   * SetBinned x coordinate of the found beam electron.
   * @param x The x coordinate of the found beam electron.
   */
  void setBinnedX(double x) { binnedX_ = x; }

  /**
   * SetBinned y coordinate of the found beam electron.
   * @param y The y coordinate of the found beam electron.
   */
  void setBinnedY(double y) { binnedY_ = y; }

  /**
   * Set all three binned spatial coordinates at once
   * @param x The binned x coordinate of the found beam electron.
   * @param y The binned y coordinate of the found beam electron.
   */
  void setBinnedXY(double x, double y) {
    binnedX_ = x;
    binnedY_ = y;
  }

  /**
   * Get binned x coordinate of the beam electron.
   * @return x The x coordinate of the found beam electron.
   */
  double getBinnedX() { return binnedX_; }

  /**
   * GetBinned y coordinate of the beam electron.
   * @return y The y coordinate of the found beam electron.
   */
  double getBinnedY() { return binnedY_; }

  // repeat for bar numbers instead of a spatial (bar center) coordinate

  /**
   * Set x bar number of the found beam electron.
   * @param x The x (vertical) bar number of the found beam electron.
   */
  void setBarX(double x) { barX_ = x; }

  /**
   * Set y bar number of the found beam electron.
   * @param y The y (horizontal) bar number of the found beam electron.
   */
  void setBarY(double y) { barY_ = y; }

  /**
   * Set both bar number coordinates at once
   * @param x The x (vertical) bar number of the found beam electron.
   * @param y The y (horizontal) bar number of the found beam electron.
   */
  void setBarXY(double x, double y) {
    barX_ = x;
    barY_ = y;
  }

  /**
   * Get x bar number of the found beam electron.
   * @return x The x (vertical) bar number of the found beam electron.
   */
  double getBarX() { return barX_; }

  /**
   * Get y bar number of the found beam electron.
   * @return y The y (horizontal) bar number of the found beam electron.
   */
  double getBarY() { return barY_; }

  // TODO could add separate setters for each momentum component

  /**
   * Set the entire three-momentum at once
   * @param px The x component of the three-momentum
   * @param py The y component of the three-momentum
   * @param pz The z component of the three-momentum
   */
  void setThreeMomentum(double px, double py, double pz);

  /**
   * Get px component of the beam electron momentum.
   * @return px The px component of the beam electron momentum.
   */
  double getPx() { return px_; }

  /**
   * Get py component of the beam electron momentum.
   * @return py The py component of the beam electron momentum.
   */
  double getPy() { return py_; }

  /**
   * Get pz component of the beam electron momentum.
   * @return pz The pz component of the beam electron momentum.
   */
  double getPz() { return pz_; }

  /**
   * some sorting operator is mandatory
   * sort on hit Z coordinate
   */
  bool operator<(BeamElectronTruth &rhs) { return this->getZ() < rhs.getZ(); }

 private:
  /* Algorithm variable results from simhit associations. */

  /** x coordinate ("truth" resolution, but within merging tolerance) **/
  double x_{-999};

  /** y coordinate ("truth" resolution, but within merging tolerance) **/
  double y_{-999};

  /** z coordinate ("truth" resolution, in practice, set to taget z = 0 **/
  double z_{-9999};

  /** x coordinate (with TS resolution) **/
  double binnedX_{-999};

  /** y coordinate (with TS resolution) **/
  double binnedY_{-999};

  /** TS vertical bar number overlapping with x coordinate **/
  double barX_{-1};

  /** TS horizontal bar number overlapping with y coordinate **/
  double barY_{-1};

  /** x momentum component **/
  double px_{-999};
  /** y momentum component **/
  double py_{-999};
  /** z momentum component **/
  double pz_{-999};

  ClassDef(BeamElectronTruth, 1);
};  // class BeamElectronTruth
}  // namespace ldmx

#endif
