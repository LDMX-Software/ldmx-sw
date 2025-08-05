
#include "Tracking/geo/TrackingGeometry.h"

namespace tracking::geo {

/**
 * This class throws away all of the messages from Geant4
 *
 * Copied from SimCore/include/SimCore/G4Session.h
 */
class SilentG4 : public G4UIsession {
 public:
  SilentG4() = default;
  ~SilentG4() = default;
  G4UIsession* SessionStart() { return nullptr; }
  G4int ReceiveG4cout(const G4String&) { return 0; }
  G4int ReceiveG4cerr(const G4String&) { return 0; }
};

TrackingGeometry::TrackingGeometry(const std::string& name,
                                   const Acts::GeometryContext& gctx,
                                   const std::string& gdml)
    : framework::ConditionsObject(name), gctx_{gctx}, gdml_{gdml} {
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

  /**
   * We are about to use the G4GDMLParser and would like to silence
   * the output from parsing the geometry. This can only be done by
   * redirecting G4cout and G4cerr via the G4UImanager.
   *
   * The Simulator (if it is running) will already do this redirection
   * for us and we don't want to override it, so we check if there is
   * a simulation running by seeing if the run manager is created. If
   * it isn't, then we redirect G4cout and G4cerr to a G4Session that
   * just throws away all those messages.
   */
  std::unique_ptr<SilentG4> silence;
  if (G4RunManager::GetRunManager() == nullptr) {
    // no run manager ==> no simulation
    silence = std::make_unique<SilentG4>();
    // these lines compied from G4UImanager::SetCoutDestination
    // to avoid creating G4UImanager unnecessarily
    G4coutbuf.SetDestination(silence.get());
    G4cerrbuf.SetDestination(silence.get());
  }

  // Get the world volume
  G4GDMLParser parser;

  // Validation requires internet
  parser.Read(gdml_, false);

  fWorldPhysVol_ = parser.GetWorldVolume();

  if (silence) {
    // we created the session and silenced G4
    // undo that now incase others have use for G4
    // nullptr => standard (ldmx_log(trace) and std::cerr)
    G4coutbuf.SetDestination(nullptr);
    G4cerrbuf.SetDestination(nullptr);
  }
}

G4VPhysicalVolume* TrackingGeometry::findDaughterByName(G4VPhysicalVolume* pvol,
                                                        G4String name) {
  G4LogicalVolume* lvol = pvol->GetLogicalVolume();
  for (G4int i = 0; i < lvol->GetNoDaughters(); i++) {
    G4VPhysicalVolume* fDaughterPhysVol = lvol->GetDaughter(i);
    std::string dName = fDaughterPhysVol->GetName();
    if (dName.find(name) != std::string::npos) return fDaughterPhysVol;
    // if (fDaughterPhysVol->GetName() == name) return fDaughterPhysVol;
  }

  return nullptr;
}

void TrackingGeometry::getAllDaughters(G4VPhysicalVolume* pvol) {
  G4LogicalVolume* lvol = pvol->GetLogicalVolume();

  ldmx_log(trace) << "Checking daughters of ::" << pvol->GetName();

  for (G4int i = 0; i < lvol->GetNoDaughters(); i++) {
    G4VPhysicalVolume* fDaughterPhysVol = lvol->GetDaughter(i);

    ldmx_log(trace) << "name::" << fDaughterPhysVol->GetName();
    ldmx_log(trace) << "pos_::" << fDaughterPhysVol->GetTranslation();
    ldmx_log(trace) << "n_dau::"
                    << fDaughterPhysVol->GetLogicalVolume()->GetNoDaughters();
    ldmx_log(trace) << "replica::" << fDaughterPhysVol->IsReplicated();
    ldmx_log(trace) << "copyNR::" << fDaughterPhysVol->GetCopyNo();

    getAllDaughters(fDaughterPhysVol);
  }
}

// Retrieve the layers from a physical volume
// void TrackingGeometry::getComponentLayer(G4VPhysicalVolume* pvol,
//                                              std::string layer_name,
//                                              std::string component_type,
//                                              std::vector<std::reference_wrapper<G4PhysicalVolume>>
//                                              & components) {

//  G4LogicalVolume* l_vol = pvol->GetLogicalVolume();
//  for (G4int i=0; i<l_vol->GetNoDaughters(); i++) {

//  }

//}

void TrackingGeometry::dumpGeometry(const std::string& outputDir,
                                    const Acts::GeometryContext& gctx) const {
  if (!tGeometry_) return;

  {
    ldmx_log(trace) << __PRETTY_FUNCTION__;

    for (auto const& surfaceId : layer_surface_map_) {
      ldmx_log(trace) << " " << surfaceId.first;
      ldmx_log(trace) << " Check the surface";
      //      surfaceId.second->toStream(gctx, ldmx_log(trace));
      surfaceId.second->toStream(gctx);
      ldmx_log(trace) << " GeometryID: " << surfaceId.second->geometryId();
      ldmx_log(trace) << " GeometryID value: "
                      << surfaceId.second->geometryId().value();
    }
  }

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
      objVis, *(tGeometry_->highestTrackingVolume()), gctx, containerView,
      volumeView, passiveView, sensitiveView, gridView, true, "", ".");
}

