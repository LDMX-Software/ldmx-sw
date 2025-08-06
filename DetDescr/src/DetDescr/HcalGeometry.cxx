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
        default:  // Should not be possible with current geometries
          EXCEPTION_RAISE("InvalidRotation",
                          "Attempted to rotate into an invalid "
                          "orientation for a scintillator bar!");
      }
    case ldmx::HcalID::HcalSection::TOP:
      [[fallthrough]];
    case ldmx::HcalID::HcalSection::BOTTOM:
      switch (orientation) {
        case ScintillatorOrientation::horizontal:
          return {globalPosition[1], globalPosition[2], globalPosition[0]};
        case ScintillatorOrientation::depth:
          return {globalPosition[1], globalPosition[0], globalPosition[2]};
        default:  // Should not be possible with current geometries
          EXCEPTION_RAISE("InvalidRotation",
                          "Attempted to rotate into an invalid "
                          "orientation for a scintillator bar!");
      }
    case ldmx::HcalID::HcalSection::LEFT:
      [[fallthrough]];
    case ldmx::HcalID::HcalSection::RIGHT:
      switch (orientation) {
        case ScintillatorOrientation::vertical:
          return {globalPosition[0], globalPosition[2], globalPosition[1]};
        case ScintillatorOrientation::depth:
          return globalPosition;
        default:  // Should not be possible with current geometries
          EXCEPTION_RAISE("InvalidRotation",
                          "Attempted to rotate into an invalid "
                          "orientation for a scintillator bar!");
      }
    default:
      // Can only reach this part if we somehow didn't match any of the options
      // above. This could happen if someone introduces a new geometry but
      // doesn't patch this part.
      EXCEPTION_RAISE("InvalidRotation",
                      "Attempted to rotate into an invalid "
                      "orientation for a scintillator bar!");
  }
}

