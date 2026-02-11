/**
 * @file DetectorGeometry.cxx
 * @brief Implementation file for class DetectorGeometry
 */

#include "EventDisplay/DetectorGeometry.h"

namespace eventdisplay {

DetectorGeometry::DetectorGeometry() {
  ///////////////////////////////////////////////////////////////////////////////////
  // HCAL

  hcal_parity_vertical_ = 1;

  hcal_uncertainty_timing_pos_ = 50.0;

  hcal_thickness_scint_ = 20.0;

  hcal_width_scint_ = 50.0;

  hcal_n_layers_[ldmx::HcalID::HcalSection::BACK] = 100;
  hcal_n_layers_[ldmx::HcalID::HcalSection::TOP] = 28;
  hcal_n_layers_[ldmx::HcalID::HcalSection::BOTTOM] = 28;
  hcal_n_layers_[ldmx::HcalID::HcalSection::LEFT] = 26;
  hcal_n_layers_[ldmx::HcalID::HcalSection::RIGHT] = 26;

  hcal_n_strips_[ldmx::HcalID::HcalSection::BACK] = 62;
  hcal_n_strips_[ldmx::HcalID::HcalSection::TOP] = 12;
  hcal_n_strips_[ldmx::HcalID::HcalSection::BOTTOM] = 12;
  hcal_n_strips_[ldmx::HcalID::HcalSection::LEFT] = 12;
  hcal_n_strips_[ldmx::HcalID::HcalSection::RIGHT] = 12;

  double ecal_z = 440.;
  double ecal_xy = 600.;
  double back_transverse_width = 3100.;
  double ecal_front_z = 220.;

  hcal_length_scint_[ldmx::HcalID::HcalSection::BACK] = back_transverse_width;
  hcal_length_scint_[ldmx::HcalID::HcalSection::TOP] =
      (back_transverse_width + ecal_xy) / 2.;
  hcal_length_scint_[ldmx::HcalID::HcalSection::BOTTOM] =
      (back_transverse_width + ecal_xy) / 2.;
  hcal_length_scint_[ldmx::HcalID::HcalSection::LEFT] =
      (back_transverse_width + ecal_xy) / 2.;
  hcal_length_scint_[ldmx::HcalID::HcalSection::RIGHT] =
      (back_transverse_width + ecal_xy) / 2.;

  hcal_zero_layer_[ldmx::HcalID::HcalSection::BACK] =
      ecal_front_z + 600.;  // leaving 60cm cube for ecal
  hcal_zero_layer_[ldmx::HcalID::HcalSection::TOP] = ecal_xy / 2.;
  hcal_zero_layer_[ldmx::HcalID::HcalSection::BOTTOM] = ecal_xy / 2.;
  hcal_zero_layer_[ldmx::HcalID::HcalSection::LEFT] = ecal_xy / 2.;
  hcal_zero_layer_[ldmx::HcalID::HcalSection::RIGHT] = ecal_xy / 2.;

  hcal_zero_strip_[ldmx::HcalID::HcalSection::BACK] =
      back_transverse_width / 2.;
  hcal_zero_strip_[ldmx::HcalID::HcalSection::TOP] = ecal_front_z;
  hcal_zero_strip_[ldmx::HcalID::HcalSection::BOTTOM] = ecal_front_z;
  hcal_zero_strip_[ldmx::HcalID::HcalSection::LEFT] = ecal_front_z;
  hcal_zero_strip_[ldmx::HcalID::HcalSection::RIGHT] = ecal_front_z;

  // absorber + scintillator + 2*air
  hcal_layer_thickness_[ldmx::HcalID::HcalSection::BACK] =
      25. + hcal_thickness_scint_ + 2 * 2.;
  hcal_layer_thickness_[ldmx::HcalID::HcalSection::TOP] =
      20. + hcal_thickness_scint_ + 2 * 2.;
  hcal_layer_thickness_[ldmx::HcalID::HcalSection::BOTTOM] =
      20. + hcal_thickness_scint_ + 2 * 2.;
  hcal_layer_thickness_[ldmx::HcalID::HcalSection::LEFT] =
      20. + hcal_thickness_scint_ + 2 * 2.;
  hcal_layer_thickness_[ldmx::HcalID::HcalSection::RIGHT] =
      20. + hcal_thickness_scint_ + 2 * 2.;

  ///////////////////////////////////////////////////////////////////////////////////
  // ECAL

  ecal_zero_layer_ = ecal_front_z;

  ecal_si_thickness_ = 0.5;

  ecal_depth_ = ecal_z;

  std::vector<double> ecalSiPlanes = {
      7.850,   13.300,  26.400,  33.500,  47.950,  56.550,  72.250,
      81.350,  97.050,  106.150, 121.850, 130.950, 146.650, 155.750,
      171.450, 180.550, 196.250, 205.350, 221.050, 230.150, 245.850,
      254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
      362.550, 375.150, 394.350, 406.950, 426.150, 438.750};

  std::map<std::string, std::any> hexReadoutParams;
  hexReadoutParams["gap"] = 1.5;
  hexReadoutParams["module_min_r"] = 85.0;
  hexReadoutParams["layerZPositions"] = ecalSiPlanes;
  hexReadoutParams["ecal_front_z"] = ecal_zero_layer_;
  hexReadoutParams["n_cell_r_height"] = 35.3;
  hexReadoutParams["verbose"] = 0;

  framework::config::Parameters hexReadout;
  hexReadout.setParameters(hexReadoutParams);
  ecal_hex_reader_ = std::unique_ptr<ldmx::EcalHexReadout>(
      ldmx::EcalHexReadout::debugMake(hexReadout));

  /////////////////////////////////////////////////////////////
  // RECOIL TRACKER
  //      The gdml file for the recoil tracker is kinda opaque.
  //      The layer and module IDs are calculated from the copy number of each
  //      of the sensor volumes.
  //          layer  = copyNum / 10 (integer division)
  //          module = copyNum % 10
  //      The first 8 layer IDs are the first 4 layers of stereo sensors.
  //          Each stereo layer contains a front layer that is not tilted at an
  //          angle and a back layer that is tilted.
  //      The last 2 layer IDs correspond to the 2 layers of mono sensors.
  //          Each mono layer contains 10 modules (ids 0 - 9) that have a
  //          complicated position arrangement.
  //
  //      In order to avoid mistakes, the position and angle of each module will
  //      be hard coded here instead of calculated from design specifications
  //      like the HCAL case.

  recoil_stereo_strip_length_ = 98.0;

  recoil_stereo_x_width_ = 40.34;

  recoil_stereo_y_width_ = 100.0;

  recoil_stereo_separation_ = 3.0;

  recoil_stereo_angle_ = 0.1;

  recoil_mono_strip_length_ = 78.0;

  recoil_mono_x_width_ = 50.0;

  recoil_mono_y_width_ = 80.0;

  recoil_mono_separation_ = 1.0;

  recoil_sensor_thickness_ = 0.52;

  // The following keys for the position and angle maps should correspond to the
  // copynumber in the recoil.gdml file At writing, the layerIDs and moduleIDs
  // are set in the simulation from this copy number (TrackerSD.cxx in SimCore)

  std::vector<double> recoilStereoLayerZPos = {7.5, 22.5, 37.5, 52.5};

  recoil_module_pos_[10] = {
      0, 0, recoilStereoLayerZPos.at(0) - recoil_stereo_separation_};
  recoil_module_pos_[20] = {
      0, 0, recoilStereoLayerZPos.at(0) + recoil_stereo_separation_};

  recoil_module_pos_[30] = {
      0, 0, recoilStereoLayerZPos.at(1) - recoil_stereo_separation_};
  recoil_module_pos_[40] = {
      0, 0, recoilStereoLayerZPos.at(1) + recoil_stereo_separation_};

  recoil_module_pos_[50] = {
      0, 0, recoilStereoLayerZPos.at(2) - recoil_stereo_separation_};
  recoil_module_pos_[60] = {
      0, 0, recoilStereoLayerZPos.at(2) + recoil_stereo_separation_};

  recoil_module_pos_[70] = {
      0, 0, recoilStereoLayerZPos.at(3) - recoil_stereo_separation_};
  recoil_module_pos_[80] = {
      0, 0, recoilStereoLayerZPos.at(3) + recoil_stereo_separation_};

  std::vector<double> recoilMonoLayerZPos = {90.0, 180.0};

  recoil_module_pos_[90] = {
      2 * recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) + recoil_mono_separation_};
  recoil_module_pos_[91] = {
      recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) - recoil_mono_separation_};
  recoil_module_pos_[92] = {
      0.0, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) + recoil_mono_separation_};
  recoil_module_pos_[93] = {
      -1 * recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) - recoil_mono_separation_};
  recoil_module_pos_[94] = {
      -2 * recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) + recoil_mono_separation_};
  recoil_module_pos_[95] = {
      2 * recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) + recoil_mono_separation_};
  recoil_module_pos_[96] = {
      recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) - recoil_mono_separation_};
  recoil_module_pos_[97] = {
      0.0, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) + recoil_mono_separation_};
  recoil_module_pos_[98] = {
      -1 * recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) - recoil_mono_separation_};
  recoil_module_pos_[99] = {
      -2 * recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(0) + recoil_mono_separation_};

  recoil_module_pos_[100] = {
      2 * recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) + recoil_mono_separation_};
  recoil_module_pos_[101] = {
      recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) - recoil_mono_separation_};
  recoil_module_pos_[102] = {
      0.0, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) + recoil_mono_separation_};
  recoil_module_pos_[103] = {
      -1 * recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) - recoil_mono_separation_};
  recoil_module_pos_[104] = {
      -2 * recoil_mono_x_width_, 0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) + recoil_mono_separation_};
  recoil_module_pos_[105] = {
      2 * recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) + recoil_mono_separation_};
  recoil_module_pos_[106] = {
      recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) - recoil_mono_separation_};
  recoil_module_pos_[107] = {
      0.0, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) + recoil_mono_separation_};
  recoil_module_pos_[108] = {
      -1 * recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) - recoil_mono_separation_};
  recoil_module_pos_[109] = {
      -2 * recoil_mono_x_width_, -0.5 * recoil_mono_y_width_,
      recoilMonoLayerZPos.at(1) + recoil_mono_separation_};

  // Recoil Angles
  recoil_module_angle_[10] = 0.0;
  recoil_module_angle_[20] = recoil_stereo_angle_;

  recoil_module_angle_[30] = 0.0;
  recoil_module_angle_[40] = -recoil_stereo_angle_;

  recoil_module_angle_[50] = 0.0;
  recoil_module_angle_[60] = recoil_stereo_angle_;

  recoil_module_angle_[70] = 0.0;
  recoil_module_angle_[80] = -recoil_stereo_angle_;

  recoil_module_angle_[90] = 0.0;
  recoil_module_angle_[91] = 0.0;
  recoil_module_angle_[92] = 0.0;
  recoil_module_angle_[93] = 0.0;
  recoil_module_angle_[94] = 0.0;
  recoil_module_angle_[95] = 0.0;
  recoil_module_angle_[96] = 0.0;
  recoil_module_angle_[97] = 0.0;
  recoil_module_angle_[98] = 0.0;
  recoil_module_angle_[99] = 0.0;

  recoil_module_angle_[100] = 0.0;
  recoil_module_angle_[101] = 0.0;
  recoil_module_angle_[102] = 0.0;
  recoil_module_angle_[103] = 0.0;
  recoil_module_angle_[104] = 0.0;
  recoil_module_angle_[105] = 0.0;
  recoil_module_angle_[106] = 0.0;
  recoil_module_angle_[107] = 0.0;
  recoil_module_angle_[108] = 0.0;
  recoil_module_angle_[109] = 0.0;

  // TODO Tagger
  // TODO Trigger Pad
}

