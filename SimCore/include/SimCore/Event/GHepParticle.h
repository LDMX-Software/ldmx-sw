////////////////////////////////////////////////////////////////////////
/// \file  GHepParticle.h
/// \Class to hold the particle information needed to recreate a genie::EventRecord
/// \author  wketchum@fnal.gov
///
/// See https://github.com/GENIE-MC/Generator/blob/master/src/Framework/GHEP/GHepParticle.h
///
/// Adopted for LDMX-SW by wketchum@fnal.gov
////////////////////////////////////////////////////////////////////////

#ifndef SIMCORE_EVENT_GHEPPARTICLE_H
#define SIMCORE_EVENT_GHEPPARTICLE_H

#include "TObject.h"
#include <iostream>

namespace ldmx {

  class GHepParticle {

  public:

    GHepParticle() {}
    
    void Clear() {}
    void Print() const;

    int fPosition;
    
    int fPdgCode;
    int fStatus;
    int fRescatterCode;
    int fFirstMother;
    int fLastMother;
    int fFirstDaugher;
    int fLastDaughter;
    
    double fP_x;
    double fP_y;
    double fP_z;
    double fP_t;

    double fX_x;
    double fX_y;
    double fX_z;
    double fX_t;

    double fPolzTheta;
    double fPolzPhi;

    double fRemovalEnergy;
    bool fIsBound;    

  public:
    
    ClassDef(GHepParticle, 1);

  };

} // end simb namespace

#endif // SIMCORE_EVENT_GTRUTH_H
