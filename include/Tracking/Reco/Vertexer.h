#ifndef TRACKING_RECO_VERTEXER_H_
#define TRACKING_RECO_VERTEXER_H_


//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

// --- Tracking --- //
#include "Tracking/Event/Track.h"
#include "Tracking/Sim/BFieldXYZUtils.h"
#include "Tracking/Sim/TrackingUtils.h"

// --- ACTS --- //

// Utils and definitions

#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"


// Vertexing

#include "Acts/Vertexing/FullBilloirVertexFitter.hpp"
#include "Acts/Vertexing/HelicalTrackLinearizer.hpp"
#include "Acts/Vertexing/Vertex.hpp"

// Magfield

#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"

// Propagator

#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
//#include "Acts/Propagator/Navigator.hpp"
//#include "Acts/Propagator/StandardAborters.hpp"

// Geometry
#include "Acts/Surfaces/PerigeeSurface.hpp"



// Propagator with void navigator
using VoidPropagator = Acts::Propagator<Acts::EigenStepper<>>;

namespace tracking {
namespace reco {
  
class Vertexer : public framework::Producer {
 public:

  Vertexer(const std::string &name, framework::Process &process);

  ~Vertexer();

  void onProcessStart() final override;
  void onProcessEnd() final override;

  void configure(framework::config::Parameters &parameters) final override ;
      
  void produce(framework::Event &event);

  void TaggerRecoilMonitoring(const std::vector<ldmx::Track>& tagger_tracks,
                              const std::vector<ldmx::Track>& recoil_tracks);
    
  
  
 private:
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext bctx_;

  bool debug_{false};
  int nevents_{0};
  int nvertices_{0};
  int nreconstructable_{0};
  std::shared_ptr<InterpolatedMagneticField3> sp_interpolated_bField_;
  std::shared_ptr<Acts::ConstantBField> bField_;  
  std::string bfieldMap_;
  std::string trk_c_name_1{"TaggerTracks"};
  std::string trk_c_name_2{"RecoilTracks"};
  std::shared_ptr<VoidPropagator> propagator_;
  double processing_time_{0.};


  //Monitoring histograms

  TH1F* h_delta_d0;  
  TH1F* h_delta_z0; 
  TH1F* h_delta_p;;
  TH1F* h_delta_phi;
  TH1F* h_delta_theta;
  
  TH2F* h_delta_d0_vs_recoil_p;
  TH2F* h_delta_z0_vs_recoil_p;

  TH2F* h_td0_vs_rd0;
  TH2F* h_tz0_vs_rz0;

  //pT and photon direction
  
  
  
};
    
}// reco
}//tracking

#endif