BoundingBox DetectorGeometry::getBoundingBox(const ldmx::HcalHit &hit) const {
  // pairs that will go into BoundingBox
  std::pair<double, double> X(0, 0), Y(0, 0), Z(0, 0);

  ldmx::HcalID id(hit.getID());
  ldmx::HcalID::HcalSection section = (ldmx::HcalID::HcalSection)id.section();
  int layer = id.layer();
  int strip = id.strip();

  // calculate center of layer,strip with respect to detector section
  double layercenter =
      layer * hcal_layer_thickness_.at(section) + 0.5 * hcal_thickness_scint_;
  double stripcenter = (strip + 0.5) * hcal_width_scint_;

  // calculate error in layer,strip position
  double elayer = 0.5 * hcal_thickness_scint_;
  double estrip = 0.5 * hcal_width_scint_;

  double x, y, z;
  if (section == ldmx::HcalID::HcalSection::BACK) {
    z = hcal_zero_layer_.at(section) + layercenter;
    Z.first = z - elayer;
    Z.second = z + elayer;

    // only horizontal layers implemented currently
    if (false) {  //( (layer ^ hcal_parity_vertical_) & 1) == 0 ) { //checks for
                  // same parity
      // Vertical Layers

      x = -hcal_zero_strip_.at(section) + stripcenter;
      X.first = x - estrip;
      X.second = x + estrip;

      y = hit.getYPos();
      Y.first = y - hcal_uncertainty_timing_pos_;
      Y.second = y + hcal_uncertainty_timing_pos_;

    } else {
      // Horizontal Layers

      x = hit.getXPos();
      X.first = x - hcal_uncertainty_timing_pos_;
      X.second = x + hcal_uncertainty_timing_pos_;

      y = -1 * hcal_zero_strip_.at(section) + stripcenter;
      Y.first = y - estrip;
      Y.second = y + estrip;

    }  // calculate depending on layer

  } else {
    z = hcal_zero_strip_.at(section) + stripcenter;
    Z.first = z - estrip;
    Z.second = z + estrip;

    if (section == ldmx::HcalID::HcalSection::TOP or
        section == ldmx::HcalID::HcalSection::BOTTOM) {
      x = hit.getXPos();
      X.first = x - hcal_uncertainty_timing_pos_;
      X.second = x + hcal_uncertainty_timing_pos_;

      y = hcal_zero_layer_.at(section) + layercenter;
      if (section == ldmx::HcalID::HcalSection::BOTTOM) {
        y *= -1;
      }

      Y.first = y - elayer;
      Y.second = y + elayer;

    } else if (section == ldmx::HcalID::HcalSection::LEFT or
               section == ldmx::HcalID::HcalSection::RIGHT) {
      y = hit.getYPos();
      Y.first = y - hcal_uncertainty_timing_pos_;
      Y.second = y + hcal_uncertainty_timing_pos_;

      x = hcal_zero_layer_.at(section) + layercenter;
      if (section == ldmx::HcalID::HcalSection::RIGHT) {
        x *= -1;
      }

      X.first = x - elayer;
      X.second = x + elayer;

    } else {
      std::cerr
          << "[ DetectorGeometry::getBoundingBox ] : Unknown Hcal Section!"
          << std::endl;
      std::cerr << "    Returning a valid BoundingBox but with values that are "
                   "all zero."
                << std::endl;
    }  // side hcal

  }  // calculate depending on section

  BoundingBox hbox;
  hbox.push_back(X);
  hbox.push_back(Y);
  hbox.push_back(Z);
  return hbox;
}

