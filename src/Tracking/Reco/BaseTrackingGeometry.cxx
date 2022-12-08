#include "Tracking/Reco/BaseTrackingGeometry.h"

namespace tracking {
namespace reco {

BaseTrackingGeometry::BaseTrackingGeometry(std::string gdmlfile,
                                           Acts::GeometryContext* gctx,
                                           bool debug) {
  debug_ = debug;
  gctx_ = gctx;
  gdml_ = gdmlfile;

  // Build The rotation matrix to the tracking frame
  // Rotate the sensors to be orthogonal to X
  double rotationAngle = M_PI * 0.5;

  // 0 0 -1
  // 0 1 0
  // 1 0 0

  // This rotation is needed to have the plane orthogonal to the X direction.
  //  Rotation of the surfaces
  Acts::Vector3 xPos1(cos(rotationAngle), 0., sin(rotationAngle));
  Acts::Vector3 yPos1(0., 1., 0.);
  Acts::Vector3 zPos1(-sin(rotationAngle), 0., cos(rotationAngle));

  y_rot_.col(0) = xPos1;
  y_rot_.col(1) = yPos1;
  y_rot_.col(2) = zPos1;

  // Rotate the sensors to put them in the proper orientation in Z
  Acts::Vector3 xPos2(1., 0., 0.);
  Acts::Vector3 yPos2(0., cos(rotationAngle), sin(rotationAngle));
  Acts::Vector3 zPos2(0., -sin(rotationAngle), cos(rotationAngle));

  x_rot_.col(0) = xPos2;
  x_rot_.col(1) = yPos2;
  x_rot_.col(2) = zPos2;

  // Get the world volume

  G4GDMLParser parser;

  // Validation requires internet
  parser.Read(gdml_, false);

  fWorldPhysVol_ = parser.GetWorldVolume();
}

G4VPhysicalVolume* BaseTrackingGeometry::findDaughterByName(
    G4VPhysicalVolume* pvol, G4String name) {
  G4LogicalVolume* lvol = pvol->GetLogicalVolume();
  for (G4int i = 0; i < lvol->GetNoDaughters(); i++) {
    G4VPhysicalVolume* fDaughterPhysVol = lvol->GetDaughter(i);
    if (fDaughterPhysVol->GetName() == name) return fDaughterPhysVol;
  }

  return nullptr;
}

void BaseTrackingGeometry::getAllDaughters(G4VPhysicalVolume* pvol) {
  G4LogicalVolume* lvol = pvol->GetLogicalVolume();

  if (debug_)
    std::cout << "Checking daughters of ::" << pvol->GetName() << std::endl;

  for (G4int i = 0; i < lvol->GetNoDaughters(); i++) {
    G4VPhysicalVolume* fDaughterPhysVol = lvol->GetDaughter(i);

    if (debug_) {
      std::cout << "name::" << fDaughterPhysVol->GetName() << std::endl;
      std::cout << "pos::" << fDaughterPhysVol->GetTranslation() << std::endl;
      std::cout << "n_dau::"
                << fDaughterPhysVol->GetLogicalVolume()->GetNoDaughters()
                << std::endl;
      std::cout << "replica::" << fDaughterPhysVol->IsReplicated() << std::endl;
      std::cout << "copyNR::" << fDaughterPhysVol->GetCopyNo() << std::endl;
    }
  }
}

// Retrieve the layers from a physical volume
// void BaseTrackingGeometry::getComponentLayer(G4VPhysicalVolume* pvol,
//                                              std::string layer_name,
//                                              std::string component_type,
//                                              std::vector<std::reference_wrapper<G4PhysicalVolume>>
//                                              & components) {

//  G4LogicalVolume* l_vol = pvol->GetLogicalVolume();
//  for (G4int i=0; i<l_vol->GetNoDaughters(); i++) {

//  }

//}

void BaseTrackingGeometry::dumpGeometry(const std::string& outputDir) {
  if (!tGeometry_) return;

  // Should fail if already exists
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
      objVis, *(tGeometry_->highestTrackingVolume()), *gctx_, containerView,
      volumeView, passiveView, sensitiveView, gridView, true, "", ".");
}

// This method gets the transform from the physical volume to the tracking frame
Acts::Transform3 BaseTrackingGeometry::GetTransform(
    const G4VPhysicalVolume& phex, bool toTrackingFrame) {
  Acts::Vector3 pos(phex.GetTranslation().x(), phex.GetTranslation().y(),
                    phex.GetTranslation().z());

  if (toTrackingFrame) {
    pos(0) = phex.GetTranslation().z();
    pos(1) = phex.GetTranslation().x();
    pos(2) = phex.GetTranslation().y();
  }

  Acts::RotationMatrix3 rotation;

  ConvertG4Rot(phex.GetObjectRotationValue(), rotation);

  // rotate to the tracking frame
  if (toTrackingFrame) rotation = x_rot_ * y_rot_ * rotation;

  Acts::Translation3 translation(pos);
  Acts::Transform3 transform(translation * rotation);

  return transform;
}

// This method returns the transformation to the tracker coordinates z->x x->y
// y->z
Acts::Transform3 BaseTrackingGeometry::toTracker(
    const Acts::Transform3& trans) {
  Acts::Vector3 pos{trans.translation()(2), trans.translation()(0),
                    trans.translation()(1)};

  Acts::RotationMatrix3 rotation = trans.rotation();
  rotation = x_rot_ * y_rot_ * rotation;

  Acts::Translation3 translation(pos);
  Acts::Transform3 transform(translation * rotation);

  return transform;
}

// Convert rotation
void BaseTrackingGeometry::ConvertG4Rot(const G4RotationMatrix& g4rot,
                                        Acts::RotationMatrix3& rot) {
  rot(0, 0) = g4rot.xx();
  rot(0, 1) = g4rot.xy();
  rot(0, 2) = g4rot.xz();

  rot(1, 0) = g4rot.yx();
  rot(1, 1) = g4rot.yy();
  rot(1, 2) = g4rot.yz();

  rot(2, 0) = g4rot.zx();
  rot(2, 1) = g4rot.zy();
  rot(2, 2) = g4rot.zz();
}

// Convert translation

Acts::Vector3 BaseTrackingGeometry::ConvertG4Pos(const G4ThreeVector& g4pos) {
  Acts::Vector3 trans{g4pos.x(), g4pos.y(), g4pos.z()};

  if (debug_) {
    std::cout << std::endl;
    std::cout << "g4pos::" << g4pos << std::endl;
    std::cout << trans << std::endl;
  }

  return trans;
}

void BaseTrackingGeometry::getSurfaces(
    std::vector<const Acts::Surface*>& surfaces) {
  if (!tGeometry_)
    throw std::runtime_error(
        "BaseTrackingGeometry::getSurfaces tGeometry is null");

  const Acts::TrackingVolume* tVolume = tGeometry_->highestTrackingVolume();
  if (tVolume->confinedVolumes()) {
    for (auto volume : tVolume->confinedVolumes()->arrayObjects()) {
      if (volume->confinedLayers()) {
        for (const auto& layer : volume->confinedLayers()->arrayObjects()) {
          if (layer->layerType() == Acts::navigation) continue;
          for (auto surface : layer->surfaceArray()->surfaces()) {
            if (surface) {
              surfaces.push_back(surface);

            }  // surface exists
          }    // surfaces
        }      // layers objects
      }        // confined layers
    }          // volumes objects
  }            // confined volumes
}

void BaseTrackingGeometry::makeLayerSurfacesMap() {
  std::vector<const Acts::Surface*> surfaces;
  getSurfaces(surfaces);

  for (auto& surface : surfaces) {
    // std::cout<<"Check the surfaces"<<std::endl;
    // surface->toStream(gctx_,std::cout);
    // std::cout<<"GeometryID::"<<surface->geometryId()<<std::endl;
    // std::cout<<"GeometryID::"<<surface->geometryId().value()<<std::endl;

    // Layers from 1 to 14 - for the tagger
    // unsigned int layerId = (surface->geometryId().layer() / 2) ;  // Old 1
    // sensor per layer

    unsigned int volumeId = surface->geometryId().volume();
    unsigned int layerId = (surface->geometryId().layer() /
                            2);  // set layer ID  from 1 to 7 for the tagger and
                                 // from 1 to 6 for the recoil
    unsigned int sensorId =
        surface->geometryId().sensitive() -
        1;  // set sensor ID from 0 to 1 for the tagger and from 0 to 9 for the
            // axial sensors in the back layers of the recoil

    // surface ID = vol * 1000 + ly * 100 + sensor

    unsigned int surfaceId = volumeId * 1000 + layerId * 100 + sensorId;

    layer_surface_map_[surfaceId] = surface;

  }  // surfaces loop

  if (debug_) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;

    for (auto const& surfaceId : layer_surface_map_) {
      std::cout << " " << surfaceId.first << std::endl;
      std::cout << " Check the surface" << std::endl;
      surfaceId.second->toStream(*gctx_, std::cout);
      std::cout << " GeometryID::" << surfaceId.second->geometryId()
                << std::endl;
      std::cout << " GeometryID::" << surfaceId.second->geometryId().value()
                << std::endl;
    }
  }
}

}  // namespace reco
}  // namespace tracking
