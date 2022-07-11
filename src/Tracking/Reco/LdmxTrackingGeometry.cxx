#include "Tracking/Reco/LdmxTrackingGeometry.h"

namespace tracking{
namespace reco {
LdmxTrackingGeometry::LdmxTrackingGeometry(dd4hep::Detector* detector,
                                           Acts::GeometryContext* gctx) {

  detector_ = detector;
  gctx_  = gctx;


  dd4hep::DetElement world{detector_->world()};
  Acts::CuboidVolumeBuilder cvb;
  std::vector<dd4hep::DetElement> subdetectors;

  auto loggingLevel = Acts::Logging::INFO;
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("DD4hepConversion", loggingLevel));
  
  collectSubDetectors_dd4hep(world,subdetectors);

  if (debug_)
    std::cout<<__PRETTY_FUNCTION__<<" size of subdetectors::"<<subdetectors.size()<<std::endl;


  std::vector<Acts::CuboidVolumeBuilder::VolumeConfig> volBuilderConfigs;
  for (auto& subDetector : subdetectors) {
    if (debug_)
      std::cout<<"PF::DEBUG:: Translating DD4Hep sub detector: " << subDetector.name()<<std::endl;
    //create the cuboid volume configurations for the builder
    volBuilderConfigs.push_back(volumeBuilder_dd4hep(subDetector,loggingLevel));
  }
  
  //Create the builder
  Acts::CuboidVolumeBuilder::Config config;
  config.position = {0., 0., 0.};
  config.length = {2000, 2000, 2000};
  config.volumeCfg = volBuilderConfigs;
  
  cvb.setConfig(config);
    
  Acts::TrackingGeometryBuilder::Config tgbCfg;
  tgbCfg.trackingVolumeBuilders.push_back(
      [=](const auto& cxt, const auto& inner, const auto&) {
        return cvb.trackingVolume(cxt, inner, nullptr);
      });
  
  Acts::TrackingGeometryBuilder tgb(tgbCfg);
  