BoundingBox DetectorGeometry::getBoundingBox(
    const std::vector<ldmx::HcalHit> &hitVec) const {
  std::vector<double> pointSum(3, 0.0);   // sums of weighted coordinates
  std::vector<double> weightSum(3, 0.0);  // sums of weights for each coordinate

  // calculate real space point for each hit
  for (const ldmx::HcalHit &hit : hitVec) {
    BoundingBox box = getBoundingBox(hit);

    // Add weighted values to sums
    double weight;
    for (unsigned int iC = 0; iC < 3; iC++) {
      double cer = abs(box[iC].second - box[iC].first) / 2.0;

      weight = 1.0 / (cer * cer);
      weightSum[iC] += weight;
      pointSum[iC] += weight * ((box[iC].second + box[iC].first) / 2.0);
    }
  }  // go through hitVec

  // Construct final BoundingBox
  BoundingBox hbox;
  for (int iC = 0; iC < 3; iC++) {
    double c = pointSum[iC] / weightSum[iC];
    double ec = 1.0 / sqrt(weightSum[iC]);
    hbox.emplace_back(c - ec, c + ec);
  }

  return hbox;
}

BoundingBox DetectorGeometry::getBoundingBox(
    ldmx::HcalID::HcalSection section) const {
  std::pair<double, double> X(0, 0), Y(0, 0), Z(0, 0);

  double total_strip_width = hcal_n_strips_.at(section) * hcal_width_scint_;
  double total_thickness =
      hcal_n_layers_.at(section) * hcal_layer_thickness_.at(section);
  if (section == ldmx::HcalID::HcalSection::BACK) {
    X.first = -hcal_zero_strip_.at(ldmx::HcalID::HcalSection::BACK);
    X.second = X.first + total_strip_width;

    Y.first = -hcal_length_scint_.at(ldmx::HcalID::HcalSection::BACK) / 2.0;
    Y.second = hcal_length_scint_.at(ldmx::HcalID::HcalSection::BACK) / 2.0;

    Z.first = hcal_zero_layer_.at(ldmx::HcalID::HcalSection::BACK);
    Z.second = Z.first + total_thickness;

  } else {
    Z.first = hcal_zero_strip_.at(section);
    Z.second = Z.first + total_strip_width;

    if (section == ldmx::HcalID::HcalSection::LEFT) {
      X.first = hcal_zero_layer_.at(ldmx::HcalID::HcalSection::LEFT);
      X.second = X.first + total_thickness;

      Y.second = hcal_zero_layer_.at(ldmx::HcalID::HcalSection::TOP);
      Y.first =
          Y.second - hcal_length_scint_.at(ldmx::HcalID::HcalSection::LEFT);

    } else if (section == ldmx::HcalID::HcalSection::RIGHT) {
      X.second = -hcal_zero_layer_.at(ldmx::HcalID::HcalSection::RIGHT);
      X.first = X.second - total_thickness;

      Y.first = -hcal_zero_layer_.at(ldmx::HcalID::HcalSection::BOTTOM);
      Y.second =
          Y.first + hcal_length_scint_.at(ldmx::HcalID::HcalSection::RIGHT);

    } else if (section == ldmx::HcalID::HcalSection::TOP) {
      Y.first = hcal_zero_layer_.at(ldmx::HcalID::HcalSection::TOP);
      Y.second = Y.first + total_thickness;

      X.first = -hcal_zero_layer_.at(ldmx::HcalID::HcalSection::RIGHT);
      X.second =
          X.first + hcal_length_scint_.at(ldmx::HcalID::HcalSection::TOP);

    } else if (section == ldmx::HcalID::HcalSection::BOTTOM) {
      Y.second = -hcal_zero_layer_.at(ldmx::HcalID::HcalSection::BOTTOM);
      Y.first = Y.second - total_thickness;

      X.second = hcal_zero_layer_.at(ldmx::HcalID::HcalSection::LEFT);
      X.first =
          X.second - hcal_length_scint_.at(ldmx::HcalID::HcalSection::BOTTOM);

    } else {
      std::cerr << "[ Warning ] : Unrecognized ldmx::HcalID::HcalSection in "
                   "DetectorGeometry::getBoundingBox."
                << std::endl;
      std::cerr << "    Will return an incorrect geometry description!"
                << std::endl;
    }
  }

  BoundingBox boundingbox;
  boundingbox.push_back(X);
  boundingbox.push_back(Y);
  boundingbox.push_back(Z);

  return boundingbox;
}

