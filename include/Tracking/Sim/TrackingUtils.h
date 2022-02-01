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
//Vol==2 for tagger, Vol==3 for recoil

inline ldmx::LdmxSpacePoint* convertSimHitToLdmxSpacePoint(const ldmx::SimTrackerHit& hit, unsigned int vol = 2) {

  bool debug = false;
      
  std::vector<float> sim_hit_pos = hit.getPosition();
          
  //This is in the transverse plane
  float sigma_rphi = 0.25;  //250um
        
  //This is in the direction along the b-field
  float sigma_z    = 0.50;  //50 um
        
  float ldmxsp_x = sim_hit_pos[2];
  float ldmxsp_y = sim_hit_pos[0];
  float ldmxsp_z = sim_hit_pos[1];

  unsigned int sensorId = 0;
  unsigned int layerId  = 0;
  
  //tagger numbering scheme for surfaces mapping
  //Layers from 1 to 14  => transform to 0->13
  if (vol == 2) {
    sensorId = (hit.getLayerID() + 1) % 2; //0,1,0,1 ...
    layerId  = (hit.getLayerID() + 1) / 2; //1,2,3,4,5,6,7    
  }

  //recoil numbering scheme for surfaces mapping 
  if (vol == 3) {
    sensorId = hit.getModuleID();
    layerId  = hit.getLayerID();
  }

  //vol * 1000 + ly * 100 + sensor
  unsigned int index  = vol * 1000 + layerId * 100 + sensorId;

  if (debug) {
    std::cout<<"LdmxSpacePointConverter::Check index::"<<vol<<"--"<<layerId<<"--"<<sensorId<<"==>"<<index<<std::endl;
    std::cout<<vol<<"==="<<hit.getLayerID()<<"==="<<hit.getModuleID()<<std::endl;
    std::cout<<"("<<ldmxsp_x<<","<<ldmxsp_y<<","<<ldmxsp_z<<")"<<std::endl;
  }

  return new ldmx::LdmxSpacePoint(ldmxsp_x, ldmxsp_y,ldmxsp_z,
                                  hit.getTime(), index, hit.getEdep(), 
                                  sigma_rphi*sigma_rphi, sigma_z*sigma_z,
                                  hit.getID());
  
}
}//utils
}//sim
}//tracking
      
      
#endif