HcalGeometry::ScintillatorOrientation HcalGeometry::getScintillatorOrientation(
    const ldmx::HcalID id) const {
  if (hasSide3DReadout()) {
    // v14 or later detector
    switch (id.section()) {
      case ldmx::HcalID::HcalSection::TOP:
      case ldmx::HcalID::HcalSection::BOTTOM:
        // Odd layers are in z_/depth direction, even are in the x_/horizontal
        // direction
        return id.layer() % 2 == 0 ? ScintillatorOrientation::horizontal
                                   : ScintillatorOrientation::depth;

      case ldmx::HcalID::HcalSection::LEFT:
      case ldmx::HcalID::HcalSection::RIGHT:
        // Odd layers are in the z_/depth direction, even are in the y_/vertical
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
  // Can only reach this part if we somehow didn't match any of the options
  // above. This could happen if someone introduces a new geometry but doesn't
  // patch this part.
  EXCEPTION_RAISE("InvalidRotation",
                  "Attempted to rotate into an invalid "
                  "orientation for a scintillator bar!");
}
void HcalGeometry::printPositionMap(int section) const {
  // Note that layer_ numbering starts at 1 rather than 0
  for (int layer_ = 1; layer_ <= num_layers_[section]; ++layer_) {
    for (int strip = 0; strip < getNumStrips(section, layer_); ++strip) {
      HcalID id(section, layer_, strip);
      auto centerPosition = getStripCenterPosition(id);
      auto x_ = centerPosition.X();
      auto y_ = centerPosition.Y();
      auto z_ = centerPosition.Z();
      std::cout << id << ": Center position: (" << x_ << ", " << y_ << ", "
                << z_ << ")\n";
    }
  }
}

void HcalGeometry::buildStripPositionMap() {
  // We hard-code the number of sections as seen in HcalID
  for (unsigned int section = 0; section < num_sections_; section++) {
    for (unsigned int layer_ = 1; layer_ <= num_layers_[section]; layer_++) {
      for (unsigned int strip = 0; strip < getNumStrips(section, layer_);
           strip++) {
        // initialize values
        double x_{-99999}, y_{-99999}, z_{-99999};

        // get hcal section
        ldmx::HcalID::HcalSection hcalsection =
            (ldmx::HcalID::HcalSection)section;

        const ldmx::HcalID id{section, layer_, strip};
        const auto orientation{getScintillatorOrientation(id)};
        // the center of a layer_: (layer_-1) * (layer_thickness) +
        // scint_thickness/2
        double layercenter = (layer_ - 1) * layer_thickness_.at(section) +
                             0.5 * scint_thickness_;

        // the center of a strip: (strip + 0.5) * (strip_dx)
        double stripcenter = (strip + 0.5) * scint_width_;

        if (hcalsection == ldmx::HcalID::HcalSection::BACK) {
          /**
             For back Hcal:
             - layers in z_
             - strips occupy thickness of scintillator in z_ (e.g. 20mm)
             - strips orientation is in x_(y_) depending on back_horizontal
             parity
          */
          // z_ position: zero-layer_(z_) + layer_z + scint_thickness / 2
          z_ = zero_layer_.at(section) + layercenter;

          /**
            Now compute, y_(x_) position for horizontal(vertical) layers,
            relative to the center of detector. Strips enumeration starts from
            -y_(-x_) stripcenter will be large for +y_(+x_) and the half width
            of the strip needs to be subtracted The halfwidth of the
            scintillator is given by half_total_width_. The x_(y_) position is
            set to the center of the strip (0).
          */
          if (orientation == ScintillatorOrientation::horizontal) {
            y_ = stripcenter - getZeroStrip(section, layer_);
            x_ = 0;
          } else {
            x_ = stripcenter - getZeroStrip(section, layer_);
            y_ = 0;
          }
        } else {
          if (side_3d_readout_) {
            /*
             *
             * For 3D readout:
             * - odd layers have strips in z_
             * - even layers have strips in x_(y_) for top-bottom (left-right)
             * sections
             * - odd layers have strips occupying width of scintillator in
             * x_(y_)
             * - even layers have strips occupying width of scintillator in z_
             *
             */
            switch (hcalsection) {
              case ldmx::HcalID::HcalSection::BACK:
                // Handled earlier in the code!
              case ldmx::HcalID::HcalSection::LEFT:
              case ldmx::HcalID::HcalSection::RIGHT:
                if (orientation == ScintillatorOrientation::vertical) {
                  x_ = zero_layer_[section] + 0.5 * scint_thickness_ +
                       (layer_ - 1) * layer_thickness_[section];
                  y_ = ecal_dy_ -
                       (getScintillatorLength({id.section(), 2, id.strip()}) -
                        getScintillatorLength(id)) /
                           2;
                  z_ = getZeroStrip(section, layer_) +
                       (strip + 0.5) * getScintillatorWidth();
                } else if (orientation == ScintillatorOrientation::depth) {
                  x_ = zero_layer_[section] + 0.5 * scint_thickness_ +
                       layer_thickness_[section] * (layer_ - 1);
                  y_ = -ecal_dy_ / 2 + (strip + 0.5) * getScintillatorWidth();
                  z_ = getZeroStrip(section, layer_ + 1) +
                       getScintillatorLength(id) / 2;
                }
                if (section == ldmx::HcalID::HcalSection::LEFT) {
                  y_ *= -1;
                  x_ *= -1;
                }
                break;

              case ldmx::HcalID::HcalSection::BOTTOM:
              case ldmx::HcalID::HcalSection::TOP:
                if (orientation == ScintillatorOrientation::horizontal) {
                  //
                  // Second half of the expression is the difference between the
                  // longest strips (first module_) and the current module_.
                  //
                  // 22 mm extra for space for 1 absorber and one air box
                  x_ = -ecal_dx_ / 2 - 2 - 20 +
                       (getScintillatorLength({id.section(), 2, id.strip()}) -
                        getScintillatorLength(id)) /
                           2;
                  y_ = zero_layer_[section] + 0.5 * scint_thickness_ +
                       (layer_ - 1) * layer_thickness_[section];
                  z_ = getZeroStrip(section, layer_) +
                       (strip + 0.5) * getScintillatorWidth();
                }
                if (orientation == ScintillatorOrientation::depth) {
                  x_ = (ecal_dx_ / 2) - (strip + 0.5) * getScintillatorWidth();
                  y_ = zero_layer_[section] + 0.5 * scint_thickness_ +
                       layer_thickness_[section] * (layer_ - 1);
                  z_ = getZeroStrip(section, layer_ + 1) +
                       getScintillatorLength(id) / 2;
                }
                if (section == ldmx::HcalID::HcalSection::BOTTOM) {
                  y_ *= -1;
                  x_ *= -1;
                }
                break;
            }

          } else {
            /**
            For side Hcal before 3D readout
            - layers in y_(x_)
            - all layers have strips in x_(y_) for top-bottom (left-right)
            sections
            - all layers have strips occupying width of scintillator in z_ (e.g.
            50mm)
            */

            // z_ position: zero-strip(z_) + strip_center(z_)
            z_ = getZeroStrip(section, layer_) + stripcenter;
            if (hcalsection == ldmx::HcalID::HcalSection::TOP or
                hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
              y_ = zero_layer_.at(section) + layercenter;
              x_ = getHalfTotalWidth(section, layer_);
              if (hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
                y_ *= -1;
                x_ *= -1;
              }

            } else {
              x_ = zero_layer_.at(section) + layercenter;
              y_ = getHalfTotalWidth(section, layer_);
              if (hcalsection == ldmx::HcalID::HcalSection::RIGHT) {
                x_ *= -1;
                y_ *= -1;
              }
            }
          }
        }

        y_ += y_offset_;
        ROOT::Math::XYZVector pos_;
        pos_.SetXYZ(x_, y_, z_);
        strip_position_map_[ldmx::HcalID(section, layer_, strip)] = pos_;
      }  // loop over strips
    }  // loop over layers
  }  // loop over sections
}  // strip position map

}  // namespace ldmx