HexPrism DetectorGeometry::getHexPrism(const ldmx::EcalID &id) const {
  HexPrism hexpris;
  ecal_hex_reader_->getCellAbsolutePosition(id, hexpris.x, hexpris.y,
                                            hexpris.z);
  hexpris.height = ecal_si_thickness_;
  hexpris.radius = ecal_hex_reader_->getCellMaxR();

  return hexpris;
}

HexPrism DetectorGeometry::getHexTower(int towerIndex) const {
  HexPrism hexpris;

  if (towerIndex < 0 or towerIndex > 6) {
    std::cerr << "[ Warning ] : towerIndex " << towerIndex << " out of bounds!"
              << std::endl;
    std::cerr << "    Will return a malformed HexPrism." << std::endl;
    return hexpris;
  }

  hexpris.x = ecal_hex_reader_->getModuleCenter(towerIndex).first;
  hexpris.y = ecal_hex_reader_->getModuleCenter(towerIndex).second;
  hexpris.z = ecal_zero_layer_ + ecal_depth_ / 2;
  hexpris.height = ecal_depth_;
  hexpris.radius = ecal_hex_reader_->getModuleMaxR();

  return hexpris;
}

double DetectorGeometry::getRotAngle(int layerID, int moduleID) const {
  int combined = layerID * 10 + moduleID;

  if (recoil_module_angle_.find(combined) == recoil_module_angle_.end()) {
    std::cerr << "[ Warning ] : DetectorGeometry::getRotAngle : Input layerID ("
              << layerID << ") and input moduleID (" << moduleID
              << ") are not included in the geometry!" << std::endl;
    return 0.0;
  }

  return recoil_module_angle_.at(combined);
}

