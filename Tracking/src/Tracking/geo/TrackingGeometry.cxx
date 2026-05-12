
#include "Tracking/geo/TrackingGeometry.h"

#include "Framework/Exception/Exception.h"

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
  double rotation_angle = M_PI * 0.5;

  // 0 0 -1
  // 0 1 0
  // 1 0 0

  // This rotation is needed to have the plane orthogonal to the X direction.
  //  Rotation of the surfaces
  Acts::Vector3 x_pos1(cos(rotation_angle), 0., sin(rotation_angle));
  Acts::Vector3 y_pos1(0., 1., 0.);
  Acts::Vector3 z_pos1(-sin(rotation_angle), 0., cos(rotation_angle));

  y_rot_.col(0) = x_pos1;
  y_rot_.col(1) = y_pos1;
  y_rot_.col(2) = z_pos1;

  // Rotate the sensors to put them in the proper orientation in Z
  Acts::Vector3 x_pos2(1., 0., 0.);
  Acts::Vector3 y_pos2(0., cos(rotation_angle), sin(rotation_angle));
  Acts::Vector3 z_pos2(0., -sin(rotation_angle), cos(rotation_angle));

  x_rot_.col(0) = x_pos2;
  x_rot_.col(1) = y_pos2;
  x_rot_.col(2) = z_pos2;

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

  // Extract field map filename from GDML auxiliary data and resolve full path.
  // GDML path: .../data/detectors/<det>/detector.gdml
  // Field map:  .../data/fieldmap/<filename>
  const G4GDMLAuxListType* aux_list = parser.GetAuxList();
  for (const auto& aux : *aux_list) {
    if (aux.type == "MagneticField") {
      for (const auto& sub : *aux.auxList) {
        if (sub.type == "File") {
          boost::filesystem::path fmap(std::string(sub.value));
          if (fmap.is_absolute()) {
            field_map_file_ = fmap.string();
          } else {
            boost::filesystem::path prefix = boost::filesystem::path(gdml_)
                                                 .parent_path()
                                                 .parent_path()
                                                 .parent_path();
            field_map_file_ = (prefix / "fieldmap" / fmap).string();
          }
          break;
        }
      }
      break;
    }
  }

  if (field_map_file_.empty()) {
    EXCEPTION_RAISE("BadGeometry",
                    "TrackingGeometry: no MagneticField/File auxiliary entry "
                    "found in '" + gdml_ + "'");
  }

  f_world_phys_vol_ = parser.GetWorldVolume();

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
    G4VPhysicalVolume* f_daughter_phys_vol = lvol->GetDaughter(i);
    std::string d_name = f_daughter_phys_vol->GetName();
    if (d_name.find(name) != std::string::npos) return f_daughter_phys_vol;
    // if (fDaughterPhysVol->GetName() == name) return fDaughterPhysVol;
  }

  return nullptr;
}

