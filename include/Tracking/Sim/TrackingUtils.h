#ifndef TRACKUTILS_H_
#define TRACKUTILS_H_

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Sim/LdmxSpacePoint.h"

namespace tracking{
namespace sim{
namespace utils {
      
//This method converts a SimHit in a LdmxSpacePoint for the Acts seeder.
// (1) Rotate the coordinates into acts::seedFinder coordinates defined by B-Field along z axis [Z_ldmx -> X_acts, X_ldmx->Y_acts, Y_ldmx->Z_acts]
// (2) Saves the error information. At the moment the errors are fixed. They should be obtained from the digitized hits.
      
//TODO::Move to shared pointers?!
//TODO::Pass to instances?
inline ldmx::LdmxSpacePoint* convertSimHitToLdmxSpacePoint(const ldmx::SimTrackerHit& hit) {
        
  std::vector<float> sim_hit_pos = hit.getPosition();
        
        
  //This is in the transverse plane
  float sigma_rphi = 0.25;  //250um
        
  //This is in the direction along the b-field
  float sigma_z    = 0.50;  //50 um
        
  float ldmxsp_x = sim_hit_pos[2];
  float ldmxsp_y = sim_hit_pos[0];
  float ldmxsp_z = sim_hit_pos[1];
        
        
  return new ldmx::LdmxSpacePoint(ldmxsp_x, ldmxsp_y,ldmxsp_z,
                                  hit.getTime(), hit.getLayerID(), hit.getEdep(), 
                                  sigma_rphi*sigma_rphi, sigma_z*sigma_z,
                                  hit.getID());
        
}
}
}
}
      
      
#endif