BoundingBox DetectorGeometry::getBoundingBox(int layerID, int moduleID) const {
  int combined = layerID * 10 + moduleID;

  BoundingBox bbox;
  if (recoil_module_pos_.find(combined) == recoil_module_pos_.end()) {
    std::cerr
        << "[ Warning ] : DetectorGeometry::getBoundingBox : Input layerID ("
        << layerID << ") and input moduleID (" << moduleID
        << ") are not included in the geometry!" << std::endl;
    return bbox;
  }

  double xWidth = recoil_stereo_x_width_;
  double yWidth = recoil_stereo_strip_length_;
  if (layerID > 8) {
    xWidth = recoil_mono_x_width_;
    yWidth = recoil_mono_strip_length_;
  }

  bbox.emplace_back(recoil_module_pos_.at(combined).at(0) - xWidth / 2.,
                    recoil_module_pos_.at(combined).at(0) + xWidth / 2.);
  bbox.emplace_back(recoil_module_pos_.at(combined).at(1) - yWidth / 2.,
                    recoil_module_pos_.at(combined).at(1) + yWidth / 2.);
  bbox.emplace_back(
      recoil_module_pos_.at(combined).at(2) - recoil_sensor_thickness_ / 2.,
      recoil_module_pos_.at(combined).at(2) + recoil_sensor_thickness_ / 2.);

  return bbox;
}

