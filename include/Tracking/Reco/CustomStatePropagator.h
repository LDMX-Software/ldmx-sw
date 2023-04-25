#pragma once

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"


//--- ACTS ---//
#include "Acts/Definitions/Units.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"



//--- Tracking ---//
#include "Tracking/Sim/BFieldXYZUtils.h"
#include "Tracking/Sim/TrackingUtils.h"

//--- C++ ---//
#include <random>

//--- ROOT ---//
#include "TFile.h"
#include "TTree.h"


using AbortList = Acts::AbortList<Acts::EndOfWorldReached>;

namespace tracking::reco{
  
  class CustomStatePropagator : public framework::Producer {
 public:
    CustomStatePropagator(const std::string& name, framework::Process& process);
    ~CustomStatePropagator();
    
    void onProcessStart() final override;
    void onProcessEnd() final override;
    
    void configure(framework::config::Parameters& parameters) final override;

    void produce(framework::Event& event) {};

    void fillTree(int state,
                  int q,
                  const Acts::Vector3 gen_pos,
                  const Acts::Vector3 gen_mom,
                  const Acts::BoundTrackParameters* endParams);
    
    Acts::GeometryContext gctx_;
    Acts::MagneticFieldContext bctx_;

    //The interpolated bfield
    std::string field_map_{""};
    double surf_location_{0.};
    int nstates_{0};
    std::vector<double> bs_size_;
    std::vector<double> prange_;
    std::vector<double> thetarange_;
    std::vector<double> phirange_;
    
    //Output ntuple
    TFile* outFile_;
    TTree* outTree_;
    
    
    double state_nr{0.};
    int charge{0};
    double gen_x{0.};
    double gen_y{0.};
    double gen_z{0.};
    double gen_px{0.};
    double gen_py{0.};
    double gen_pz{0.};
    
    double end_x{0.};
    double end_y{0.};
    double end_z{0.};
    double end_loc0{0.};
    double end_loc1{0.};
    
    double end_px{0.};
    double end_py{0.};
    double end_pz{0.};

  };
  
}//tracking::reco