void TrackingGeometry::getAllDaughters(G4VPhysicalVolume* pvol) {
  G4LogicalVolume* lvol = pvol->GetLogicalVolume();

  ldmx_log(trace) << "Checking daughters of ::" << pvol->GetName();

  for (G4int i = 0; i < lvol->GetNoDaughters(); i++) {
    G4VPhysicalVolume* f_daughter_phys_vol = lvol->GetDaughter(i);

    ldmx_log(trace) << "name::" << f_daughter_phys_vol->GetName();
    ldmx_log(trace) << "pos_::" << f_daughter_phys_vol->GetTranslation();
    ldmx_log(trace)
        << "n_dau::"
        << f_daughter_phys_vol->GetLogicalVolume()->GetNoDaughters();
    ldmx_log(trace) << "replica::" << f_daughter_phys_vol->IsReplicated();
    ldmx_log(trace) << "copyNR::" << f_daughter_phys_vol->GetCopyNo();

    getAllDaughters(f_daughter_phys_vol);
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
  if (!t_geometry_) return;

  {
    ldmx_log(trace) << __PRETTY_FUNCTION__;

    for (auto const& surface_id : layer_surface_map_) {
      ldmx_log(trace) << " " << surface_id.first;
      ldmx_log(trace) << " Check the surface";
      //      surfaceId.second->toStream(gctx, ldmx_log(trace));
      surface_id.second->toStream(gctx);
      ldmx_log(trace) << " GeometryID: " << surface_id.second->geometryId();
      ldmx_log(trace) << " GeometryID value: "
                      << surface_id.second->geometryId().value();
    }
  }

  // Should fail if already exists
  boost::filesystem::create_directory(outputDir);

  double output_scalor = 1.0;
  size_t output_precision = 6;

  Acts::ObjVisualization3D obj_vis(output_precision, output_scalor);
  Acts::ViewConfig container_view = Acts::ViewConfig({220, 220, 220});
  Acts::ViewConfig volume_view = Acts::ViewConfig({220, 220, 0});
  Acts::ViewConfig sensitive_view = Acts::ViewConfig({0, 180, 240});
  Acts::ViewConfig passive_view = Acts::ViewConfig({240, 280, 0});
  Acts::ViewConfig grid_view = Acts::ViewConfig({220, 0, 0});

  Acts::GeometryView3D::drawTrackingVolume(
      obj_vis, *(t_geometry_->highestTrackingVolume()), gctx, container_view,
      volume_view, passive_view, sensitive_view, grid_view, true, "", ".");
}

// This method gets the transform from the physical volume to the tracking frame
Acts::Transform3 TrackingGeometry::getTransform(const G4VPhysicalVolume& phex,
                                                bool toTrackingFrame) const {
  Acts::Vector3 pos(phex.GetTranslation().x(), phex.GetTranslation().y(),
                    phex.GetTranslation().z());

  Acts::RotationMatrix3 rotation;
  convertG4Rot(phex.GetRotation(), rotation);

  // rotate to the tracking frame
  if (toTrackingFrame) {
    pos(0) = phex.GetTranslation().z();
    pos(1) = phex.GetTranslation().x();
    pos(2) = phex.GetTranslation().y();
    rotation = x_rot_ * y_rot_ * rotation;
  }

  Acts::Translation3 translation(pos);

  Acts::Transform3 transform(translation * rotation);

  return transform;
}

// This method returns the transformation to the tracker coordinates z_->x_
// x_->y_ y_->z_
Acts::Transform3 TrackingGeometry::toTracker(
    const Acts::Transform3& trans) const {
  Acts::Vector3 pos{trans.translation()(2), trans.translation()(0),
                    trans.translation()(1)};

  Acts::RotationMatrix3 rotation = trans.rotation();
  rotation = x_rot_ * y_rot_ * rotation;

  Acts::Translation3 translation(pos);
  Acts::Transform3 transform(translation * rotation);

  return transform;
}

// Convert rotation
void TrackingGeometry::convertG4Rot(const G4RotationMatrix* g4rot,
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

Acts::Vector3 TrackingGeometry::convertG4Pos(const G4ThreeVector& g4pos) const {
  Acts::Vector3 trans{g4pos.x(), g4pos.y(), g4pos.z()};

  {
    ldmx_log(trace) << "g4pos::" << g4pos;
    ldmx_log(trace) << "trans" << trans;
  }

  return trans;
}

void TrackingGeometry::getSurfaces(
    std::vector<const Acts::Surface*>& surfaces) const {
  if (!t_geometry_)
    EXCEPTION_RAISE("BadGeometry",
                    "TrackingGeometry::getSurfaces tGeometry is null");

  const Acts::TrackingVolume* t_volume = t_geometry_->highestTrackingVolume();
  if (t_volume->confinedVolumes()) {
    for (auto volume : t_volume->confinedVolumes()->arrayObjects()) {
      if (volume->confinedLayers()) {
        for (const auto& layer : volume->confinedLayers()->arrayObjects()) {
          if (layer->layerType() == Acts::navigation) continue;
          for (auto surface : layer->surfaceArray()->surfaces()) {
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

    unsigned int volume_id = surface->geometryId().volume();
    unsigned int layer_id = (surface->geometryId().layer() /
                             2);  // set layer_ ID  from 1 to 7 for the tagger
                                  // and from 1 to 6 for the recoil
    unsigned int sensor_id =
        surface->geometryId().sensitive() -
        1;  // set sensor ID from 0 to 1 for the tagger and from 0 to 9 for the
            // axial sensors in the back layers of the recoil

    ldmx_log(trace) << "VolumeID " << volume_id << " LayerId " << layer_id
                    << " sensorId " << sensor_id;

    // surface ID = vol * 1000 + ly * 100 + sensor
    unsigned int surface_id = volume_id * 1000 + layer_id * 100 + sensor_id;

    layer_surface_map_[surface_id] = surface;

  }  // surfaces loop
}

}  // namespace tracking::geo