BoundingBox DetectorGeometry::getBoundingBox(
    const ldmx::SimTrackerHit &recoilHit) const {
  int layerID = recoilHit.getLayerID();
  int moduleID = recoilHit.getModuleID();
  int combined = layerID * 10 + moduleID;

  BoundingBox bbox;
  if (recoil_module_pos_.find(combined) == recoil_module_pos_.end()) {
    std::cerr
        << "[ Warning ] : DetectorGeometry::getBoundingBox : Input layerID ("
        << layerID << ") and input moduleID (" << moduleID
        << ") are not included in the geometry!" << std::endl;
    return bbox;
  }

  std::vector<float> hitPos = recoilHit.getPosition();

  double xWidth = 1.0;
  double yWidth = recoil_stereo_strip_length_;
  if (layerID > 8) {
    yWidth = recoil_mono_strip_length_;
  }

  // we have to un-rotate the x-position of the hit, so we can rotate it later
  // with the drawer
  double rotAngle = this->getRotAngle(layerID, moduleID);
  double xPos = hitPos.at(0) * cos(-rotAngle) - hitPos.at(1) * sin(-rotAngle);

  bbox.emplace_back(xPos - xWidth / 2., xPos + xWidth / 2.);
  bbox.emplace_back(recoil_module_pos_.at(combined).at(1) - yWidth / 2.,
                    recoil_module_pos_.at(combined).at(1) + yWidth / 2.);
  bbox.emplace_back(
      recoil_module_pos_.at(combined).at(2) - recoil_sensor_thickness_ / 2.,
      recoil_module_pos_.at(combined).at(2) + recoil_sensor_thickness_ / 2.);

  return bbox;
}
}  // namespace eventdisplay
