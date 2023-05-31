#include "Tracking/Event/TruthTrack.h"
#include <iostream>

ClassImp(ldmx::TruthTrack)

namespace ldmx {


std::ostream& operator<<(std::ostream& output,
                         const TruthTrack& trk) {
  output<< "[ TruthTrack ]:\n"
        <<"("<<trk.getD0()<<","<<trk.getZ0()<<","<<trk.getPhi()<<","<<trk.getTheta()<<","<<trk.getQoP()<<","<<trk.getT()<<")\n"
        <<"perigee : ["<<trk.perigee_[0]<<","<<trk.perigee_[1]<<","<<trk.perigee_[2]<<"]\n"
        <<"position: ["<<trk.position_[0]<<","<<trk.position_[1]<<","<<trk.position_[2]<<"]\n"
        <<"momentum: ["<<trk.momentum_[0]<<","<<trk.momentum_[1]<<","<<trk.momentum_[2]<<"]\n"
        <<"pdgID:"<<trk.pdgID_<<"  trackID:"<<trk.trackID_
        <<std::endl;

  return output; 
}
}
