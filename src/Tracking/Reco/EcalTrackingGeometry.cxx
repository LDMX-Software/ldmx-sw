#include "Tracking/Reco/EcalTrackingGeometry.h"

namespace tracking {
namespace reco {


EcalTrackingGeometry::EcalTrackingGeometry(std::string gdmlfile,
                                           Acts::GeometryContext *gctx,
                                           bool debug) {

  _debug = debug;
  gctx_ = gctx;
  
  //Build The rotation matrix to the tracking frame
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
  
  y_rot_.col(0) = xPos1;
  y_rot_.col(1) = yPos1;
  y_rot_.col(2) = zPos1;
  
  //Rotate the sensors to put them in the proper orientation in Z
  Acts::Vector3 xPos2(1., 0. ,0.);
  Acts::Vector3 yPos2(0., cos(rotationAngle), sin(rotationAngle));
  Acts::Vector3 zPos2(0., -sin(rotationAngle),cos(rotationAngle));
  
  x_rot_.col(0) = xPos2;
  x_rot_.col(1) = yPos2;
  x_rot_.col(2) = zPos2;
  
  G4GDMLParser parser;

  //Validation requires internet
  parser.Read("/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12/detector.gdml", false );
  G4VPhysicalVolume* fWorldPhysVol = parser.GetWorldVolume();
  if (debug) {
    std::cout<<"World position"<<std::endl;
    std::cout<<fWorldPhysVol->GetTranslation()<<std::endl;
  }
  //Get the logical volume from the world
  G4LogicalVolume* fWorldLogicalVol = fWorldPhysVol->GetLogicalVolume();

  if (debug)
    std::cout<<"Loop on world daughters"<<std::endl;

  _ECAL = findDaughterByName(fWorldPhysVol,"em_calorimeters_PV");

  if (_ECAL) {
    std::cout<<_ECAL->GetName()<<std::endl;
    std::cout<<_ECAL->GetTranslation()<<std::endl;
    std::cout<<_ECAL->GetLogicalVolume()->GetNoDaughters()<<std::endl;
  }
  
  //getAllDaughters(_ECAL);

  
  if (_debug) {
    std::cout<<"Looking for later components"<<std::endl;
  }

  //Layers Config
  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layersCfg;

  //Get the layers
  std::vector<std::reference_wrapper<G4VPhysicalVolume>> components;
  std::vector<std::shared_ptr<const Acts::Surface>> sensor_surfaces;

  
  //PS
  getComponentRing("_a_", "Si",components);
  
  for (auto& component : components) {
    sensor_surfaces.push_back(convertHexToActsSurface(component));
  }
    
  for (int i = 0; i< sensor_surfaces.size(); i+=14) {
    
    std::vector<std::shared_ptr<const Acts::Surface> > rings;
    for (int j = 0; j < 14 ; j++) {
      rings.push_back(sensor_surfaces.at(i+j));
    }
    
    Acts::CuboidVolumeBuilder::LayerConfig lyCfg = buildLayerConfig(rings);
    layersCfg.push_back(lyCfg);
  }
  
  components.clear();
  sensor_surfaces.clear();
  
  //A

  
  getComponentRing("_a_", "Si",components);
  
  for (auto& component : components) {
    sensor_surfaces.push_back(convertHexToActsSurface(component));
  }
    
  for (int i = 0; i< sensor_surfaces.size(); i+=14) {
    
    std::vector<std::shared_ptr<const Acts::Surface> > rings;
    for (int j = 0; j < 14 ; j++) {
      rings.push_back(sensor_surfaces.at(i+j));
    }
    
    Acts::CuboidVolumeBuilder::LayerConfig lyCfg = buildLayerConfig(rings);
    layersCfg.push_back(lyCfg);
  }
  
  components.clear();
  sensor_surfaces.clear();
  
  /*
  //B

  
  getComponentRing("_b_", "Si",components);
    
  for (auto& component : components) {
    sensor_surfaces.push_back(convertHexToActsSurface(component));
  }
    
  for (int i = 0; i< sensor_surfaces.size(); i+=14) {
    
    std::vector<std::shared_ptr<const Acts::Surface> > rings;
    for (int j = 0; j < 14 ; j++) {
      rings.push_back(sensor_surfaces.at(i+j));
    }
    
    Acts::CuboidVolumeBuilder::LayerConfig lyCfg = buildLayerConfig(rings);
    layersCfg.push_back(lyCfg);
  }
  
  components.clear();
  sensor_surfaces.clear();
  

  
  //C

  std::cout<<"Getting the components for C"<<std::endl;
  getComponentRing("_c_", "Si",components);

  std::cout<<"Building the surfaces"<<std::endl;
  for (auto& component : components) {
    sensor_surfaces.push_back(convertHexToActsSurface(component));
  }

  std::cout<<"Selecting the rings"<<std::endl;
  
  for (int i = 0; i< sensor_surfaces.size(); i+=14) {
    
    std::vector<std::shared_ptr<const Acts::Surface> > rings;

    std::cout<<"Getting ring " << i / 14 << std::endl;
    for (int j = 0; j < 14 ; j++) {
      rings.push_back(sensor_surfaces.at(i+j));
    }
    
    Acts::CuboidVolumeBuilder::LayerConfig lyCfg = buildLayerConfig(rings);
    layersCfg.push_back(lyCfg);
  }
  
  components.clear();
  sensor_surfaces.clear();

  
  //D
  
  getComponentRing("_d_", "Si",components);
    
  for (auto& component : components) {
    sensor_surfaces.push_back(convertHexToActsSurface(component));
  }
    
  for (int i = 0; i< sensor_surfaces.size(); i+=14) {
    
    std::vector<std::shared_ptr<const Acts::Surface> > rings;
    for (int j = 0; j < 14 ; j++) {
      rings.push_back(sensor_surfaces.at(i+j));
    }
    
    Acts::CuboidVolumeBuilder::LayerConfig lyCfg = buildLayerConfig(rings);
    layersCfg.push_back(lyCfg);
  }
  
  components.clear();
  sensor_surfaces.clear();
  */
  
  
  Acts::CuboidVolumeBuilder::VolumeConfig ecal_vcfg;
  
  //TODO change this with the translation!
  ecal_vcfg.position  = {0.,0.,0.};
  G4Box* ecal_box = (G4Box*) _ECAL->GetLogicalVolume()->GetSolid();
  //ecal_vcfg.length   = {ecal_box->GetXHalfLength() * 2., ecal_box->GetYHalfLength() * 2. , ecal_box->GetZHalfLength() * 2.};

  ecal_vcfg.length   = {ecal_box->GetZHalfLength() * 2., ecal_box->GetXHalfLength() * 2. , ecal_box->GetYHalfLength() * 2.};
  
  ecal_vcfg.name = "Ecal_volume";
  ecal_vcfg.layerCfg = layersCfg;


  //Initialize the Cuboid Volume Builder
  Acts::CuboidVolumeBuilder cvb;
  Acts::CuboidVolumeBuilder::Config config;
  config.position = {0., 0., 0.};
  config.length   = {2000, 2000, 2000};
  config.volumeCfg = {ecal_vcfg};

  
  cvb.setConfig(config);

  Acts::TrackingGeometryBuilder::Config tgbCfg;
  tgbCfg.trackingVolumeBuilders.push_back(
      [=](const auto& cxt, const auto& inner, const auto&) {
        return cvb.trackingVolume(cxt, inner, nullptr);
         });
  Acts::TrackingGeometryBuilder tgb(tgbCfg);
  tGeometry_ = tgb.trackingGeometry(*gctx_);

  dumpGeometry("./ecal_test/");
}



G4VPhysicalVolume* EcalTrackingGeometry::findDaughterByName(G4VPhysicalVolume* pvol, G4String name) {
  G4LogicalVolume* lvol = pvol->GetLogicalVolume();
  for (G4int i=0; i<lvol->GetNoDaughters(); i++) {
    G4VPhysicalVolume* fDaughterPhysVol = lvol->GetDaughter(i);
    if (fDaughterPhysVol->GetName() == name)
      return fDaughterPhysVol;
  }
  
  return nullptr;
}

//A super layer is a full layer composed by multiple duouble-silicon layers.
//One pass a super layer, and this function returns a vector of all the components of the super layer of a certain type
//It then orders then by z location
//And returns a map where at every z the ring is organised in a vector with the numbering scheme as
//described here: https://www.overleaf.com/project/5f3d5a7e36c7f90001eddfc6

void EcalTrackingGeometry::getComponentRing(std::string super_layer_name,
                                            std::string component_type,
                                            std::vector<std::reference_wrapper<G4VPhysicalVolume>> & components) {
  
  G4LogicalVolume* l_ECAL = _ECAL->GetLogicalVolume();
  for (G4int i=0; i<l_ECAL->GetNoDaughters(); i++) {
    //std::cout<<"DEBUG:: Getting daughter.."<< i << " of "<< l_ECAL->GetNoDaughters()<<std::endl;
    std::string sln = l_ECAL->GetDaughter(i)->GetName();
    
    //std::cout<<sln<<std::endl;
    
    //Look for the specific super layer of the specific component
    if (sln.find(super_layer_name) != std::string::npos &&
        sln.find(component_type) != std::string::npos) {

      //std::cout<<"Found and adding " << sln<<std::endl;
      components.push_back(*(l_ECAL->GetDaughter(i)));
    }
    
  }

  //Sort the components along z
  //std::cout<<"Sorting..." <<std::endl;
  sort(components.begin(), components.end(), compareZlocation);
  
  
}

void EcalTrackingGeometry::getAllDaughters(G4VPhysicalVolume* pvol) {
  G4LogicalVolume* lvol = pvol->GetLogicalVolume();

  for (G4int i=0; i<lvol->GetNoDaughters(); i++) {
    G4VPhysicalVolume* fDaughterPhysVol = lvol->GetDaughter(i);
    if (_debug) {
      std::cout<<"name::"<<fDaughterPhysVol->GetName()<<std::endl;
      std::cout<<"pos::"<<fDaughterPhysVol->GetTranslation()<<std::endl;
      std::cout<<"n_dau::"<<fDaughterPhysVol->GetLogicalVolume()->GetNoDaughters()<<std::endl;
      std::cout<<"replica::"<<fDaughterPhysVol->IsReplicated()<<std::endl;
      std::cout<<"copyNR::"<<fDaughterPhysVol->GetCopyNo()<<std::endl;
    }
  }
}



void EcalTrackingGeometry::dumpGeometry(const std::string& outputDir ) {

  //Should fail if already exists
  boost::filesystem::create_directory(outputDir);
  
  
  double outputScalor = 1.0;
  size_t outputPrecision = 6;

  Acts::ObjVisualization3D objVis(outputPrecision, outputScalor);
  Acts::ViewConfig containerView = Acts::ViewConfig({220, 220, 220});
  Acts::ViewConfig volumeView = Acts::ViewConfig({220, 220, 0});
  Acts::ViewConfig sensitiveView = Acts::ViewConfig({0, 180, 240});
  Acts::ViewConfig passiveView = Acts::ViewConfig({240, 280, 0});
  Acts::ViewConfig gridView = Acts::ViewConfig({220, 0, 0});
  

  Acts::GeometryView3D::drawTrackingVolume(
      objVis, *(tGeometry_->highestTrackingVolume()),
      *gctx_, containerView,
      volumeView, passiveView, sensitiveView, gridView,
      true,"",".");
}

std::shared_ptr<const Acts::Surface> EcalTrackingGeometry::convertHexToActsSurface(const G4VPhysicalVolume& phex) {
  G4Polyhedra* hex =  (G4Polyhedra*) phex.GetLogicalVolume()->GetSolid();
  G4Material* hex_mat = phex.GetLogicalVolume()->GetMaterial();
  
  double side = hex->GetCorner(3).r;
  double height = (side / 2. ) * sqrt(3);
  double thickness = 0.5 * Acts::UnitConstants::mm;
    
  std::shared_ptr<Acts::PlaneSurface> surface;
  
  /// Constructor for convex hexagon symmetric about the y axis
  ///
  /// @param halfXnegY is the halflength in x at minimal y
  /// @param halfXzeroY is the halflength in x at y = 0
  /// @param halfXposY is the halflength in x at maximal y
  /// @param halfYneg is the halflength into y < 0
  /// @param halfYpos is the halflength into y > 0
  std::shared_ptr<const Acts::DiamondBounds> d_bounds = std::make_shared<Acts::DiamondBounds> (
      side / 2. *Acts::UnitConstants::mm,
      side *Acts::UnitConstants::mm,
      side / 2. *Acts::UnitConstants::mm,
      height *Acts::UnitConstants::mm,
      height *Acts::UnitConstants::mm);

  
  //Form the position and rotation
  //Acts::Vector3 pos(phex->GetTranslation().x(), phex->GetTranslation().y(), phex->GetTranslation().z());

  Acts::Vector3 pos(phex.GetTranslation().z(), phex.GetTranslation().x(), phex.GetTranslation().y());
  
  
  Acts::RotationMatrix3 rotation;
    
  //ConvertG4Rot(phex->GetObjectRotation(),rotation);
  ConvertG4Rot(phex.GetObjectRotationValue(),rotation);
  
  //Now rotate to the tracking frame
  rotation = x_rot_ * y_rot_ * rotation;
    
  
  Acts::Translation3 translation(pos);
  Acts::Transform3 transform(translation * rotation); 
  surface = Acts::Surface::makeShared<Acts::PlaneSurface>(transform,d_bounds);

  
  
  Acts::Material silicon = Acts::Material::fromMassDensity(hex_mat->GetRadlen(),
                                                           hex_mat->GetNuclearInterLength(),
                                                           hex_mat->GetA(),
                                                           hex_mat->GetZ(),
                                                           hex_mat->GetDensity());
  
  Acts::MaterialSlab silicon_slab(silicon,thickness);
  surface->assignSurfaceMaterial(std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab));
  