  tGeometry_ = 
      tgb.trackingGeometry(*gctx_);

}

void LdmxTrackingGeometry::collectSensors_dd4hep(dd4hep::DetElement& detElement,
                                                 std::vector<dd4hep::DetElement>& sensors) {

  if (debug_)
    std::cout<<__PRETTY_FUNCTION__<<" Collecting from " <<detElement.name()<<std::endl;
        
  const dd4hep::DetElement::Children& children = detElement.children();
        
  for (auto& child : children) {
    dd4hep::DetElement childDetElement = child.second;
    std::string childType = childDetElement.type();
            
    if (childType != "si_sensor")
      continue;
            
    //Check if there is an Acts extension associated to this detElement. If not, add it.
            
    Acts::ActsExtension* detExtension = nullptr;
    try {
      detExtension = childDetElement.extension<Acts::ActsExtension>();
      //std::cout<<detExtension->toString()<<std::endl;
    }
    catch (std::runtime_error& e) {
      //std::cout<<"Caught exception in "<<__PRETTY_FUNCTION__<<std::endl;
      //continue;
    }
            
    //Add the child if the detExtension is the TaggerTracker, the RecoilTracker or the Target(?)
    if ((detExtension!=nullptr)) {
      if (detExtension->hasType(childDetElement.name(),"si_sensor")){
        sensors.push_back(childDetElement);
      }
    }
    else {  //ActsExtension doesn't exist
      //std::cout<<__PRETTY_FUNCTION__<<"PF::DEBUG:: Adding the ActsExtension for sensor "<<childDetElement.name()<<std::endl;
      detExtension = new Acts::ActsExtension();
      detExtension->addType(childDetElement.name(), "si_sensor");
      childDetElement.addExtension<Acts::ActsExtension>(detExtension);
      sensors.push_back(childDetElement);
    }
  } // children loop
}// get sensors.

//Collect the subdetectors and add the ActsExtension to them    
//I expect to find the TaggerTracker and the RecoilTracker.
void LdmxTrackingGeometry::collectSubDetectors_dd4hep(dd4hep::DetElement& detElement,
                                                       std::vector<dd4hep::DetElement>& subdetectors) {
  const dd4hep::DetElement::Children& children = detElement.children();
    
  //std::cout<<"Collecting from "<<detElement.name()<<std::endl;
        
  for (auto& child : children) {
    dd4hep::DetElement childDetElement = child.second;
    
    if (debug_) {
      std::cout<<"Child Name:: "<<childDetElement.name()<<std::endl;
      std::cout<<"Child Type:: "<<childDetElement.type()<<std::endl;
    }
    
    std::string childName = childDetElement.name();
    
    //Check here if I'm checking the TaggerTracker or the RecoilTracker. Skip the rest.
    if (
            childName != "TaggerTracker"  &&
            childName != "tagger_tracker" &&
            childName != "recoil_tracker" &&
            childName != "RecoilTracker") {
      continue;
    }
        
    //Check if an Acts extension is attached to the det Element (not necessary)
    Acts::ActsExtension* detExtension = nullptr;
    try {
      detExtension = childDetElement.extension<Acts::ActsExtension>();
      //std::cout<<detExtension->toString()<<std::endl;
    }
    catch (std::runtime_error& e) {
      //std::cout<<"Caught exception in "<<__PRETTY_FUNCTION__<<std::endl;
      //continue;
    }
        
    //Add the child if the detExtension is the TaggerTracker, the RecoilTracker or the Target(?)
    if ((detExtension!=nullptr)) {
      if (detExtension->hasType("TaggerTracker","detector") ||  
          detExtension->hasType("RecoilTracker","detector") ||
          detExtension->hasType("recoil_tracker","detector") ||
          detExtension->hasType("tagger_tracker","detector")) {
        subdetectors.push_back(childDetElement);
      }
    }
    else {  //ActsExtension doesn't exist
      //std::cout<<"PF::DEBUG:: Adding the ActsExtension to "<< childDetElement.name() << " " <<childDetElement.type() <<std::endl;
            
      detExtension = new Acts::ActsExtension();
      detExtension->addType(childDetElement.name(), "detector");
      childDetElement.addExtension<Acts::ActsExtension>(detExtension);
      detExtension->addType("axes", "definitions", "XYZ"); // no effect in changing this line for the sensors.
      subdetectors.push_back(childDetElement);
    }
        
    //recursive
    collectSubDetectors_dd4hep(childDetElement,subdetectors);
  }//children loop
}

void LdmxTrackingGeometry::resolveSensitive(
    const dd4hep::DetElement& detElement,
    std::vector<std::shared_ptr<const Acts::Surface>>& surfaces,bool force) const {
  const dd4hep::DetElement::Children& children = detElement.children();
  
  if (!children.empty()) {
    for (auto& child : children) {
      dd4hep::DetElement childDetElement = child.second;
                      
      //Check material
      //std::cout<<childDetElement.volume().material().toString()<<std::endl;
                
      if (childDetElement.volume().isSensitive() || force) {
        //std::cout<<"isSensitive.. "<<std::endl;
        // create the surface
        surfaces.push_back(createSensitiveSurface(childDetElement));
      }
      resolveSensitive(childDetElement, surfaces,force);
    }
  }
}//resolve sensitive


std::shared_ptr<const Acts::Surface>
LdmxTrackingGeometry::createSensitiveSurface(
    const dd4hep::DetElement& detElement) const {
  // access the possible extension of the DetElement
  Acts::ActsExtension* detExtension = nullptr;
  try {
    detExtension = detElement.extension<Acts::ActsExtension>();
  } catch (std::runtime_error& e) {
    //ACTS_WARNING("Could not get Acts::Extension");
    return nullptr;
  }
        
  //Axes orientations
  auto detAxis = detExtension->getType("axes", "definitions");

  //Add the material
  dd4hep::Material de_mat = detElement.volume().material();
  //std::cout<<childDetElement.volume().material().toString()<<std::endl;
  //std::cout<<"Silicon density "<<de_mat.density()<<std::endl;
  
  /*
    Acts::Material silicon = Acts::Material::fromMassDensity(de_mat.radLength() * Acts::UnitConstants::mm,
    de_mat.intLength() * Acts::UnitConstants::mm,
    de_mat.A(),
    de_mat.Z(),
    de_mat.density() * Acts::UnitConstants::g / Acts::UnitConstants::cm3);
  */
  Acts::Material silicon = Acts::Material::fromMassDensity(95.7 * Acts::UnitConstants::mm,
                                                           465.2 * Acts::UnitConstants::mm,
                                                           28.03,
                                                           14.,
                                                           2.32 * Acts::UnitConstants::g / Acts::UnitConstants::cm3);
  
  //Get the thickness. The bounding box gives back half of the size in z. I scaled of factor 10 to bring it in mm. The detElement stores in cm units
  double thickness = 2*Acts::UnitConstants::cm*detElement.volume().boundingBox().z();
  
  Acts::MaterialSlab silicon_slab(silicon,thickness); 
  std::shared_ptr<Acts::HomogeneousSurfaceMaterial> homogeneous_mat = std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab);
        
