/**
 * @file HcalGeometry.h
 * @brief Class that translates HCal ID into positions of strip hits
 */

#ifndef DETDESCR_HCALGEOMETRY_H_
#define DETDESCR_HCALGEOMETRY_H_

// LDMX
#include "DetDescr/HcalID.h"
#include "Framework/ConditionsObject.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"

// ROOT
#include "TVector3.h"

// STL
#include <map>

namespace hcal {
class HcalGeometryProvider;
}

namespace ldmx {

class HcalGeometryProvider;

/**
 * @class HcalGeometry
 * @brief Implementation of HCal strip readout
 *
 */
class HcalGeometry : public framework::ConditionsObject {
public:
  /**
   * Conditions object:
   *  The name of the python configuration calling this class
   * (Hcal/python/HcalGeometry.py) needs to match the CONDITIONS_OBJECT_NAME
   * exactly.
   */
  static constexpr const char *CONDITIONS_OBJECT_NAME{"HcalGeometry"};

  /**
   * Encodes the orientation of a bar.
   *  horizontal : The length of the bar is along the x-axis
   *  vertical   : The length of the bar is along the y-axis
   *  depth      : The length of the bar is along the z-axis
   */
  enum class ScintillatorOrientation {
    horizontal = 0,
    vertical = 1,
    depth = 2
  };

  /**
   * Class destructor.
   *
   * Does nothing because the stl containers clean up automatically.
   */
  ~HcalGeometry() = default;

  ScintillatorOrientation
  getScintillatorOrientation(const ldmx::HcalID id) const;

  /**
   * Get a strip center position from a combined hcal ID.
   *
   * @throw std::out_of_range if HcalID is not on map.
   *
   * @param HcalID
   * @return A TVector3 with the X, Y and Z position of the center of the bar.
   */
  TVector3 getStripCenterPosition(ldmx::HcalID id) const {
    return strip_position_map_.at(id);
  }

  /**
   * Get the strip position map
   */
  std::map<ldmx::HcalID, TVector3> getStripPositionMap() const {
    return strip_position_map_;
  }

  /** Check whether a given layer corresponds to a horizontal (scintillator
   * length along the x-axis) or vertical layer in the back HCal. See the
   * back_horizontal_parity_ member for details.
   */
  bool backLayerIsHorizontal(const int layer) const {
    return layer % 2 == back_horizontal_parity_;
  }
  /**
   * Get the half total width of a layer for a given section(strip) for
   * back(side) Hcal.
   * @param section
   * @param layer
   * @return half total width [mm]
   */
  double getHalfTotalWidth(int isection, int layer = 1) const {
    // Layer numbering starts at 1, but a vector is zero-indexed
    auto layer_index = layer - 1;
    return half_total_width_.at(isection).at(layer_index);
  }

  /**
   * Get the length of a scintillator bar
   * @param id The HcalID of the bar
   * @return The length of the bar with ID `id` [mm]
   */
  double getScintillatorLength(ldmx::HcalID id) const {
    return scint_length_.at(id.section()).at(id.layer() - 1);
  }
  /**
   * Get the scitillator width
   */
  double getScintillatorWidth() const { return scint_width_; }

  /**
   * Get the number of sections.
   */
  int getNumSections() const { return num_sections_; }

  /**
   * Get the number of layers for that section.
   */
  int getNumLayers(int isection) const { return num_layers_.at(isection); }

  /**
   * Get the number of strips per layer for that section and layer.
   */
  int getNumStrips(int isection, int layer = 1) const {
    auto layer_index = layer - 1;
    return num_strips_.at(isection).at(layer_index);
  }

  /**
   * Get the location of the zeroStrip in a given section and layer
   * */
  int getZeroStrip(int isection, int layer = 1) const {
    auto layer_index = layer - 1;
    return zero_strip_.at(isection).at(layer_index);
  }

  /**
   * Get the length of the Ecal in (x) for the side Hcal.
   */
  double getEcalDx() const { return ecal_dx_; }

  /**
   * Get the length of the Ecal in (y) for the side Hcal
   */
  double getEcalDy() const { return ecal_dy_; }

  /**
   * Does the Side Hcal have 3D readout?
   *
   * In other words, does the side hcal layers alter in scintillator direction
   * (z vs x/y).
   */
  bool hasSide3DReadout() const { return side_3d_readout_; }