  surface->toStream(*gctx_,std::cout);
  
  return surface;  
}

//Convert rotation

void EcalTrackingGeometry::ConvertG4Rot(const G4RotationMatrix& g4rot, Acts::RotationMatrix3& rot) {

  rot(0,0) = g4rot.xx();
  rot(0,1) = g4rot.xy();
  rot(0,2) = g4rot.xz();

  rot(1,0) = g4rot.yx();
  rot(1,1) = g4rot.yy();
  rot(1,2) = g4rot.yz();

  rot(2,0) = g4rot.zx();
  rot(2,1) = g4rot.zy();
  rot(2,2) = g4rot.zz();
  
}

//Convert translation

Acts::Vector3 EcalTrackingGeometry::ConvertG4Pos(const G4ThreeVector& g4pos) {

  std::cout<<std::endl;
  std::cout<<"g4pos::"<<g4pos<<std::endl;
  Acts::Vector3 trans{g4pos.x(), g4pos.y(), g4pos.z()};
  std::cout<<trans<<std::endl;
  return trans;
  
}

//A layer is formed by 2 sides of silicon + material
//
Acts::CuboidVolumeBuilder::LayerConfig EcalTrackingGeometry::buildLayerConfig( const std::vector<std::shared_ptr<const Acts::Surface>>& rings,
                                                                               double clearance,
                                                                               bool active) {
  
  Acts::CuboidVolumeBuilder::LayerConfig lcfg;

  for (auto & hex : rings) {
    lcfg.surfaces.push_back(hex);
  }

  //Get the surface thickness
  Acts::Vector2 ploc{0.,0.};
  double thickness = rings.front()->surfaceMaterial()->materialSlab(ploc).thickness();
  
  lcfg.envelopeX = std::pair<double,double>{thickness /2. + clearance, thickness / 2. + clearance};
  lcfg.active = active;

  return lcfg;
}
}//reco
}//tracking