  // Create the corresponding detector element !- memory leak --!
  //I've checked: cm is the right scalor to use since it picks the right units from the xml. 
  //Using cm here to multiply by 10. 
  Acts::DD4hepDetectorElement* dd4hepDetElement =
      new Acts::DD4hepDetectorElement(detElement, detAxis, Acts::UnitConstants::cm, 
                                      false, homogeneous_mat, nullptr); //is disc is always false.
        

  // return the surface
  return dd4hepDetElement->surface().getSharedPtr();
}

// I use UnitConstants::cm because the seems like that the dd4hep classes store the informations in cm
    
Acts::Transform3 LdmxTrackingGeometry::convertTransform(
    const TGeoMatrix* tGeoTrans) const {
  // get the placement and orientation in respect to its mother
  const Double_t* rotation = tGeoTrans->GetRotationMatrix();
  const Double_t* translation = tGeoTrans->GetTranslation();

  return Acts::TGeoPrimitivesHelper::makeTransform(
      Acts::Vector3(rotation[0], rotation[3], rotation[6]),
      Acts::Vector3(rotation[1], rotation[4], rotation[7]),
      Acts::Vector3(rotation[2], rotation[5], rotation[8]),
      Acts::Vector3(translation[0] * Acts::UnitConstants::cm,
                    translation[1] * Acts::UnitConstants::cm,
                    translation[2] * Acts::UnitConstants::cm));
}


void LdmxTrackingGeometry::getSurfaces(std::vector<const Acts::Surface*>& surfaces,
                                        std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry) {
  
  const Acts::TrackingVolume* tVolume = trackingGeometry->highestTrackingVolume();
  if (tVolume->confinedVolumes()) {
    for (auto volume : tVolume->confinedVolumes()->arrayObjects()) {
      if (volume->confinedLayers()) {
        for (const auto& layer : volume->confinedLayers()->arrayObjects()) {
          if (layer->layerType() == Acts::navigation) continue;
          for (auto surface : layer->surfaceArray()->surfaces()) {
            if (surface) {

              surfaces.push_back(surface);
                            
            }// surface exists
          } //surfaces
        }//layers objects
      }//confined layers
    }//volumes objects
  }//confined volumes
}

