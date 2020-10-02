
#include "XML/Helper.h"

using namespace dd4hep;

static Ref_t create_tracker(Detector &lcdd, xml::Handle_t xml_handle,
                            SensitiveDetector sens_det) {

  // Detector xml handle
  xml::DetElement det_handle = xml_handle;

  // Get the dimensions of the tracker envelope and construct a box shape made
  // out of air.  This volume will be used to contain the tracker.
  auto env_dims(det_handle.dimensions());
  Box env_box(env_dims.dx(), env_dims.dy(), env_dims.dz());
  Volume env_vol(det_handle.nameStr() + "_envelope", env_box, lcdd.air());
  // Set the attributes of the envelope
  env_vol.setAttributes(lcdd, det_handle.regionStr(), det_handle.limitsStr(),
                        det_handle.visStr());

  // The placed volume
  PlacedVolume pv;

  // Loop over all of the modules and create the sensor volumes.
  for (xml::Collection_t imodule(det_handle, _U(module)); imodule; ++imodule) {
    xml::Component xml_module(imodule);

    // Start by creating an assembly for the layers. An assembly will act as
    // bounding box for the two silicon layers it encloses.
    Assembly module_assembly("module_" + std::to_string(xml_module.id()) +
                             "_assembly");

    // Build up the layers inside of the assembly
    for (xml::Collection_t ilayer(xml_module, _U(layer)); ilayer; ++ilayer) {

      xml::Component xml_layer(ilayer);

      // Create the box shape representing the sensor.  If a box can't be
      // created, throw an exception.
      Box sensor_box{xml_layer.createShape()};
      if (!sensor_box.isValid()) {
        // throw an exception
        std::cout << "Cannot crate box volume." << std::endl;
      }

      // Create a volume out of the box and set the material it's made from.
      auto sensor_mat{lcdd.material(xml_layer.materialStr())};
      auto position{xml_layer.position()};
      Volume layer_vol("module_" + std::to_string(xml_module.id()) + "_layer_" +
                           std::to_string(xml_layer.id()),
                       sensor_box, sensor_mat);

      // Rotate the sensor if a rotation was specified.
      RotationZYX rotation;
      if (xml_layer.hasChild(_U(rotation))) {
        rotation =
            RotationZYX(xml_layer.rotation().x(), xml_layer.rotation().y(),
                        xml_layer.rotation().z());
      } else
        rotation = RotationZYX(0., 0., 0.);

      // Place the sensor inside of the module assembly
      pv = module_assembly.placeVolume(
          layer_vol, Transform3D(rotation, Position(position.x(), position.y(),
                                                    position.z())));
    }

    // Get the position of the module and place it inside of the tracker
    // envelope.
    auto module_position{xml_module.position()};
    pv = env_vol.placeVolume(module_assembly,
                             Position(module_position.x(), module_position.y(),
                                      module_position.z()));
  }

  // Create the tracker detector element
  DetElement tracker(det_handle.nameStr(), det_handle.id());

  // Get the global position of the tracker envelope and place it in the mother
  // volume.
  auto env_pos{det_handle.position()};
  auto env_placed_vol(lcdd.pickMotherVolume(tracker).placeVolume(
      env_vol, Position(env_pos.x(), env_pos.y(), env_pos.z())));
  tracker.setPlacement(env_placed_vol);

  return tracker;
}

DECLARE_DETELEMENT(Tracker, create_tracker)
