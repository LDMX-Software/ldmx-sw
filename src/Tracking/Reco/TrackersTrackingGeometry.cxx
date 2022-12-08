#include "Tracking/Reco/TrackersTrackingGeometry.h"

namespace tracking {
namespace reco {


TrackersTrackingGeometry::TrackersTrackingGeometry(std::string gdmlfile,
                         Acts::GeometryContext* ctx, bool debug) :
    BaseTrackingGeometry(gdmlfile, ctx, debug) {

  if (debug_)
    std::cout<<"Looking for Tagger and Recoil volumes"<<std::endl;

  std::vector<Acts::CuboidVolumeBuilder::VolumeConfig> volBuilderConfigs;

  Tagger_ = findDaughterByName(fWorldPhysVol_, "tagger_PV");

  //Get the layout layers-sensors and accumulate all the sensitive surfaces
  BuildTaggerLayoutMap(Tagger_,"LDMXTaggerModuleVolume_physvol");

  //Prepare the volume configuration for the tagger tracker
  Acts::CuboidVolumeBuilder::VolumeConfig tagger_volume_cfg = buildTrackerVolume();
  
  
  Recoil_ = findDaughterByName(fWorldPhysVol_, "recoil_PV");
  //  BuildLayoutMap(Recoil_,"recoil");
  

  volBuilderConfigs.push_back(tagger_volume_cfg);
  
  //Create the builder
  Acts::CuboidVolumeBuilder cvb;
  
  Acts::CuboidVolumeBuilder::Config config;
  config.position = {-200,0.,0.};
  config.length   = {900,480,240};
  config.volumeCfg = volBuilderConfigs;

  cvb.setConfig(config);

  Acts::TrackingGeometryBuilder::Config tgbCfg;
  tgbCfg.trackingVolumeBuilders.push_back(
      [=](const auto& cxt, const auto& inner, const auto&) {
        return cvb.trackingVolume(cxt, inner, nullptr);
      });
  
  Acts::TrackingGeometryBuilder tgb(tgbCfg);
  tGeometry_ = tgb.trackingGeometry(*gctx_);

  dumpGeometry("./");
        
}


Acts::CuboidVolumeBuilder::VolumeConfig TrackersTrackingGeometry::buildTrackerVolume() {
  
  Acts::CuboidVolumeBuilder::VolumeConfig subDetVolumeConfig;

  //Get the transform wrt the world volume in tracker frame
  Acts::Transform3 subDet_transform = GetTransform(*Tagger_,true);

  if (debug_) {
    std::cout<<subDet_transform.translation()<<std::endl;
    std::cout<<subDet_transform.rotation()<<std::endl;
  }

  //Add 1mm to not make it sit on the first layer surface  -  Ask Omar if it's OK
  Acts::Vector3 sub_det_position = {subDet_transform.translation()(0) - 1,
    subDet_transform.translation()(1),
    subDet_transform.translation()(2),
  };
  
  //Get the size of the volume
  G4Box* subDetBox = (G4Box*)(Tagger_->GetLogicalVolume()->GetSolid());
  
  //In tracker coordinates. I add 1mm so that it compensates with the 1mm movement of above 
  double x_length  = 2*(subDetBox->GetZHalfLength()+1 ) * Acts::UnitConstants::mm;
  
  //double y_length  = 2*subDetBox->GetXHalfLength() * Acts::UnitConstants::mm;
  //double z_length  = 2*subDetBox->GetYHalfLength() * Acts::UnitConstants::mm;

  //The bField is defined only between -70, 70 along ACTS Z Axis (-130,130 in the extended version)
  double y_length = 480;
  double z_length = 240;


  if (debug_) {
    std::cout<<Tagger_->GetName()<<std::endl;
    std::cout<<"position"<<std::endl;
    std::cout<<sub_det_position<<std::endl;
    std::cout<<"x_length "<<x_length<<" y_length "<<y_length<<" z_length "<<z_length<<std::endl;
  }

  subDetVolumeConfig.position = sub_det_position;
  subDetVolumeConfig.length = {x_length, y_length, z_length};
  subDetVolumeConfig.name = "Tagger";

  //Vacuum material
  Acts::Material subdet_mat = Acts::Material();
  subDetVolumeConfig.volumeMaterial =
      std::make_shared<Acts::HomogeneousVolumeMaterial>(subdet_mat);
  
  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layerConfig;
  
  //Prepare the layers
  for (auto& layer : tagger_layout) {

    if (debug_) {
      std::cout<< layer.first
               <<" : surfaces==>"
               << layer.second.size()
               << std::endl;
    }
    
    Acts::CuboidVolumeBuilder::LayerConfig lcfg;
    lcfg.surfaces = layer.second;
    //Get the surface thickness
    double clearance = 0.01;
    double thickness = layer.second.front()->surfaceMaterial()->materialSlab(Acts::Vector2{0.,0.}).thickness();

    //std::cout<<"Sensor Thickness from Material slab "<< thickness<<std::endl;
    
    lcfg.envelopeX = std::pair<double,double>{thickness / 2. + clearance,
      thickness / 2. + clearance};
    lcfg.active = true;
    layerConfig.push_back(lcfg);
  }

  subDetVolumeConfig.layerCfg = layerConfig;
  
  
  return subDetVolumeConfig;
  
}

//This function gets the surfaces from the trackers and orders them in ascending z.

void TrackersTrackingGeometry::BuildTaggerLayoutMap(G4VPhysicalVolume* pvol,
                                                    std::string surfacename) {
  
  if (debug_)
    std::cout<<"Building layout for the "<<pvol->GetName()<<" tracker"<<std::endl;
  
  //Get the global transform
  Acts::Transform3 tracker_transform = GetTransform(*pvol);
  
    G4LogicalVolume* l_vol = pvol -> GetLogicalVolume();
    for (G4int i=0; i<l_vol->GetNoDaughters(); i++) {
      std::string sln = l_vol->GetDaughter(i)->GetName();

      //To distinguish which layers need to be selected 
      if (sln.find(surfacename) != std::string::npos) {
        
        // Box for the module (slightly bigger than the sensor)
        // LDMXTaggerModuleVolume_physvol -> LDMXTaggerModuleVolume_component0Box, this is the sensor + inactive region
        // LDMXTaggerModuleVolume_component0Box-> LDMXTaggerModuleVolume_component0Sensor0Box, this is the sensor itself (active region)
        
        //Tagger_ -> LDMXTaggerModuleVolume_physvol1 -> LDMXTaggerModuleVolume_component0_physvol ->LDMXTaggerModuleVolume_component0Sensor0_physvol
        //        -> Get Box for the dimension: GetLogical->GetSolid
        //        For the positioning of the sensor:
        //  A     position of physvol1 + position of the component0_physvol + position of the sensor physvol. Thickness I can grab it from the box, or I just hardcode it.
        //  S     transform1 * transform2 * transform3 ....


        //Get the sensor volume placement
        Acts::Transform3 ref1_transform = GetTransform(*(l_vol->GetDaughter(i)));
        
        G4VPhysicalVolume* _Component0Volume = findDaughterByName(l_vol->GetDaughter(i),"LDMXTaggerModuleVolume_component0_physvol");
        if (!_Component0Volume)
          throw std::runtime_error("Missing component0_physvol from physvol");
        //Get Component0 transform
        Acts::Transform3 ref2_transform = GetTransform(*(_Component0Volume));
        
        G4VPhysicalVolume* _ActiveSensor = findDaughterByName(_Component0Volume,"LDMXTaggerModuleVolume_component0Sensor0_physvol");
        if (!_ActiveSensor) {
          throw std::runtime_error("Could not find the ActiveSensor from Component0Volume");
        }
        
        //Get the surface
        std::shared_ptr<Acts::PlaneSurface> sensorSurface = GetSurface(_ActiveSensor,
                                                                       tracker_transform*ref1_transform*ref2_transform);
        if (sln == "LDMXTaggerModuleVolume_physvol1" ||
            sln == "LDMXTaggerModuleVolume_physvol2")
          tagger_layout["tagger_tracker_L1"].push_back(sensorSurface);

        if (sln == "LDMXTaggerModuleVolume_physvol3" ||
            sln == "LDMXTaggerModuleVolume_physvol4")
          tagger_layout["tagger_tracker_L2"].push_back(sensorSurface);

        if (sln == "LDMXTaggerModuleVolume_physvol5" ||
            sln == "LDMXTaggerModuleVolume_physvol6")
          tagger_layout["tagger_tracker_L3"].push_back(sensorSurface);

        if (sln == "LDMXTaggerModuleVolume_physvol7" ||
            sln == "LDMXTaggerModuleVolume_physvol8")
          tagger_layout["tagger_tracker_L4"].push_back(sensorSurface);

        if (sln == "LDMXTaggerModuleVolume_physvol9" ||
            sln == "LDMXTaggerModuleVolume_physvol10")
          tagger_layout["tagger_tracker_L5"].push_back(sensorSurface);

        if (sln == "LDMXTaggerModuleVolume_physvol11" ||
            sln == "LDMXTaggerModuleVolume_physvol12")
          tagger_layout["tagger_tracker_L6"].push_back(sensorSurface);

        if (sln == "LDMXTaggerModuleVolume_physvol13" ||
            sln == "LDMXTaggerModuleVolume_physvol14")
          tagger_layout["tagger_tracker_L7"].push_back(sensorSurface);
        
      }//found a silicon surface
    }// loop on daughters
}//build the layout


std::shared_ptr<Acts::PlaneSurface> TrackersTrackingGeometry::GetSurface(G4VPhysicalVolume* pvol,
                                                                         Acts::Transform3 ref_trans) {
  
  if (!pvol) 
    throw std::runtime_error("TrackersTrackingGeometry::GetSurface:: pvol is nullptr");

  
  //Get the surface transform
  Acts::Transform3 surface_transform = GetTransform(*pvol);
  //Compose the sensor_transform with the reference transform
  surface_transform = ref_trans * surface_transform;

  //Now transform to the tracker frame
  Acts::Transform3 surface_transform_tracker = toTracker(surface_transform);

  if (debug_) {
    std::cout<<"THE SENSOR TRANSFORM - TRANSLATION"<<std::endl;
    std::cout<<surface_transform.translation()(0)<<std::endl;
    std::cout<<surface_transform.translation()(1)<<std::endl;
    std::cout<<surface_transform.translation()(2)<<std::endl;
    std::cout<<"THE SENSOR TRANSFORM - ROTATION"<<std::endl;
    std::cout<<surface_transform.rotation()<<std::endl;
    
    std::cout<<"TO THE TRACKER FRAME"<<std::endl;
    std::cout<<surface_transform_tracker.translation()(0)<<std::endl;
    std::cout<<surface_transform_tracker.translation()(1)<<std::endl;
    std::cout<<surface_transform_tracker.translation()(2)<<std::endl;
    std::cout<<"THE SENSOR TRANSFORM - ROTATION"<<std::endl;
    std::cout<<surface_transform_tracker.rotation()<<std::endl;
  }
  

  
  //This material is defined in different units with respect what acts expects. I decided to hardcode here. TODO: fix this
  
  /*
    G4Material* sens_mat = _ActiveSensor->GetLogicalVolume()->GetMaterial();
    if (debug_) {
    std::cout<<"Checking the material of "<<l_vol->GetDaughter(i)->GetName()<<std::endl;
    std::cout<<"With sensor::"<<_ActiveSensor->GetName()<<std::endl;
    std::cout<<sens_mat->GetName()<<std::endl;
    std::cout<<"RL="<<sens_mat->GetRadlen()<<" lambda="<<sens_mat->GetNuclearInterLength()<<std::endl;
    std::cout<<"A="<<sens_mat->GetA()<<" Z="<<sens_mat->GetZ()<<" rho="<<sens_mat->GetDensity()<<std::endl;
    }
    
    Acts::Material silicon = Acts::Material::fromMassDensity(sens_mat->GetRadlen(),
    sens_mat->GetNuclearInterLength(),
    sens_mat->GetA(),
    sens_mat->GetZ(),
    sens_mat->GetDensity());
  */                
  
  //Define the silicon material
  Acts::Material silicon = Acts::Material::fromMassDensity(95.7 * Acts::UnitConstants::mm,
                                                           465.2 * Acts::UnitConstants::mm,
                                                           28.03,
                                                           14.,
                                                           2.32 * Acts::UnitConstants::g / Acts::UnitConstants::cm3);
  
  //Get the active sensor box
  G4Box* surfaceSolid = (G4Box*)(pvol->GetLogicalVolume()->GetSolid());
  
  if (debug_ ) {
    std::cout<<"Sensor Dimensions"<<std::endl;
    std::cout<<surfaceSolid->GetXHalfLength()<< " " <<
        surfaceSolid->GetYHalfLength()<< " " <<
        surfaceSolid->GetZHalfLength()<< " " <<std::endl;
  }
  
  //Form the material slab
  double thickness = 2*surfaceSolid->GetZHalfLength() * Acts::UnitConstants::mm;
  Acts::MaterialSlab silicon_slab(silicon,thickness);
  
  
  //Get the bounds
  std::shared_ptr<const Acts::RectangleBounds> rect_bounds = std::make_shared<const Acts::RectangleBounds>(
      Acts::RectangleBounds(surfaceSolid->GetXHalfLength() * Acts::UnitConstants::mm,
                            surfaceSolid->GetYHalfLength() * Acts::UnitConstants::mm));
  
  
  
  //Form the active sensor surface
  std::shared_ptr<Acts::PlaneSurface> surface =
      Acts::Surface::makeShared<Acts::PlaneSurface>(surface_transform_tracker,rect_bounds);
  surface->assignSurfaceMaterial(std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab));
  
  
  return surface;
}



} //tracking
} //reco 
