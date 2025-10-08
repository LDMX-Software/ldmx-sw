#include "Tracking/geo/TrackersTrackingGeometry.h"

namespace tracking::geo {

const std::string TrackersTrackingGeometry::NAME = "TrackersTrackingGeometry";

TrackersTrackingGeometry::TrackersTrackingGeometry(
    const Acts::GeometryContext& gctx, const std::string& gdml,
    double tracker_y_length, double tracker_z_length)
    : TrackingGeometry(NAME, gctx, gdml) {
  tagger_ = findDaughterByName(f_world_phys_vol_, "tagger_PV");
  buildTaggerLayoutMap(tagger_, "tagger");
  Acts::CuboidVolumeBuilder::VolumeConfig tagger_volume_cfg = buildVolumeConfig(
      tagger_, tagger_layout_, tracker_y_length, tracker_z_length, "Tagger");

  recoil_ = findDaughterByName(f_world_phys_vol_, "recoil_PV");
  buildRecoilLayoutMap(recoil_, "recoil");
  Acts::CuboidVolumeBuilder::VolumeConfig recoil_volume_cfg = buildVolumeConfig(
      recoil_, recoil_layout_, tracker_y_length, tracker_z_length, "Recoil");

  std::vector<Acts::CuboidVolumeBuilder::VolumeConfig> vol_builder_configs{
      tagger_volume_cfg, recoil_volume_cfg};

  // Create the builder
  Acts::CuboidVolumeBuilder cvb;

  Acts::CuboidVolumeBuilder::Config config;
  config.position = {-200, 0., 0.};
  // acts x = global z
  // global z:  -200 - 900/2 = -650 to -200 + 900/2 = 250 mm
  // global y: -70 to 70
  // global x: -240 to 240
  config.length = {900, tracker_y_length, tracker_z_length};
  config.volumeCfg = vol_builder_configs;

  cvb.setConfig(config);

  Acts::TrackingGeometryBuilder::Config tgb_cfg;
  tgb_cfg.trackingVolumeBuilders.push_back(
      [=](const auto& cxt, const auto& inner, const auto&) {
        return cvb.trackingVolume(cxt, inner, nullptr);
      });

  Acts::TrackingGeometryBuilder tgb(tgb_cfg);
  t_geometry_ = tgb.trackingGeometry(gctx_);

  // dumpGeometry("./");
  makeLayerSurfacesMap();
}

void TrackersTrackingGeometry::buildRecoilLayoutMap(G4VPhysicalVolume* pvol,
                                                    std::string surfacename) {
  ldmx_log(trace) << "Building layout for the " << pvol->GetName()
                  << " tracker";
  getAllDaughters(pvol);

  // Get the global transform
  Acts::Transform3 tracker_transform = getTransform(*pvol);

  G4LogicalVolume* l_vol = pvol->GetLogicalVolume();
  for (G4int i = 0; i < l_vol->GetNoDaughters(); i++) {
    std::string sln = l_vol->GetDaughter(i)->GetName();
    if (sln.find(surfacename) != std::string::npos) {
      G4VPhysicalVolume* component0_volume{nullptr};
      G4VPhysicalVolume* active_sensor{nullptr};
      Acts::Transform3 ref1_transform = getTransform(*(l_vol->GetDaughter(i)));
      int sensor_copy_nr = -999;

      // recoil_l1(4)_axial(stereo)->LDMXRecoilL14ModuleVolume_component0_physvol
      // This works for v12
      if (sln.find("axial") != std::string::npos ||
          sln.find("stereo") != std::string::npos) {
        component0_volume =
            findDaughterByName(l_vol->GetDaughter(i),
                               "LDMXRecoilL14ModuleVolume_component0_physvol");
        if (!component0_volume)
          throw std::runtime_error(
              "Could not find component0 volume for L14 Recoil");
        active_sensor = findDaughterByName(
            component0_volume,
            "LDMXRecoilL14ModuleVolume_component0Sensor0_physvol");

      }

      // recoil_l5_sensorX->LDMXRecoilL56ModuleVolume_component0_physvol
      // This works for v12
      else if (sln.find("l5_sensor") != std::string::npos ||
               sln.find("l6_sensor") != std::string::npos) {
        component0_volume =
            findDaughterByName(l_vol->GetDaughter(i),
                               "LDMXRecoilL56ModuleVolume_component0_physvol");
        if (!component0_volume)
          throw std::runtime_error(
              "Could not find component0 volume for L56 Recoil");
        active_sensor = findDaughterByName(
            component0_volume,
            "LDMXRecoilL56ModuleVolume_component0Sensor0_physvol");
      }

      // recoil_PV tracker->recoil_l14_sensor_vol_PV->recoil_l14_active_sensor
      // This works for v14
      else if (sln.find("sensor_vol")) {
        active_sensor =
            findDaughterByName(l_vol->GetDaughter(i), "active_sensor");
        sensor_copy_nr = l_vol->GetDaughter(i)->GetCopyNo();
      }

      else
        throw std::runtime_error("Could not build recoil layout");

      if (!active_sensor)
        throw std::runtime_error(
            "Could not find ActiveSensor for recoil volume");

      Acts::Transform3 ref2_transform = Acts::Transform3::Identity();

      if (component0_volume)
        ref2_transform = getTransform(*(component0_volume));

      std::shared_ptr<Acts::PlaneSurface> sensor_surface = getSurfacePtr(
          active_sensor, tracker_transform * ref1_transform * ref2_transform);

      // Build the layout
      if (sln == "recoil_l1_axial" || sln == "recoil_l1_stereo" ||
          sensor_copy_nr == 10 || sensor_copy_nr == 20)
        recoil_layout_["recoil_tracker_L1"].push_back(sensor_surface);

      if (sln == "recoil_l2_axial" || sln == "recoil_l2_stereo" ||
          sensor_copy_nr == 30 || sensor_copy_nr == 40)
        recoil_layout_["recoil_tracker_L2"].push_back(sensor_surface);

      if (sln == "recoil_l3_axial" || sln == "recoil_l3_stereo" ||
          sensor_copy_nr == 50 || sensor_copy_nr == 60)
        recoil_layout_["recoil_tracker_L3"].push_back(sensor_surface);

      if (sln == "recoil_l4_axial" || sln == "recoil_l4_stereo" ||
          sensor_copy_nr == 70 || sensor_copy_nr == 80)
        recoil_layout_["recoil_tracker_L4"].push_back(sensor_surface);

      if (sln == "recoil_l5_sensor1" || sln == "recoil_l5_sensor2" ||
          sln == "recoil_l5_sensor3" || sln == "recoil_l5_sensor4" ||
          sln == "recoil_l5_sensor5" || sln == "recoil_l5_sensor6" ||
          sln == "recoil_l5_sensor7" || sln == "recoil_l5_sensor8" ||
          sln == "recoil_l5_sensor9" || sln == "recoil_l5_sensor10" ||
          (sensor_copy_nr >= 90 && sensor_copy_nr <= 99))

        recoil_layout_["recoil_tracker_L5"].push_back(sensor_surface);

      if (sln == "recoil_l6_sensor1" || sln == "recoil_l6_sensor2" ||
          sln == "recoil_l6_sensor3" || sln == "recoil_l6_sensor4" ||
          sln == "recoil_l6_sensor5" || sln == "recoil_l6_sensor6" ||
          sln == "recoil_l6_sensor7" || sln == "recoil_l6_sensor8" ||
          sln == "recoil_l6_sensor9" || sln == "recoil_l6_sensor10" ||
          (sensor_copy_nr >= 100 && sensor_copy_nr <= 109))
        recoil_layout_["recoil_tracker_L6"].push_back(sensor_surface);

    }  // found the daughter
  }  // loop on daughters
}  // BuildRecoilLayoutMap

// This function gets the surfaces from the trackers and orders them in
// ascending z_.

void TrackersTrackingGeometry::buildTaggerLayoutMap(G4VPhysicalVolume* pvol,
                                                    std::string surfacename) {
  ldmx_log(trace) << "Building layout for the " << pvol->GetName()
                  << " tracker";
  // getAllDaughters(pvol);

  // Get the global transform
  Acts::Transform3 tracker_transform = getTransform(*pvol);

  G4LogicalVolume* l_vol = pvol->GetLogicalVolume();
  for (G4int i = 0; i < l_vol->GetNoDaughters(); i++) {
    std::string sln = l_vol->GetDaughter(i)->GetName();

    // To distinguish which layers need to be selected
    if (sln.find(surfacename) != std::string::npos) {
      // v12

      // Box for the module_ (slightly bigger than the sensor)
      // LDMXTaggerModuleVolume_physvol -> LDMXTaggerModuleVolume_component0Box,
      // this is the sensor + inactive region
      // LDMXTaggerModuleVolume_component0Box->
      // LDMXTaggerModuleVolume_component0Sensor0Box, this is the sensor itself
      // (active region)

      // tagger_ -> LDMXTaggerModuleVolume_physvol1 ->
      // LDMXTaggerModuleVolume_component0_physvol
      // ->LDMXTaggerModuleVolume_component0Sensor0_physvol
      //         -> Get Box for the dimension: GetLogical->GetSolid
      //         For the positioning of the sensor:
      //   A     position of physvol1 + position of the component0_physvol +
      //   position of the sensor physvol. Thickness I can grab it from the box,
      //   or I just hardcode it. S     transform1 * transform2 * transform3
      //   ....

      // v14
      // tagger_PV->tagger_sensor_vol_PV->tagger_active_sensor
      // Then use the copyNumber..

      // Get the sensor volume placement
      Acts::Transform3 ref1_transform = getTransform(*(l_vol->GetDaughter(i)));

      G4VPhysicalVolume* component0_volume = findDaughterByName(
          l_vol->GetDaughter(i), "LDMXTaggerModuleVolume_component0_physvol");

      Acts::Transform3 ref2_transform = Acts::Transform3::Identity();
      G4VPhysicalVolume* active_sensor = nullptr;
      int sensor_copy_nr = -999;

      // Get Component0 transform. v12
      if (component0_volume) {
        ref2_transform = getTransform(*(component0_volume));
        active_sensor = findDaughterByName(
            component0_volume,
            "LDMXTaggerModuleVolume_component0Sensor0_physvol");
      }
      // v14
      else {
        active_sensor =
            findDaughterByName(l_vol->GetDaughter(i), "active_sensor");
        sensor_copy_nr = (l_vol->GetDaughter(i))->GetCopyNo();
      }

      if (!active_sensor) {
        ldmx_log(fatal) << "Could not find the ActiveSensor for tagger volume "
                        << l_vol->GetDaughter(i)->GetName();
      }

      // Get the surface
      std::shared_ptr<Acts::PlaneSurface> sensor_surface = getSurfacePtr(
          active_sensor, tracker_transform * ref1_transform * ref2_transform);

      if (sln == "LDMXTaggerModuleVolume_physvol1" ||
          sln == "LDMXTaggerModuleVolume_physvol2" || sensor_copy_nr == 130 ||
          sensor_copy_nr == 140)
        tagger_layout_["tagger_tracker_L1"].push_back(sensor_surface);

      if (sln == "LDMXTaggerModuleVolume_physvol3" ||
          sln == "LDMXTaggerModuleVolume_physvol4" || sensor_copy_nr == 110 ||
          sensor_copy_nr == 120)
        tagger_layout_["tagger_tracker_L2"].push_back(sensor_surface);

      if (sln == "LDMXTaggerModuleVolume_physvol5" ||
          sln == "LDMXTaggerModuleVolume_physvol6" || sensor_copy_nr == 90 ||
          sensor_copy_nr == 100)
        tagger_layout_["tagger_tracker_L3"].push_back(sensor_surface);

      if (sln == "LDMXTaggerModuleVolume_physvol7" ||
          sln == "LDMXTaggerModuleVolume_physvol8" || sensor_copy_nr == 70 ||
          sensor_copy_nr == 80)
        tagger_layout_["tagger_tracker_L4"].push_back(sensor_surface);

      if (sln == "LDMXTaggerModuleVolume_physvol9" ||
          sln == "LDMXTaggerModuleVolume_physvol10" || sensor_copy_nr == 50 ||
          sensor_copy_nr == 60)
        tagger_layout_["tagger_tracker_L5"].push_back(sensor_surface);

      if (sln == "LDMXTaggerModuleVolume_physvol11" ||
          sln == "LDMXTaggerModuleVolume_physvol12" || sensor_copy_nr == 30 ||
          sensor_copy_nr == 40)
        tagger_layout_["tagger_tracker_L6"].push_back(sensor_surface);

      if (sln == "LDMXTaggerModuleVolume_physvol13" ||
          sln == "LDMXTaggerModuleVolume_physvol14" || sensor_copy_nr == 10 ||
          sensor_copy_nr == 20)
        tagger_layout_["tagger_tracker_L7"].push_back(sensor_surface);

    }  // found a silicon surface
  }  // loop on daughters
}  // build the layout

std::shared_ptr<Acts::PlaneSurface> TrackersTrackingGeometry::getSurfacePtr(
    G4VPhysicalVolume* pvol, Acts::Transform3 ref_trans) {
  if (!pvol) {
    ldmx_log(fatal) << "pvol is nullptr";
  }

  // Get the surface transform
  Acts::Transform3 surface_transform = getTransform(*pvol);
  // Compose the sensor_transform with the reference transform
  surface_transform = ref_trans * surface_transform;

  // Now transform to the tracker frame
  Acts::Transform3 surface_transform_tracker = toTracker(surface_transform);

  ldmx_log(trace) << "THE SENSOR TRANSFORM - TRANSLATION";
  ldmx_log(trace) << surface_transform.translation()(0);
  ldmx_log(trace) << surface_transform.translation()(1);
  ldmx_log(trace) << surface_transform.translation()(2);
  ldmx_log(trace) << "THE SENSOR TRANSFORM - ROTATION";
  ldmx_log(trace) << surface_transform.rotation();

  ldmx_log(trace) << "TO THE TRACKER FRAME";
  ldmx_log(trace) << surface_transform_tracker.translation()(0);
  ldmx_log(trace) << surface_transform_tracker.translation()(1);
  ldmx_log(trace) << surface_transform_tracker.translation()(2);
  ldmx_log(trace) << "THE SENSOR TRANSFORM - ROTATION";
  ldmx_log(trace) << surface_transform_tracker.rotation();

  // This material is defined in different units with respect what acts expects.
  // I decided to hardcode here. TODO: fix this

  /*
    G4Material* sens_mat = _ActiveSensor->GetLogicalVolume()->GetMaterial();
    ldmx_log(trace)<<"Checking the material of
    "<<l_vol->GetDaughter(i)->GetName()<<std::endl; ldmx_log(trace)<<"With
    sensor::"<<_ActiveSensor->GetName()<<std::endl;
    ldmx_log(trace)<<sens_mat->GetName()<<std::endl;
    ldmx_log(trace)<<"RL="<<sens_mat->GetRadlen()<<"
    lambda="<<sens_mat->GetNuclearInterLength()<<std::endl;
    ldmx_log(trace)<<"A="<<sens_mat->GetA()<<" Z="<<sens_mat->GetZ()<<"
    rho="<<sens_mat->GetDensity()<<std::endl;


    Acts::Material silicon =
    Acts::Material::fromMassDensity(sens_mat->GetRadlen(),
    sens_mat->GetNuclearInterLength(),
    sens_mat->GetA(),
    sens_mat->GetZ(),
    sens_mat->GetDensity());
  */

  // Define the silicon material
  Acts::Material silicon = Acts::Material::fromMassDensity(
      95.7 * Acts::UnitConstants::mm, 465.2 * Acts::UnitConstants::mm, 28.03,
      14., 2.32 * Acts::UnitConstants::g / Acts::UnitConstants::cm3);

  // Get the active sensor box
  G4Box* surface_solid = (G4Box*)(pvol->GetLogicalVolume()->GetSolid());

  ldmx_log(trace) << "Sensor Dimensions";
  ldmx_log(trace) << surface_solid->GetXHalfLength() << " "
                  << surface_solid->GetYHalfLength() << " "
                  << surface_solid->GetZHalfLength() << " ";

  // Form the material slab
  double thickness =
      2 * surface_solid->GetZHalfLength() * Acts::UnitConstants::mm;
  Acts::MaterialSlab silicon_slab(silicon, thickness);

  // Get the bounds
  std::shared_ptr<const Acts::RectangleBounds> rect_bounds =
      std::make_shared<const Acts::RectangleBounds>(Acts::RectangleBounds(
          surface_solid->GetXHalfLength() * Acts::UnitConstants::mm,
          surface_solid->GetYHalfLength() * Acts::UnitConstants::mm));

  // Form the active sensor surface
  std::shared_ptr<Acts::PlaneSurface> surface =
      Acts::Surface::makeShared<Acts::PlaneSurface>(surface_transform_tracker,
                                                    rect_bounds);
  surface->assignSurfaceMaterial(
      std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab));

