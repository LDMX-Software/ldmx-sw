#include "DetDescr/HcalGeometry.h"

#include <assert.h>

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace ldmx {

HcalGeometry::HcalGeometry(const framework::config::Parameters &ps)
    : framework::ConditionsObject(HcalGeometry::CONDITIONS_OBJECT_NAME) {
  scint_thickness_ = ps.getParameter<double>("scint_thickness");
  scint_width_ = ps.getParameter<double>("scint_width");
  zero_layer_ = ps.getParameter<std::vector<double>>("zero_layer");
  layer_thickness_ = ps.getParameter<std::vector<double>>("layer_thickness");
  num_layers_ = ps.getParameter<std::vector<int>>("num_layers");
  num_sections_ = ps.getParameter<int>("num_sections");
  ecal_dx_ = ps.getParameter<double>("ecal_dx");
  ecal_dy_ = ps.getParameter<double>("ecal_dy");
  verbose_ = ps.getParameter<int>("verbose");
  back_horizontal_parity_ = ps.getParameter<int>("back_horizontal_parity");
  side_3d_readout_ = ps.getParameter<int>("side_3d_readout");
  y_offset_ = ps.getParameter<double>("y_offset");

  auto detectors_valid =
      ps.getParameter<std::vector<std::string>>("detectors_valid");
  // If one of the strings in detectors_valid is "ldmx-hcal-prototype", we
  // will use prototype geometry initialization
  is_prototype_ = std::find_if(detectors_valid.cbegin(), detectors_valid.cend(),
                               [](const auto detector) {
                                 return detector.find("ldmx-hcal-prototype") !=
                                        std::string::npos;
                               }) != detectors_valid.cend();

  num_strips_ = ps.getParameter<std::vector<std::vector<int>>>("num_strips");
  half_total_width_ =
      ps.getParameter<std::vector<std::vector<double>>>("half_total_width");
  zero_strip_ = ps.getParameter<std::vector<std::vector<double>>>("zero_strip");
  scint_length_ =
      ps.getParameter<std::vector<std::vector<double>>>("scint_length");

  buildStripPositionMap();

  if (verbose_ > 0) {
    printPositionMap();
  }
}
std::vector<double> HcalGeometry::rotateGlobalToLocalBarPosition(
    const std::vector<double> &globalPosition, const ldmx::HcalID &id) const {
  const auto orientation{getScintillatorOrientation(id)};
  switch (id.section()) {
    case ldmx::HcalID::HcalSection::BACK:
      switch (orientation) {
        case ScintillatorOrientation::horizontal:
          return {globalPosition[2], globalPosition[1], globalPosition[0]};
        case ScintillatorOrientation::vertical:
          return {globalPosition[2], globalPosition[0], globalPosition[1]};
      }
    case ldmx::HcalID::HcalSection::TOP:
    case ldmx::HcalID::HcalSection::BOTTOM:
      switch (orientation) {
        case ScintillatorOrientation::horizontal:
          return {globalPosition[1], globalPosition[2], globalPosition[0]};
        case ScintillatorOrientation::depth:
          return {globalPosition[1], globalPosition[0], globalPosition[2]};
      }
    case ldmx::HcalID::HcalSection::LEFT:
    case ldmx::HcalID::HcalSection::RIGHT:
      switch (orientation) {
        case ScintillatorOrientation::vertical:
          return {globalPosition[0], globalPosition[2], globalPosition[1]};
        case ScintillatorOrientation::depth:
          return globalPosition;
      }
  }
}

HcalGeometry::ScintillatorOrientation HcalGeometry::getScintillatorOrientation(
    const ldmx::HcalID id) const {
  if (hasSide3DReadout()) {
    // v14 or later detector
    switch (id.section()) {
      case ldmx::HcalID::HcalSection::TOP:
      case ldmx::HcalID::HcalSection::BOTTOM:
        // Odd layers are in z/depth direction, even are in the x/horizontal
        // direction
        return id.layer() % 2 == 0 ? ScintillatorOrientation::horizontal
                                   : ScintillatorOrientation::depth;

      case ldmx::HcalID::HcalSection::LEFT:
      case ldmx::HcalID::HcalSection::RIGHT:
        // Odd layers are in the z/depth direction, even are in the y/vertical
        // direction
        return id.layer() % 2 == 0 ? ScintillatorOrientation::vertical
                                   : ScintillatorOrientation::depth;
      case ldmx::HcalID::HcalSection::BACK:
        // Configurable
        return id.layer() % 2 == back_horizontal_parity_
                   ? ScintillatorOrientation::horizontal
                   : ScintillatorOrientation::vertical;
    }  // V14 or later detector
  }
  if (isPrototype()) {
    // The prototype only has the back section. However, the orientation
    // depends on the configuration so we delegate to the
    // back_horizontal_parity parameter
    return id.layer() % 2 == back_horizontal_parity_
               ? ScintillatorOrientation::horizontal
               : ScintillatorOrientation::vertical;
  }  // Prototype detector
  // v13/v12
  switch (id.section()) {
    // For the v13 side hcal, the bars in each section have the same
    // orientation
    case ldmx::HcalID::HcalSection::TOP:
    case ldmx::HcalID::HcalSection::BOTTOM:
      return ScintillatorOrientation::horizontal;
    case ldmx::HcalID::HcalSection::LEFT:
    case ldmx::HcalID::HcalSection::RIGHT:
      return ScintillatorOrientation::vertical;
    case ldmx::HcalID::HcalSection::BACK:
      // Configurable
      return id.layer() % 2 == back_horizontal_parity_
                 ? ScintillatorOrientation::horizontal
                 : ScintillatorOrientation::vertical;
  }  // v13/v12 detector
}
void HcalGeometry::printPositionMap(int section) const {
  // Note that layer numbering starts at 1 rather than 0
  for (int layer = 1; layer <= num_layers_[section]; ++layer) {
    for (int strip = 0; strip < getNumStrips(section, layer); ++strip) {
      HcalID id(section, layer, strip);
      auto centerPosition = getStripCenterPosition(id);
      auto x = centerPosition[0];
      auto y = centerPosition[1];
      auto z = centerPosition[2];
      std::cout << id << ": Center position: (" << x << ", " << y << ", " << z
                << ")\n";
    }
  }
}

void HcalGeometry::buildStripPositionMap() {
  // We hard-code the number of sections as seen in HcalID
  for (int section = 0; section < num_sections_; section++) {
    for (int layer = 1; layer <= num_layers_[section]; layer++) {
      for (int strip = 0; strip < getNumStrips(section, layer); strip++) {
        // initialize values
        double x{-99999}, y{-99999}, z{-99999};

        // get hcal section
        ldmx::HcalID::HcalSection hcalsection =
            (ldmx::HcalID::HcalSection)section;

        const ldmx::HcalID id{section, layer, strip};
        const auto orientation{getScintillatorOrientation(id)};
        // the center of a layer: (layer-1) * (layer_thickness) +
        // scint_thickness/2
        double layercenter =
            (layer - 1) * layer_thickness_.at(section) + 0.5 * scint_thickness_;

        // the center of a strip: (strip + 0.5) * (strip_dx)
        double stripcenter = (strip + 0.5) * scint_width_;

        if (hcalsection == ldmx::HcalID::HcalSection::BACK) {
          /**
             For back Hcal:
             - layers in z
             - strips occupy thickness of scintillator in z (e.g. 20mm)
             - strips orientation is in x(y) depending on back_horizontal
             parity
          */
          // z position: zero-layer(z) + layer_z + scint_thickness / 2
          z = zero_layer_.at(section) + layercenter;

          /**
            Now compute, y(x) position for horizontal(vertical) layers,
            relative to the center of detector. Strips enumeration starts from
            -y(-x) stripcenter will be large for +y(+x) and the half width of
            the strip needs to be subtracted The halfwidth of the scintillator
            is given by half_total_width_. The x(y) position is set to the
            center of the strip (0).
          */
          if (orientation == ScintillatorOrientation::horizontal) {
            y = stripcenter - getZeroStrip(section, layer);
            x = 0;
          } else {
            x = stripcenter - getZeroStrip(section, layer);
            y = 0;
          }
        } else {
          /**
          For side Hcal before 3D readout
          - layers in y(x)
          - all layers have strips in x(y) for top-bottom (left-right) sections
          - all layers have strips occupying width of scintillator in z (e.g.
          50mm)
          For 3D readout:
          - odd layers have strips in z
          - even layers have strips in x(y) for top-bottom (left-right) sections
          - odd layers have strips occupying width of scintillator in x(y)
          - even layers have strips occupying width of scintillator in z
          */
          if (side_3d_readout_ &&
              orientation == ScintillatorOrientation::depth) {
            // z position: zero-strip + half-width (center_strip) of strip
            z = getZeroStrip(section, layer) +
                getHalfTotalWidth(section, layer);
          } else {
            // z position: zero-strip(z) + strip_center(z)
            z = getZeroStrip(section, layer) + stripcenter;
          }

          if (hcalsection == ldmx::HcalID::HcalSection::TOP or
              hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
            y = zero_layer_.at(section) + layercenter;
            x = getHalfTotalWidth(section, layer);
            if (side_3d_readout_ &&
                orientation == ScintillatorOrientation::horizontal) {
              x = getZeroStrip(section, layer) + stripcenter;
            }
            if (hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
              y *= -1;
              x *= -1;
            }
          } else {
            x = zero_layer_.at(section) + layercenter;
            y = getHalfTotalWidth(section, layer);
            if (side_3d_readout_ &&
                orientation == ScintillatorOrientation::vertical) {
              y = getZeroStrip(section, layer) + stripcenter;
            }
            if (hcalsection == ldmx::HcalID::HcalSection::RIGHT) {
              x *= -1;
              y *= -1;
            }
          }
        }

        y += y_offset_;
        TVector3 pos;
        pos.SetXYZ(x, y, z);
        strip_position_map_[ldmx::HcalID(section, layer, strip)] = pos;
      }  // loop over strips
    }    // loop over layers
  }      // loop over sections
}  // strip position map

}  // namespace ldmx
