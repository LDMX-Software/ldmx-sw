#include "DetDescr/HcalGeometry.h"

#include <assert.h>

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace ldmx {

HcalGeometry::HcalGeometry(const framework::config::Parameters& ps)
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
  horizontal_parity_ = ps.getParameter<int>("horizontal_parity");
  side_3d_readout_ = ps.getParameter<int>("side_3d_readout");
  
  auto detectors_valid =
      ps.getParameter<std::vector<std::string>>("detectors_valid");
  // If one of the strings in detectors_valid is "ldmx-hcal-prototype", we will
  // use prototype geometry initialization
  auto is_prototype =
      std::find_if(detectors_valid.cbegin(), detectors_valid.cend(),
                   [](const auto detector) {
                     return detector.find("ldmx-hcal-prototype") !=
                            std::string::npos;
                   }) != detectors_valid.cend();
  
  if (is_prototype) {
    auto zero_strip_prototype_ =
        ps.getParameter<std::vector<double>>("zero_strip", {});
    auto num_strips_prototype_ =
        ps.getParameter<std::vector<int>>("num_strips", {});
    auto half_total_width_prototype_ =
        ps.getParameter<std::vector<double>>("half_total_width", {});
    // The prototype only has one section, so we only have one entry into these
    // vectors
    zero_strip_ = {zero_strip_prototype_};
    num_strips_ = {num_strips_prototype_};
    half_total_width_ = {half_total_width_prototype_};
  }
  else if (side_3d_readout_==1) {
    num_strips_ = ps.getParameter<std::vector<std::vector<int>>>("num_strips", {});
    half_total_width_ = ps.getParameter<std::vector<std::vector<double>>>("half_total_width", {});
    zero_strip_ = ps.getParameter<std::vector<std::vector<double>>>("zero_strip", {});
  }
  else {
    auto zero_strip_v12_ = ps.getParameter<std::vector<double>>("zero_strip", {});
    auto num_strips_v12_ = ps.getParameter<std::vector<int>>("num_strips", {});
    auto half_total_width_v12_ = ps.getParameter<std::vector<double>>("half_total_width", {});

    zero_strip_ = makeCanonicalLayeredParameter(zero_strip_v12_);
    num_strips_ = makeCanonicalLayeredParameter(num_strips_v12_);
    half_total_width_ = makeCanonicalLayeredParameter(half_total_width_v12_);
  }

  buildStripPositionMap();
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

        // the center of a layer: (layer-1) * (layer_thickness) + scint_thickness/2
        double layercenter =
            (layer - 1) * layer_thickness_.at(section) + 0.5 * scint_thickness_;

        // the center of a strip: (strip + 0.5) * (strip_dx)
        double stripcenter = (strip + 0.5) * scint_width_;

        if (hcalsection == ldmx::HcalID::HcalSection::BACK) {
	  /**
	     For back Hcal:
	     - layers in z
	     - strips occupy thickness of scintillator in z (e.g. 20mm)
	     - strips orientation is in x(y) depending on horizontal parity
	  */
          // z position: zero-layer(z) + layer_z + scint_thickness
          z = zero_layer_.at(section) + layercenter;

          /**
            Now compute, y(x) position for horizontal(vertical) layers, relative
            to the center of detector. Strips enumeration starts from -y(-x)
            stripcenter will be large for +y(+x) and the half width of the strip
            needs to be subtracted The halfwidth of the scintillator is given by
            half_total_width_.
	    The x(y) position is set to the center of the strip (0).
          */
          if (layerIsHorizontal(layer)) {
            y = stripcenter - getZeroStrip(section, layer);
            x = 0;
          } else {
            x = stripcenter - getZeroStrip(section, layer);
            y = 0;
          }
	  // std::cout << "back (section,layer,strip)" << section << " " << layer << " " << strip;
	  // std::cout << " (x,y,z) " << x << " " << y << " " << z << std::endl;
        } else {
	  /**
	     For side Hcal
	     - layers in y(x)
	     - all layers have strips in x(y) for top-bottom (left-right) sections
	     - all layers have strips occupying width of scintillator in z (e.g. 50mm)
	     For 3D readout:
	     - odd layers have strips in z
	     - even layers have strips in x(y) for top-bottom (left-right) sections
	     - odd layers have strips occupying width of scintillator in x(y)
	     - even layers have strips occupying width of scintillator in z
	  */
	  if(side_3d_readout_ && (layer%2==1)){
	    // z position: zero-strip + half-width (center_strip) of strip
	    z = getZeroStrip(section, layer) + getHalfTotalWidth(section, layer);
	  }
	  else {
	    // z position: zero-strip(z) + strip_center(z)
	    z = getZeroStrip(section, layer) + stripcenter;
	  }
	  
          if (hcalsection == ldmx::HcalID::HcalSection::TOP or
              hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
	    y = zero_layer_.at(section) + layercenter;
	    x = getHalfTotalWidth(section, layer);
	    if(side_3d_readout_ && layer%2==1)
	      x = getZeroStrip(section, layer) + stripcenter;
	    if (hcalsection == ldmx::HcalID::HcalSection::BOTTOM) {
	      y *= -1;
	      x *= -1;
	    }
          } else {
            x = zero_layer_.at(section) + layercenter;
            y = getHalfTotalWidth(section, layer);
	    if(side_3d_readout_ && layer%2==1)
	      y = getZeroStrip(section, layer) + stripcenter;
            if (hcalsection == ldmx::HcalID::HcalSection::RIGHT) {
              x *= -1;
              y *= -1;
            }
          }

	  // std::cout << "side (section,layer,strip)" << section << " " << layer << " " << strip;
          // std::cout << " (x,y,z) " << x << " " << y << " " << z << std::endl; 
        }
        TVector3 pos;
        pos.SetXYZ(x, y, z);
        strip_position_map_[ldmx::HcalID(section, layer, strip)] = pos;
      }  // loop over strips
    }    // loop over layers
  }      // loop over sections
}  // strip position map

}  // namespace ldmx