  // Create an alignable detector element and assign it to the surface.
  // The default transformation is the surface parsed transformation

  auto det_element = std::make_shared<DetectorElement>(
      surface, surface_transform_tracker, thickness);

  // This is the call that modify the behaviour of surface->transform(gctx)
  // After this call each surface will use the underlying detectorElement
  // transformation which will take care of effectively reading the gctx

  surface->assignDetectorElement(std::move(*det_element));
  det_elements_.push_back(det_element);

  return surface;
}

Acts::CuboidVolumeBuilder::VolumeConfig
TrackersTrackingGeometry::buildVolumeConfig(
    const G4VPhysicalVolume* detector,
    const std::map<std::string,
                   std::vector<std::shared_ptr<const Acts::Surface>>>
        layout,
    double tracker_y_length, double tracker_z_length,
    const std::string& volumeName) {
  Acts::CuboidVolumeBuilder::VolumeConfig sub_det_volume_config;

  // Get the transform wrt the world volume in tracker frame
  Acts::Transform3 sub_det_transform = getTransform(*detector, true);

  // Add 1mm to not make it sit on the first layer_ surface
  Acts::Vector3 sub_det_position = {
      sub_det_transform.translation()(0) - 1,
      sub_det_transform.translation()(1),
      sub_det_transform.translation()(2),
  };

  ldmx_log(trace) << sub_det_position;
  // Get the size of the volume
  G4Box* sub_det_box = (G4Box*)(detector->GetLogicalVolume()->GetSolid());

  // In tracker coordinates. Add 1mm to compensate for the movement above
  double x_length =
      2 * (sub_det_box->GetZHalfLength() + 1) * Acts::UnitConstants::mm;
  ldmx_log(info) << "x_length = " << x_length
                 << " y_length = " << tracker_y_length
                 << " z_length = " << tracker_z_length;

  sub_det_volume_config.position = sub_det_position;
  sub_det_volume_config.length = {x_length, tracker_y_length, tracker_z_length};
  sub_det_volume_config.name = volumeName;

  // Vacuum material
  Acts::Material subdet_mat = Acts::Material();
  sub_det_volume_config.volumeMaterial =
      std::make_shared<Acts::HomogeneousVolumeMaterial>(subdet_mat);

  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layer_config;

  // Prepare the layers
  for (auto& layer : layout) {
    ldmx_log(trace) << layer.first << " : surfaces==>" << layer.second.size();

    Acts::CuboidVolumeBuilder::LayerConfig lcfg;
    lcfg.surfaces = layer.second;

    // Get the surface thickness
    double clearance = 0.01;
    double thickness = layer.second.front()
                           ->surfaceMaterial()
                           ->materialSlab(Acts::Vector2{0., 0.})
                           .thickness();

    lcfg.envelopeX = std::array<double, 2>{thickness / 2. + clearance,
                                           thickness / 2. + clearance};
    lcfg.active = true;
    layer_config.push_back(lcfg);
  }

  sub_det_volume_config.layerCfg = layer_config;

  return sub_det_volume_config;
}

}  // namespace tracking::geo