  /*
   * Is the hcal geometry one of the geometries used for the CERN testbeam
   * activities?
   */
  bool isPrototype() const { return is_prototype_; }

  /**
   * Coordinates that are given by Geant4 are typically global. These can be
   * transformed into corresponding local coordinates of a volume with a
   * TopTransform. However, a TopTransform translates to the local bar but does
   * not do the rotation (this is because we don't do a rotation when placing
   * the bars in the GDML). This is used primarily for recording pre and post
   * step positions in local coordinates of the volume in HcalSD.
   *
   * the logic below does the rotation to the local coordiates where
   *  x : short transverse side of bar
   *  y : long transverse side of bar
   *  z : along length of bar
   *
   * @note This logic only applies to the v14 and prototype detector; however,
   * support for v12 is not broken because no studies using these pre/post step
   * positions have been (or should be) done with the v12 detector.
   /
   * @note: The native position type in Geant4 is typically a G4ThreeVector,
   * which is a typedef for a CLHEP::Hep3Vector. However, DetDescr currently
   * does not have a dependency on Geant4/CLHEP so we are taking the position
   * as a vector of floats (which is what is used by SimCalorimeterHit)
   **/
  std::vector<double>
  rotateGlobalToLocalBarPosition(const std::vector<double> &globalPosition,
                                 const ldmx::HcalID &id) const;

private:
  /**
   * Class constructor, for use only by the provider
   *
   * @param ps Parameters to configure the HcalGeometry
   */
  HcalGeometry(const framework::config::Parameters &ps);
  friend class hcal::HcalGeometryProvider;

  /**
   * Map builder of HcalID and position.
   * To build the map we loop over the number of Hcal sections, layers and
   * strips. The Hcal sections range from 0 to 4. (We hard-code the number of
   * sections as seen in HcalID) The Hcal layers range from 1 to
   * NumLayers_[section]. The Hcal strips range from 0 to NumStrips_[section].
   *
   * Odd layers have horizontal strips.
   * Even layers have vertical strips.
   */
  void buildStripPositionMap();
  /**
   * Debugging utility, prints out the HcalID and corresponding value of all
   * entries in the strip_position_map_ for a given section.
   *
   * @param section The section number to print, see HcalID for details.
   */
  void printPositionMap(int section) const;
  /**
   * Debugging utility, prints out the HcalID and corresponding value of all
   * entries in the strip_position_map_. For printing only one of the sections,
   * see the overloaded version of this function taking a section parameter.
   *
   */
  void printPositionMap() const {
    for (int section = 0; section < num_sections_; ++section) {
      printPositionMap(section);
    }
  }

private:
  /// Parameters that apply to all types of geometries
  /// Verbosity, not configurable but helpful if developing
  int verbose_{0};

  /// Thickness of scintillator
  double scint_thickness_;

  /// Width of Scintillator Strip [mm]
  double scint_width_;

  /// Front of HCal relative to world geometry for each section [mm]
  std::vector<double> zero_layer_;

  /// Thickness of the layers in each section [mm]
  std::vector<double> layer_thickness_;

  /// Number of layers in each section
  std::vector<int> num_layers_;

  /// Number of sections
  int num_sections_;

  /// Lenght of the Ecal (in x and y)
  double ecal_dx_;
  double ecal_dy_;

  // Offset of the entire Hcal geometry in y (mm)
  double y_offset_;
  // Defines what parity (0/1, i.e. even/odd parity) of a layer number in the
  // geometry that corresponds to a horizontal layer (scintillator bar length
  // along the x-axis) in the back HCal.
  int back_horizontal_parity_{};

  // 3D readout for side Hcal
  int side_3d_readout_{};

  /// Number of strips per layer in each section and each layer
  std::vector<std::vector<int>> num_strips_;
  /// The plane of the zero'th strip of each section [mm]
  std::vector<std::vector<double>> zero_strip_;
  /// Half Total Width of Strips [mm]
  std::vector<std::vector<double>> half_total_width_;
  /// Length of strips [mm]
  std::vector<std::vector<double>> scint_length_;

  bool is_prototype_{};

  /**
   Map of the HcalID position of strip centers relative to world geometry.
   The map is not configurable and is calculated by buildStripPositionMap().
   */
  std::map<ldmx::HcalID, TVector3> strip_position_map_;
};

} // namespace ldmx

#endif
