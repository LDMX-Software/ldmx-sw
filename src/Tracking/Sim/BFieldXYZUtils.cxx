#include "Tracking/Sim/BFieldXYZUtils.h"

Acts::Vector3 default_transformPos(const Acts::Vector3& pos) {
  
  Acts::Vector3 rot_pos;
  rot_pos(0)=pos(1);
  rot_pos(1)=pos(2);
  rot_pos(2)=pos(0) + 400.;

  //std::cout<<"PF::DEFAULT TRANSFORM"<<std::endl;
  //std::cout<<"PF::Check:: transforming Pos"<<std::endl;
  //std::cout<<pos<<std::endl;
  //std::cout<<"TO"<<std::endl;
  //std::cout<<rot_pos<<std::endl;
  
  return rot_pos;
}

Acts::Vector3 default_transformBField(const Acts::Vector3& field,
                                  const Acts::Vector3& /*pos*/) {
  
  
  Acts::Vector3 rot_field;
  
  rot_field(0) = field(2);
  rot_field(1) = field(0);
  rot_field(2) = field(1);

  //std::cout<<"PF::DEFAULT TRANSFORM"<<std::endl;
  //std::cout<<"PF::Check:: transforming"<<std::endl;
  //std::cout<<field<<std::endl;
  //std::cout<<"TO"<<std::endl;
  //std::cout<<rot_field<<std::endl;
  
  return rot_field;
}

size_t localToGlobalBin_xyz(std::array<size_t, 3> bins,
                               std::array<size_t, 3> sizes) {
  return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] +
          bins[2]);  // xyz - field space
  // return (bins[1] * (sizes[2] * sizes[0]) + bins[2] * sizes[0] + bins[0]);
  // //zxy
}
