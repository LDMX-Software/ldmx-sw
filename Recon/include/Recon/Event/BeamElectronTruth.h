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
  virtual ~BeamElectronTruth() {};

  /**
   * Print a description of this object.
   */
  friend std::ostream& operator<<(std::ostream& o, const BeamElectronTruth& d);

  /**
   * Clear the data in the object.
   */
  void clear();

  /**
   * Set x coordinate of the found beam electron.
   * @param x The x coordinate of the found beam electron.
   */
  void setX(double x) { x_ = x; }

  /**
   * Set y_ coordinate of the found beam electron.
   * @param y_ The y_ coordinate of the found beam electron.
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
   * Get y_ coordinate of the beam electron.
   * @return y_ The y_ coordinate of the found beam electron.
   */
  double getY() { return y_; }

  /**
   * Get z_ coordinate of the beam electron.
   * @return z_ The z_ coordinate of the found beam electron.
   */
  double getZ() { return z_; }

  // binned coordinates, according to some detector granularity. here, assume TS

  /**
   * SetBinned x coordinate of the found beam electron.
   * @param x The x coordinate of the found beam electron.
   */
  void setBinnedX(double x) { binned_x_ = x; }

  /**
   * SetBinned y coordinate of the found beam electron.
   * @param y The y coordinate of the found beam electron.
   */
  void setBinnedY(double y) { binned_y_ = y; }

  /**
   * Set all three binned spatial coordinates at once
   * @param x The binned x coordinate of the found beam electron.
   * @param y The binned y coordinate of the found beam electron.
   */
  void setBinnedXY(double x, double y) {
    binned_x_ = x;
    binned_y_ = y;
  }

  /**
   * Get binned x coordinate of the beam electron.
   * @return x The x coordinate of the found beam electron.
   */
  double getBinnedX() { return binned_x_; }

  /**
   * GetBinned y_ coordinate of the beam electron.
   * @return y_ The y_ coordinate of the found beam electron.
   */
  double getBinnedY() { return binned_y_; }

  // repeat for bar numbers instead of a spatial (bar center) coordinate

  /**
   * Set x bar number of the found beam electron.
   * @param x The x (vertical) bar number of the found beam electron.
   */
  void setBarX(double x) { bar_x_ = x; }

  /**
   * Set y_ bar number of the found beam electron.
   * @param y_ The y_ (horizontal) bar number of the found beam electron.
   */
  void setBarY(double y) { bar_y_ = y; }

  /**
   * Set both bar number coordinates at once
   * @param x The x (vertical) bar number of the found beam electron.
   * @param y The y (horizontal) bar number of the found beam electron.
   */
  void setBarXY(double x, double y) {
    bar_x_ = x;
    bar_y_ = y;
  }

  /**
   * Get x bar number of the found beam electron.
   * @return x The x (vertical) bar number of the found beam electron.
   */
  double getBarX() { return bar_x_; }

  /**
   * Get y_ bar number of the found beam electron.
   * @return y_ The y_ (horizontal) bar number of the found beam electron.
   */
  double getBarY() { return bar_y_; }

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
  bool operator<(BeamElectronTruth& rhs) { return this->getZ() < rhs.getZ(); }

 private:
  /* Algorithm variable results from simhit associations. */

  /** x-coordinate ("truth" resolution, but within merging tolerance) **/
  double x_{-999};

  /** y-coordinate ("truth" resolution, but within merging tolerance) **/
  double y_{-999};

  /** z-coordinate ("truth" resolution, in practice, set to taget z-coord = 0
   * **/
  double z_{-9999};

  /** x-coordinate (with TS resolution) **/
  double binned_x_{-999};

  /** y-coordinate (with TS resolution) **/
  double binned_y_{-999};

  /** TS vertical bar number overlapping with x-coordinate **/
  double bar_x_{-1};

  /** TS horizontal bar number overlapping with y-coordinate **/
  double bar_y_{-1};

  /** momentum x-component **/
  double px_{-999};
  /** momentum y-component **/
  double py_{-999};
  /** momentum z-component **/
  double pz_{-999};

  ClassDef(BeamElectronTruth, 2);
};  // class BeamElectronTruth
}  // namespace ldmx

#endif
