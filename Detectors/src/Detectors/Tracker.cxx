
#include "XML/Helper.h"

using namespace dd4hep;

static Ref_t create_tracker(Detector &lcdd, xml::Handle_t xml_handle,
                            SensitiveDetector sens_det) {

  // Detector xml handle
  xml::DetElement detector_handle = xml_handle;

  // Get the dimensions of the tracker envelope and construct a box volume made
  // out of air.  This volume will be used to contain the tracker.
  auto envelope_dims{detector_handle.dimensions()};
  Box envelope{envelope_dims.x(), envelope_dims.y(), envelope_dims.z()}; 
  Volume envelope_vol(detector_handle.nameStr() + "_envelope", envelope, lcdd.air()); 

  // Create a new detector element
  DetElement tracker(detector_handle.nameStr(), detector_handle.id());

  // Place the evelope at the origin within the mother volume.
  auto mother_vol{lcdd.pickMotherVolume(tracker).placeVolume(envelope_volume, Position(0, 0, 0))}; 
  tracker.setPlacement(mother_vol); 

  return tracker;
}

DECLARE_DETELEMENT(Tracker, create_tracker)
