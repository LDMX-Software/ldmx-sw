/**
 * @file DetectorGeometry.h
 * @author Tom Eichlersmith, University of Minnesota
 * @brief Header file for class DetectorGeometry
 */

#ifndef EVENTDISPLAY_DETECTORGEOMETRY_H
#define EVENTDISPLAY_DETECTORGEOMETRY_H

// STL
#include <cmath>     //sqrt
#include <iostream>  //cerr
#include <map>       //storage maps
#include <memory>    //unique_ptr
#include <utility>   //BoundingBox
#include <vector>    //BoundingBox

// LDMX Framework
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/HcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Hcal/Event/HcalHit.h"
#include "SimCore/Event/SimTrackerHit.h"  //recoil hits

namespace eventdisplay {

/**
 * @type BoundingBox
 * @brief Stores the minimum and maximum of each coordinate for a box.
 *
 * This has all of the information needed to define an axis-aligned rectangular
 * prism.
 */
typedef std::vector<std::pair<double, double> > BoundingBox;

/**
 * @struct HexPrism
 * @brief Stores the necessary geometry details for a hexagonal prism.
 */
struct HexPrism {
  double x_;
  double y_;
  double z_;
  double height_;
  double radius_;
};

/**
 * @class DetectorGeometry
 * @brief Class to translated between detector location (section, layer, strip)
 * and real space.
 */
class DetectorGeometry {
 public:
  /**
   * Get the single instance of this class.
   */
  static const DetectorGeometry& getInstance() {
    static const DetectorGeometry detector_geometry;
    return detector_geometry;
  }

  /**
   * Calculate real space coordinates from detector location.
   *
   * @param hit HcalHit to find real space hit for
   * @return BoundingBox in real space
   */
  BoundingBox getBoundingBox(const ldmx::HcalHit& hit) const;

  /**
   * Calculate real space coordinates of a cluster of hits.
   *
   * Determines cluster's coordinates by a weighted mean of the individuals.
   *
   * @param hitVec vector of HcalHits to find a "center" for
   * @return BoundingBox in real space
   */
  BoundingBox getBoundingBox(const std::vector<ldmx::HcalHit>& hitVec) const;

  /**
   * Get bounding box for the input section.
   *
   * @param section HcalID::HcalSection
   * @return BoundingBox that bounds section
   */
  BoundingBox getBoundingBox(ldmx::HcalID::HcalSection section) const;

  /**
   * Calculate bounding hexagonal prism for input EcalHit.
   *
   * @param id EcalID for the hit
   * @return HexPrism
   */
  HexPrism getHexPrism(const ldmx::EcalID& id) const;

  /**
   * Get HexPrism for a tower
   *
   * @param towerIndex int index of hexagonal tower
   * @return HexPrism
   */
  HexPrism getHexTower(int towerIndex) const;

  /**
   * Get Rotation Angle around z-axis for the input layerID and moduleID
   *
   * @param layerID index for layer of recoil module
   * @param moduleID index for module of recoil module
   * @return rotation angle in radians
   */
  double getRotAngle(int layerID, int moduleID) const;

  /**
   * Get Bounding Box for input recoil module
   * NOTE: This does not take into account any rotation! Use getRotAngle as
   * well!
   *
   * @param layerID index for layer of module
   * @param moduleID index for module
   * @return BoundingBox that bounds the module
   */
  BoundingBox getBoundingBox(int layerID, int moduleID) const;

  /**
   * Get Bounding Box for input recoil hit
   * NOTE: This does not take into account any rotation! Use getRotAngle as
   * well!
   *
   * @param recoilHit SimTrackerHit in recoil tracker
   * @return BoundingBox that bounds the hit
   */
  BoundingBox getBoundingBox(const ldmx::SimTrackerHit& recoilHit) const;

 private:
  /**
   * Constructor
   * This is where all the detector constants are set.
   *
   * It is private so that the user is forced to use the single instance
   * acquired from getInstance() above.
   */
  DetectorGeometry();

  /////////////////////////////////////////////////////////////
  // HCAL

  /** Number of layers in each section */
  std::map<ldmx::HcalID::HcalSection, int> hcal_n_layers_;

  /** Number of strips per layer in each section */
  std::map<ldmx::HcalID::HcalSection, int> hcal_n_strips_;

  /** Length of Scintillator Strip [mm] */
  std::map<ldmx::HcalID::HcalSection, double> hcal_length_scint_;

  /** The plane of the zero'th layer of each section [mm] */
  std::map<ldmx::HcalID::HcalSection, double> hcal_zero_layer_;

  /** The plane of the zero'th strip of each section [mm] */
  std::map<ldmx::HcalID::HcalSection, double> hcal_zero_strip_;

  /** Thickness of the layers in each seciton [mm] */
  std::map<ldmx::HcalID::HcalSection, double> hcal_layer_thickness_;

  /** an example layer number of a vertical layer */
  int hcal_parity_vertical_;

  /** Uncertainty in timing position along a bar/strip [mm] */
  double hcal_uncertainty_timing_pos_;

  /** Thickness of Scintillator Strip [mm] */
  double hcal_thickness_scint_;

  /** Width of Scintillator Strip [mm] */
  double hcal_width_scint_;

  /////////////////////////////////////////////////////////////
  // ECAL

  /** Thickness of sensitive Si layers */
  double ecal_si_thickness_;

  /** Total depth of ECAL (length in Z direction) */
  double ecal_depth_;

  /** z-coordinate of plane for first ecal layer [mm] */
  double ecal_zero_layer_;

  /** Helper class to calculate (x,y) coordinate from hexagons */
  std::unique_ptr<ldmx::EcalHexReadout> ecal_hex_reader_;

  /////////////////////////////////////////////////////////////
  // RECOIL TRACKER

  double recoil_stereo_strip_length_;

  double recoil_stereo_x_width_;

  double recoil_stereo_y_width_;

  double recoil_stereo_separation_;

  double recoil_stereo_angle_;

  double recoil_mono_strip_length_;

  double recoil_mono_x_width_;

  double recoil_mono_y_width_;

  double recoil_mono_separation_;

  double recoil_sensor_thickness_;

  /** position of each module in recoil detector
   * The key in this map is 10*layerID+moduleID
   */
  std::map<int, std::vector<double> > recoil_module_pos_;

  /** angular tilt for each module in recoil detector
   * The key in this map is 10*layerID+moduleID
   */
  std::map<int, double> recoil_module_angle_;
};

}  // namespace eventdisplay

#endif /* EVENTDISPLAY_DETECTORGEOMETRY_H */
