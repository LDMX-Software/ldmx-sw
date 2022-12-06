#include "Tracking/Reco/TrackersTrackingGeometry.h"

namespace tracking {
namespace reco {


TrackersTrackingGeometry::TrackersTrackingGeometry(std::string gdmlfile,
                         Acts::GeometryContext* ctx, bool debug) :
    BaseTrackingGeometry(gdmlfile, ctx, debug) {

  if (debug_)
    std::cout<<"Looking for Tagger and Recoil volumes"<<std::endl;


  Tagger_ = findDaughterByName(fWorldPhysVol_, "tagger_PV");
  getAllDaughters(Tagger_);
  BuildLayoutMap(Tagger_,"LDMXTaggerModuleVolume_physvol");
    
  
  
  Recoil_ = findDaughterByName(fWorldPhysVol_, "recoil_PV");
  getAllDaughters(Recoil_);
  BuildLayoutMap(Recoil_,"recoil");
  
  


  //Layers Config
  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layersCfg;

  //Get the layers
  std::vector<std::shared_ptr<const Acts::Surface>> sensor_surfaces;
  
}


//This function gets the surfaces from the trackers and orders them in ascending z.

void TrackersTrackingGeometry::BuildLayoutMap(G4VPhysicalVolume* pvol,
                                              std::string surfacename) {
  
  if (debug_)
    std::cout<<"Building layout for the "<<pvol->GetName()<<" tracker"<<std::endl;
    
    G4LogicalVolume* l_vol = pvol -> GetLogicalVolume();
    for (G4int i=0; i<l_vol->GetNoDaughters(); i++) {
      std::string sln = l_vol->GetDaughter(i)->GetName();

      //To distinguish which layers need to be selected 
      if (sln.find(surfacename) != std::string::npos) {
        
        Acts::CuboidVolumeBuilder::SurfaceConfig cfg;
        
        //Convert the physical volume from G4 to an ACTS::Surface
        Acts::Transform3 surface_transform = GetTransform(*(l_vol->GetDaughter(i)));
        
        //Get the silicon sensor
        // Box for the module (slightly bigger than the sensor)
        // LDMXTaggerModuleVolume_physvol -> LDMXTaggerModuleVolume_component0Box, this is the sensor + inactive region
        // LDMXTaggerModuleVolume_component0Box-> LDMXTaggerModuleVolume_component0Sensor0Box, this is the sensor itself (active region)

        //Tagger_ -> LDMXTaggerModuleVolume_physvol1 -> LDMXTaggerModuleVolume_component0_physvol ->LDMXTaggerModuleVolume_component0Sensor0_physvol
        //        -> Get Box for the dimension: GetLogical->GetSolid
        //        For the positioning of the sensor:
        //  A     position of physvol1 + position of the component0_physvol + position of the sensor physvol. Thickness I can grab it from the box, or I just hardcode it.
        //  S     transform1 * transform2 * transform3 ....

        

        G4VPhysicalVolume* _Component0Volume = findDaughterByName(l_vol->GetDaughter(i),"LDMXTaggerModuleVolume_component0_physvol");
        G4VPhysicalVolume* _ActiveSensor = findDaughterByName(_Component0Volume,"LDMXTaggerModuleVolume_component0Sensor0_physvol");
        
        if (!_ActiveSensor) {
          
          getAllDaughters(l_vol->GetDaughter(i));
          return;
        }
        //  throw std::runtime_error("Could not find LDMXTaggerModuleVolume_component0Sensor0");
        // }
        
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
        
        //Thickness?!
        double thickness = 0.32 * Acts::UnitConstants::mm;
        Acts::MaterialSlab silicon_slab(silicon,thickness);

        std::shared_ptr<const Acts::RectangleBounds> rect_bounds = std::make_shared<const Acts::RectangleBounds>(
            Acts::RectangleBounds(10 * Acts::UnitConstants::mm,
                                  10 * Acts::UnitConstants::mm));
        
        std::shared_ptr<Acts::PlaneSurface> surface =
            Acts::Surface::makeShared<Acts::PlaneSurface>(surface_transform,rect_bounds);
        surface->assignSurfaceMaterial(std::make_shared<Acts::HomogeneousSurfaceMaterial>(silicon_slab));
        

        //Get the position
        
        if (sln == "LDMXTaggerModuleVolume_physvol1" ||
            sln == "LDMXTaggerModuleVolume_physvol2")
          tracker_layout["tagger_tracker_L1"].push_back(cfg);

        if (sln == "LDMXTaggerModuleVolume_physvol3" ||
            sln == "LDMXTaggerModuleVolume_physvol4")
          tracker_layout["tagger_tracker_L2"].push_back(cfg);

        if (sln == "LDMXTaggerModuleVolume_physvol5" ||
            sln == "LDMXTaggerModuleVolume_physvol6")
          tracker_layout["tagger_tracker_L3"].push_back(cfg);

        if (sln == "LDMXTaggerModuleVolume_physvol7" ||
            sln == "LDMXTaggerModuleVolume_physvol8")
          tracker_layout["tagger_tracker_L4"].push_back(cfg);

        if (sln == "LDMXTaggerModuleVolume_physvol9" ||
            sln == "LDMXTaggerModuleVolume_physvol10")
          tracker_layout["tagger_tracker_L5"].push_back(cfg);

        if (sln == "LDMXTaggerModuleVolume_physvol11" ||
            sln == "LDMXTaggerModuleVolume_physvol12")
          tracker_layout["tagger_tracker_L6"].push_back(cfg);

        if (sln == "LDMXTaggerModuleVolume_physvol13" ||
            sln == "LDMXTaggerModuleVolume_physvol14")
          tracker_layout["tagger_tracker_L7"].push_back(cfg);
        
      }//found a silicon surface
    }// loop on daughters
}//build the layout


} //tracking
} //reco 