// This method gets the transform from the physical volume to the tracking frame
Acts::Transform3 TrackingGeometry::GetTransform(const G4VPhysicalVolume& phex,
                                                bool toTrackingFrame) const {
  Acts::Vector3 pos_(phex.GetTranslation().x(), phex.GetTranslation().y(),
                    phex.GetTranslation().z());

  Acts::RotationMatrix3 rotation;
  ConvertG4Rot(phex.GetRotation(), rotation);

  // rotate to the tracking frame
  if (toTrackingFrame) {
    pos_(0) = phex.GetTranslation().z();
    pos_(1) = phex.GetTranslation().x();
    pos_(2) = phex.GetTranslation().y();
    rotation = x_rot_ * y_rot_ * rotation;
  }

  Acts::Translation3 translation(pos_);

  Acts::Transform3 transform(translation * rotation);

  return transform;
}

// This method returns the transformation to the tracker coordinates z_->x_ x_->y_
// y_->z_
Acts::Transform3 TrackingGeometry::toTracker(
    const Acts::Transform3& trans) const {
  Acts::Vector3 pos_{trans.translation()(2), trans.translation()(0),
                    trans.translation()(1)};

  Acts::RotationMatrix3 rotation = trans.rotation();
  rotation = x_rot_ * y_rot_ * rotation;

  Acts::Translation3 translation(pos_);
  Acts::Transform3 transform(translation * rotation);

  return transform;
}

// Convert rotation
void TrackingGeometry::ConvertG4Rot(const G4RotationMatrix* g4rot,
                                    Acts::RotationMatrix3& rot) const {
  // If the rotation is the identity then g4rot will be a null ptr.
  // So then check it and fill rot accordingly

  rot = Acts::RotationMatrix3::Identity();

  if (g4rot) {
    rot(0, 0) = g4rot->xx();
    rot(0, 1) = g4rot->xy();
    rot(0, 2) = g4rot->xz();

    rot(1, 0) = g4rot->yx();
    rot(1, 1) = g4rot->yy();
    rot(1, 2) = g4rot->yz();

    rot(2, 0) = g4rot->zx();
    rot(2, 1) = g4rot->zy();
    rot(2, 2) = g4rot->zz();
  }
}

// Convert translation

Acts::Vector3 TrackingGeometry::ConvertG4Pos(const G4ThreeVector& g4pos) const {
  Acts::Vector3 trans{g4pos.x(), g4pos.y(), g4pos.z()};

  {
    ldmx_log(trace) << "g4pos::" << g4pos;
    ldmx_log(trace) << "trans" << trans;
  }

  return trans;
}

void TrackingGeometry::getSurfaces(
    std::vector<const Acts::Surface*>& surfaces) const {
  if (!tGeometry_)
    throw std::runtime_error("TrackingGeometry::getSurfaces tGeometry is null");

  const Acts::TrackingVolume* tVolume = tGeometry_->highestTrackingVolume();
  if (tVolume->confinedVolumes()) {
    for (auto volume : tVolume->confinedVolumes()->arrayObjects()) {
      if (volume->confinedLayers()) {
        for (const auto& layer_ : volume->confinedLayers()->arrayObjects()) {
          if (layer_->layerType() == Acts::navigation) continue;
          for (auto surface : layer_->surfaceArray()->surfaces()) {
            if (surface) {
              surfaces.push_back(surface);

            }  // surface exists
          }  // surfaces
        }  // layers objects
      }  // confined layers
    }  // volumes objects
  }  // confined volumes
}

void TrackingGeometry::makeLayerSurfacesMap() {
  std::vector<const Acts::Surface*> surfaces;
  getSurfaces(surfaces);

  for (auto& surface : surfaces) {
    // Layers from 1 to 14 - for the tagger
    // unsigned int layerId = (surface->geometryId().layer() / 2) ;  // Old 1
    // sensor per layer_

    unsigned int volumeId = surface->geometryId().volume();
    unsigned int layerId = (surface->geometryId().layer() /
                            2);  // set layer_ ID  from 1 to 7 for the tagger and
                                 // from 1 to 6 for the recoil
    unsigned int sensorId =
        surface->geometryId().sensitive() -
        1;  // set sensor ID from 0 to 1 for the tagger and from 0 to 9 for the
            // axial sensors in the back layers of the recoil

    ldmx_log(trace) << "VolumeID " << volumeId << " LayerId " << layerId
                    << " sensorId " << sensorId;

    // surface ID = vol * 1000 + ly * 100 + sensor
    unsigned int surfaceId = volumeId * 1000 + layerId * 100 + sensorId;

    layer_surface_map_[surfaceId] = surface;

  }  // surfaces loop
}

}  // namespace tracking::geo