//A copy is not a good idea. TODO
Acts::CuboidVolumeBuilder::VolumeConfig LdmxTrackingGeometry::volumeBuilder_dd4hep(dd4hep::DetElement& subdetector,Acts::Logging::Level logLevel) {
  
  // Get the extension, if it exists
  Acts::ActsExtension* subDetExtension = nullptr;
  try {
    subDetExtension = subdetector.extension<Acts::ActsExtension>();
  } catch (std::runtime_error& e) {
  }
    
  //Just a place holder in the case we will make compound detectors. 
  if (subdetector.type() == "compound") {}
  // Now create the Layerbuilders and Volumebuilder

  // Define the configuration for the cuboid volume builder object 
  Acts::CuboidVolumeBuilder::Config cvbConfig;
    
  // Get the sensors
  std::vector<dd4hep::DetElement> sensors;
    
  //collect the sensors (just a trick to add the ActsExtension)
    
  collectSensors_dd4hep(subdetector,sensors);
  if (debug_)
    std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" the size of sensors="<<sensors.size()<<std::endl;
    
  std::vector<std::shared_ptr<const Acts::Surface>> surfaces;
    
  //Get all the sensitive surfaces for the tagger Tracker. 
  //For the moment I'm forcing to grep everything.
  
  //resolveSensitive(subdetector,surfaces,true);
  //if (debug_)
  //  std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" surfaces size::"<<surfaces.size()<<std::endl;
    
  //Check the surfaces that I created (but I will not use)

  //if (debug_)  {
    
  //  for (auto& surface : surfaces) {
      
  //    surface->toStream(gctx_,std::cout);
  //    std::cout<<std::endl;
  //    surface->surfaceMaterial()->toStream(std::cout);
  //    std::cout<<std::endl;
  //    std::cout<<surface->surfaceMaterial()->materialSlab(0,0).material().massDensity()<<std::endl;
  //    std::cout<<surface->surfaceMaterial()->materialSlab(0,0).material().molarDensity()<<std::endl;
  //  }
  //}
    
  //Surfaces configs
  //This bypasses the surfaces built before
  std::vector<Acts::CuboidVolumeBuilder::SurfaceConfig> surfaceConfig;
  
  //Reorder the sensors in ascending order along the beam axis.
  //TODO::USE A MAP instead of all these vector manipulations//
    
  std::sort(sensors.begin(),sensors.end(),[](const dd4hep::DetElement& lhs,
                                             const dd4hep::DetElement& rhs) {
    const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
    const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
    return t_lhs[2] <= t_rhs[2];
  });
  
  //Check if I'm doing the recoil
  if (sensors.size() > 25 ) {
    
    //Now all the recoil sensors should be at the end of the sensor vector (10 elements).
    //Let's sort the first 5 and then the second 5

    // - First sort by y
    
    std::sort(sensors.end() - 20, sensors.end() - 10,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[1] > t_rhs[1];
              });
    
    // - Then sort by x
    
    std::sort(sensors.end() - 20, sensors.end() - 15,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });

    std::sort(sensors.end() - 15, sensors.end() - 10,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });

    
    // - First sort by y
    
    std::sort(sensors.end() - 10, sensors.end(),
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[1] > t_rhs[1];
              });
    
    // - Then sort by x
    
    std::sort(sensors.end() - 10, sensors.end() - 5,
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });

    std::sort(sensors.end() - 5, sensors.end(),
              [](const dd4hep::DetElement& lhs,
                 const dd4hep::DetElement& rhs){
                const Double_t* t_lhs = lhs.nominal().worldTransformation().GetTranslation();
                const Double_t* t_rhs = rhs.nominal().worldTransformation().GetTranslation();
                return t_lhs[0] > t_rhs[0];
              });
    
  } // recoil check
  
  //Prepare the surface configurations 
  int counter = 0;
  for (auto& sensor : sensors) {
    counter++;
    
    Acts::CuboidVolumeBuilder::SurfaceConfig cfg;

    if (debug_)
      std::cout<<"Getting the transform "<<sensor.name()<<std::endl;
        
    //Get the tranformation from the alignment support. 
    auto transform =
        convertTransform(&(sensor.nominal().worldTransformation()));

    //Rotate Z->X, X->Y, Y->Z 
    Acts::Vector3 position = {transform.translation()[2], transform.translation()[0], transform.translation()[1]};
        
    //Rotate the sensors to be orthogonal to X
    double rotationAngle = M_PI * 0.5;
        
    // 0 0 -1
    // 0 1 0
    // 1 0 0
        
    //This rotation is needed to have the plane orthogonal to the X direction. 
    // Rotation of the surfaces
    Acts::Vector3 xPos1(cos(rotationAngle), 0., sin(rotationAngle));
    Acts::Vector3 yPos1(0., 1., 0.);
    Acts::Vector3 zPos1(-sin(rotationAngle), 0., cos(rotationAngle));
        
    Acts::RotationMatrix3 y_rot;
    y_rot.col(0) = xPos1;
    y_rot.col(1) = yPos1;
    y_rot.col(2) = zPos1;

    //Rotate the sensors to put them in the proper orientation in Z
    Acts::Vector3 xPos2(1., 0. ,0.);
    Acts::Vector3 yPos2(0., cos(rotationAngle), sin(rotationAngle));
    Acts::Vector3 zPos2(0., -sin(rotationAngle),cos(rotationAngle));

    Acts::RotationMatrix3 x_rot;
    x_rot.col(0) = xPos2;
    x_rot.col(1) = yPos2;
    x_rot.col(2) = zPos2;
    
        
    cfg.position = position;
    //cfg.rotation = cfg.rotation*transform.rotation();
    cfg.rotation = x_rot*y_rot*transform.rotation();

    /*
    if (debug_) {
      std::cout<<"POSITION AND ROTATION OF THE SURFACES"<<std::endl;
      //Position and rotation of the surface
      std::cout<<cfg.position<<std::endl;
      std::cout<<cfg.rotation<<std::endl;
    }
    */
    
    //Get the bounds - 
    //cfg.rBounds  = std::make_shared<const Acts::RectangleBounds>(Acts::RectangleBounds(20.17, 50));
    
    cfg.rBounds  = std::make_shared<const Acts::RectangleBounds>(
        Acts::RectangleBounds(Acts::UnitConstants::cm*sensor.volume().boundingBox().x(),
                              Acts::UnitConstants::cm*sensor.volume().boundingBox().y()));
    
    // I don't think I need this to be defined.
    cfg.detElementConstructor = nullptr; 
        
    // Thickness. The units in dd4hep are in cm, that's why scaling. And they are in half lengths, that's why x2
    double thickness = 2*Acts::UnitConstants::cm*sensor.volume().boundingBox().z();
                    
    // Material
        
    dd4hep::Material de_mat = sensor.volume().material();
    Acts::Material silicon = Acts::Material::fromMassDensity(de_mat.radLength(),de_mat.intLength(), de_mat.A(), de_mat.Z(), de_mat.density());
    Acts::MaterialSlab silicon_slab(silicon,thickness); 
    cfg.thickness = thickness;
    cfg.surMat = std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab);


    //Store the configurations
    std::string sensor_name=sensor.name();

    if (sensor_name == "tagger_tracker_layer_1" || sensor_name == "tagger_tracker_layer_2") {
      tracker_layout["tagger_tracker_layer_L1"].push_back(cfg);
    }
    
    if (sensor_name == "tagger_tracker_layer_3" || sensor_name == "tagger_tracker_layer_4") {
      tracker_layout["tagger_tracker_layer_L2"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_5" || sensor_name == "tagger_tracker_layer_6") {
      tracker_layout["tagger_tracker_layer_L3"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_7" || sensor_name == "tagger_tracker_layer_8") {
      tracker_layout["tagger_tracker_layer_L4"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_9" || sensor_name == "tagger_tracker_layer_10") {
      tracker_layout["tagger_tracker_layer_L5"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_11" || sensor_name == "tagger_tracker_layer_12") {
      tracker_layout["tagger_tracker_layer_L6"].push_back(cfg);
    }

    if (sensor_name == "tagger_tracker_layer_13" || sensor_name == "tagger_tracker_layer_14") {
      tracker_layout["tagger_tracker_layer_L7"].push_back(cfg);
    }
    
    /* recoil association */
    
    if (sensor_name == "recoil_tracker_layer_1" || sensor_name == "recoil_tracker_layer_2") {
      tracker_layout["recoil_tracker_layer_L1"].push_back(cfg);
    }
    
    if (sensor_name == "recoil_tracker_layer_3" || sensor_name == "recoil_tracker_layer_4") {
      tracker_layout["recoil_tracker_layer_L2"].push_back(cfg);
    }
        
    if (sensor_name == "recoil_tracker_layer_5" || sensor_name == "recoil_tracker_layer_6") {
      tracker_layout["recoil_tracker_layer_L3"].push_back(cfg);
    }

    if (sensor_name == "recoil_tracker_layer_7" || sensor_name == "recoil_tracker_layer_8") {
      tracker_layout["recoil_tracker_layer_L4"].push_back(cfg);
    }
    
    if (sensor_name == "recoil_tracker_layer_9" ||
        sensor_name == "recoil_tracker_layer_10" ||
        sensor_name == "recoil_tracker_layer_11" ||
        sensor_name == "recoil_tracker_layer_12" ||
        sensor_name == "recoil_tracker_layer_13" ||
        sensor_name == "recoil_tracker_layer_14" ||
        sensor_name == "recoil_tracker_layer_15" ||
        sensor_name == "recoil_tracker_layer_16" ||
        sensor_name == "recoil_tracker_layer_17" ||
        sensor_name == "recoil_tracker_layer_18" 
        ) {
      tracker_layout["recoil_tracker_layer_L5"].push_back(cfg);
    }

    if (sensor_name == "recoil_tracker_layer_19" ||
        sensor_name == "recoil_tracker_layer_20" ||
        sensor_name == "recoil_tracker_layer_21" ||
        sensor_name == "recoil_tracker_layer_22" ||
        sensor_name == "recoil_tracker_layer_23" ||
        sensor_name == "recoil_tracker_layer_24" ||
        sensor_name == "recoil_tracker_layer_25" ||
        sensor_name == "recoil_tracker_layer_26" ||
        sensor_name == "recoil_tracker_layer_27" ||
        sensor_name == "recoil_tracker_layer_28" 
        ) {
      tracker_layout["recoil_tracker_layer_L6"].push_back(cfg);
    }
        
  } // sensors loop
  
  //if (debug_)
  //  std::cout<<"Formed " <<surfaceConfig.size()<< " Surface configs"<<std::endl;
  
  //Layer Configurations
  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layerConfig;
    
  for (auto const& x : tracker_layout)
  {
    /*
    if (debug_) {
      std::cout << x.first  
                << ": surfaces==>" 
                << x.second.size()
                << std::endl;
    }
    */
    
    Acts::CuboidVolumeBuilder::LayerConfig lcfg;
    lcfg.surfaceCfg = x.second;
    //lcfg.rotation = Acts::RotationMatrix3::Identity();
    double clearance = 0.01; //0.001
    lcfg.envelopeX = std::pair<double,double>{x.second.front().thickness / 2. + clearance, x.second.front().thickness / 2. + clearance};
    layerConfig.push_back(lcfg);
    lcfg.active = true;
  }
  
  
  
  if (debug_)
    std::cout<<"Formed " <<layerConfig.size()<< " layer configs"<<std::endl;
    
  //Create the volume

  if (debug_)
    std::cout<<"FORMING the boundaries for:"<<subdetector.name()<<std::endl;
  // Build the sub-detector volume configuration
  Acts::CuboidVolumeBuilder::VolumeConfig subDetVolumeConfig;
    
  // Get the transform wrt the world
  auto subDet_transform = convertTransform(&(subdetector.nominal().worldTransformation()));

  if (debug_) {
    std::cout<<subDet_transform.translation()<<std::endl;
    std::cout<<subDet_transform.rotation()<<std::endl;
  }

  //Rotate..Z->X, X->Y, Y->Z
  //Add 1mm to not make it sit on the first layer surface
  Acts::Vector3 sub_det_position = {subDet_transform.translation()[2]-1, subDet_transform.translation()[0], subDet_transform.translation()[1]};
  
  double x_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().z()+1;
  double y_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().x();
  double z_length = 2*Acts::UnitConstants::cm*subdetector.volume().boundingBox().y();

  //Larger volume to check propagation in the recoil area
  //x_length = 7*Acts::UnitConstants::cm*subdetector.volume().boundingBox().z()+1;
  
  if (debug_)
    std::cout<<"x "<<x_length<<" y "<<y_length<<" z "<<z_length<<std::endl;

  subDetVolumeConfig.position = sub_det_position;
  subDetVolumeConfig.length = {x_length, y_length, z_length};
  subDetVolumeConfig.layerCfg = layerConfig;
  subDetVolumeConfig.name = subdetector.name();
    
  //Form the Homogeneous material for the tagger volume
  dd4hep::Material subde_mat = subdetector.volume().material();
  Acts::Material subdet_mat = Acts::Material::fromMassDensity(subde_mat.radLength(),
                                                              subde_mat.intLength(), subde_mat.A(), 
                                                              subde_mat.Z(), subde_mat.density()); 
  
  subDetVolumeConfig.volumeMaterial =
      std::make_shared<Acts::HomogeneousVolumeMaterial>(subdet_mat);
  
  
  //Clear up the layout map to accept the new sub-detector
  tracker_layout.clear();
  
  return subDetVolumeConfig;
  
}

}
}
