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
  static constexpr const char* CONDITIONS_OBJECT_NAME{"HcalGeometry"};

  /**
   * Class destructor.
   *
   * Does nothing because the stl containers clean up automatically.
   */
  ~HcalGeometry() = default; 

  /**
   * Get a strip center position from a combined hcal ID.
   *
   * @throw std::out_of_range if HcalID is not on map.
   *
   * @param HcalID
   * @return A TVector3 with the X, Y and Z position of the center of the bar.
   */
  TVector3 getStripCenterPosition(ldmx::HcalID id) const {
    return stripPositionMap_.at(id);
  }

  /**
   * Get the half total width of a layer for a given section(strip) for back(side) Hcal.
   * @param section
   * @param layer
   * @return half total width [mm]
   */
  double getHalfTotalWidth(int isection, int layer = 1) const {
    auto layer_index = layer - 1;
    return HalfTotalWidth_.at(isection).at(layer_index);
  }

  /**
   * Get the number of sections.
   */
  int getNumSections() const { return NumSections_; }

  /**
   * Get the number of layers for that section.
   */
  int getNumLayers(int isection) const { return NumLayers_.at(isection); }

  /**
   * Get the number of strips per layer for that section and layer.
   */
  int getNumStrips(int isection, int layer = 1) const {
    auto layer_index = layer - 1;
    return NumStrips_.at(isection).at(layer_index);
  }

  /**
   * Get the location of the zeroStrip in a given section and layer
   * */
  int getZeroStrip(int isection, int layer = 1) const {
    auto layer_index = layer - 1;
    return ZeroStrip_.at(isection).at(layer_index);
    }


  /**
   * Get the length of the Ecal in (x) for the side Hcal.
   */
  double getEcalDx() const { return EcalDx_; }

  /**
   * Get the length of the Ecal in (y) for the side Hcal
   */
  double getEcalDy() const { return EcalDy_; }

 private:
  /**
   * Class constructor, for use only by the provider
   *
   * @param ps Parameters to configure the HcalGeometry
   */
  HcalGeometry(const framework::config::Parameters& ps);
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
   * entries in the stripPositionMap_ for a given section.
   *
   * @param section The section number to print, see HcalID for details.
   */
    void printPositionMap(int section) const;
  /**
   * Debugging utility, prints out the HcalID and corresponding value of all
   * entries in the stripPositionMap_. For printing only one of the sections,
   * see the overloaded version of this function taking a section parameter.
   *
   */
    void printPositionMap() const {
      for (int section = 0; section < NumSections_ ; ++section) {
        printPositionMap(section);
      }
    }

   // Deserves a better name
   //
   // Takes a parameter which only has dimensions of section and creates a
   // version with both section and layer. All layers in a given section will be
   // copies.
   template <typename T>
   std::vector<std::vector<T>> makeCanonicalLayeredParameter(const std::vector<T>& parameter) {
     std::vector<std::vector<T>> result;
     for(auto section = 0; section < parameter.size(); ++section) {
       result.push_back(std::vector<T>(NumLayers_[section], parameter[section]));
     }
     return result;
   }

private:
  /// Parameters that apply to all types of geometries
  /// Verbosity, not configurable but helpful if developing
  int verbose_{0};

  /// Thickness of scintillator
  double ThicknessScint_;

  /// Width of Scintillator Strip [mm]
  double WidthScint_;

  /// Front of HCal relative to world geometry for each section [mm]
  std::vector<double> ZeroLayer_;

  /// Thickness of the layers in each section [mm]
  std::vector<double> LayerThickness_;

  /// Number of layers in each section
  std::vector<int> NumLayers_;


  /// Number of sections
  int NumSections_;

  /// Lenght of the Ecal (in x and y)
  double EcalDx_;
  double EcalDy_;

  // Defines what parity (0/1, i.e. even/odd parity) of a layer number in the
  // geometry that corresponds to a horizontal layer (scintillator bar length
  // along the x-axis).
  int horizontalParity_{};

  /// Canonical layered version of parameters that differ between geometry
  /// versions.

  /// Number of strips per layer in each section and each layer
  std::vector<std::vector<int>> NumStrips_;
  /// The plane of the zero'th strip of each section [mm]
  std::vector<std::vector<double>> ZeroStrip_;
  /// Half Total Width of Strips [mm]
  std::vector<std::vector<double>> HalfTotalWidth_;
  /**
   Map of the HcalID position of strip centers relative to world geometry.
   The map is not configurable and is calculated by buildStripPositionMap().
   */
  std::map<ldmx::HcalID, TVector3> stripPositionMap_;
};

}  // namespace ldmx

#endif
