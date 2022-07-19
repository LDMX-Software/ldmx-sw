#include "DetDescr/HcalGeometry.h"

#include <assert.h>

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace ldmx {

HcalGeometry::HcalGeometry(const framework::config::Parameters& ps)
    : framework::ConditionsObject(HcalGeometry::CONDITIONS_OBJECT_NAME) {
  ThicknessScint_ = ps.getParameter<double>("ThicknessScint");
  WidthScint_ = ps.getParameter<double>("WidthScint");
  ZeroLayer_ = ps.getParameter<std::vector<double>>("ZeroLayer");
  LayerThickness_ = ps.getParameter<std::vector<double>>("LayerThickness");
  NumLayers_ = ps.getParameter<std::vector<int>>("NumLayers");
  NumSections_ = ps.getParameter<int>("NumSections");
  EcalDx_ = ps.getParameter<double>("EcalDx");
  EcalDy_ = ps.getParameter<double>("EcalDy");
  verbose_ = ps.getParameter<int>("verbose");
  horizontalParity_ = ps.getParameter<int>("horizontal_parity");

  auto detectors_valid =
      ps.getParameter<std::vector<std::string>>("detectors_valid");
  // If one of the strings in detectors_valid is "ldmx-hcal-prototype", we will
  // use prototype geometry initialization
  //
  auto is_prototype =
      std::find_if(detectors_valid.cbegin(), detectors_valid.cend(),
                   [](const auto detector) {
                     return detector.find("ldmx-hcal-prototype") !=
                            std::string::npos;
                   }) != detectors_valid.cend();
  if (is_prototype) {
    auto ZeroStrip_prototype_ =
        ps.getParameter<std::vector<double>>("ZeroStrip", {});
    auto NumStrips_prototype_ =
        ps.getParameter<std::vector<int>>("NumStrips", {});
    auto HalfTotalWidth_prototype_ =
        ps.getParameter<std::vector<double>>("HalfTotalWidth", {});
    // The prototype only has one section, so we only have one entry into these
    // vectors
    ZeroStrip_ = {HalfTotalWidth_prototype_};
    NumStrips_ = {NumStrips_prototype_};
    HalfTotalWidth_ = {HalfTotalWidth_prototype_};
  } else {
    auto ZeroStrip_v12_ = ps.getParameter<std::vector<double>>("ZeroStrip", {});
    auto NumStrips_v12_ = ps.getParameter<std::vector<int>>("NumStrips", {});
    auto HalfTotalWidth_v12_ =
        ps.getParameter<std::vector<double>>("HalfTotalWidth", {});

    ZeroStrip_ = makeCanonicalLayeredParameter(ZeroStrip_v12_);
    NumStrips_ = makeCanonicalLayeredParameter(NumStrips_v12_);
    HalfTotalWidth_ = makeCanonicalLayeredParameter(HalfTotalWidth_v12_);
  }
  buildStripPositionMap();
}
void HcalGeometry::printPositionMap(int section) const {
  // Note that layer numbering starts at 1 rather than 0
  for (int layer = 1; layer <= NumLayers_[section]; ++layer) {
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
  for (int section = 0; section < NumSections_; section++) {
    for (int layer = 1; layer <= NumLayers_[section]; layer++) {
      for (int strip = 0; strip < getNumStrips(section, layer); strip++) {
        // initialize values
        double x{-99999}, y{-99999}, z{-99999};

        // get hcal section
        ldmx::HcalID::HcalSection hcalsection =
            (ldmx::HcalID::HcalSection)section;

        // the center of a layer: layer * (layer_dz) + (layer_dz)/2
        double layercenter =
            (layer - 1) * LayerThickness_.at(section) + 0.5 * ThicknessScint_;

        // the center of a strip: (strip + 0.5) * (strip_dx)
        double stripcenter = (strip + 0.5) * WidthScint_;

        /**
          For back Hcal:
          - layers in z
          - strips occupy thickness of scintillator in z (e.g. 20mm)
          - strips orientation is in x(y) for odd(even) layers
          For side Hcal:
          - layers in x(y)
          - strips occupy width of scintillator in z (e.g. 50mm)
          - strips orientation is in x(y) for top-bottom(left-right) sections
        */
        if (hcalsection == ldmx::HcalID::HcalSection::BACK) {
          // z position: zero-layer(z) + layer_z + thickness_scint
          z = ZeroLayer_.at(section) + layercenter;

          /**
            Now compute, y(x) position for horizontal(vertical) layers, relative
            to the center of detector. Strips enumeration starts from -y(-x)
            stripcenter will be large for +y(+x) and the halfwidth of the strip
            needs to be subtracted The halfwidth of the scintillator is given by
            ZeroStrip_. The x(y) position is set to the center of the strip (0).
          */
          if (layerIsHorizontal(layer)) {
            y = stripcenter - getZeroStrip(section, layer);
            x = 0;
          } else {
            x = stripcenter - getZeroStrip(section, layer);
            y = 0;
          }
        } else {
          // z position: zero-strip(z) + strip_center(z)
          z = getZeroStrip(section, layer) + stripcenter;

          /**
            Top and Bottom sections have strips orientated in x
            The y coordinate will be given by layercenter + ZeroLayer_(y)
          */
          if (hcalsection == ldmx::HcalID::HcalSection::TOP or
              hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
            y = ZeroLayer_.at(section) + layercenter;
            x = getHalfTotalWidth(section, layer);
            if (hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
              y *= -1;
              x *= -1;
            }
          } else {
            x = ZeroLayer_.at(section) + layercenter;
            y = getHalfTotalWidth(section, layer);
            if (hcalsection == ldmx::HcalID::HcalSection::RIGHT) {
              x *= -1;
              y *= -1;
            }
          }
        }
        TVector3 pos;
        pos.SetXYZ(x, y, z);
        stripPositionMap_[ldmx::HcalID(section, layer, strip)] = pos;
      }  // loop over strips
    }    // loop over layers
  }      // loop over sections
}  // strip position map

}  // namespace ldmx
