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

#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Units.hpp"

// Propagator

#include "Acts/Propagator/Propagator.hpp"
#include "Tracking/EigenStepper.h"
// #include "Acts/Propagator/Navigator.hpp"
// #include "Acts/Propagator/StandardAborters.hpp"

// Vertexing

#include "Acts/Vertexing/FullBilloirVertexFitter.hpp"
#include "Acts/Vertexing/HelicalTrackLinearizer.hpp"
#include "Acts/Vertexing/Vertex.hpp"

// Magfield

#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"

// Geometry
#include "Acts/Surfaces/PerigeeSurface.hpp"

// Propagator with void navigator
#include "Acts/Propagator/VoidNavigator.hpp"
using VoidPropagator = Acts::Propagator<Acts::EigenStepper<>, Acts::VoidNavigator>;

namespace tracking {
namespace reco {

class Vertexer : public framework::Producer {
 public:
  Vertexer(const std::string& name, framework::Process& process);

  virtual ~Vertexer() = default;

  void onProcessStart() override;
  void onProcessEnd() override;

  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

  void taggerRecoilMonitoring(const std::vector<ldmx::Track>& tagger_tracks,
                              const std::vector<ldmx::Track>& recoil_tracks);

 private:
  Acts::MagneticFieldContext bctx_;

  int nevents_{0};
  int nvertices_{0};
  int nreconstructable_{0};
  std::shared_ptr<InterpolatedMagneticField3> sp_interpolated_b_field_;
  std::shared_ptr<Acts::ConstantBField> b_field_;

  /// Path to the magnetic field map.
  std::string field_map_;

  std::string trk_c_name_1_{"TaggerTracks"};
  std::string trk_c_name_2_{"RecoilTracks"};
  std::string input_pass_name_{""};
  std::shared_ptr<VoidPropagator> propagator_;

  // Monitoring histograms
  TH1F* h_delta_d0_;
  TH1F* h_delta_z0_;
  TH1F* h_delta_p_;

  TH1F* h_delta_phi_;
  TH1F* h_delta_theta_;

  TH2F* h_delta_d0_vs_recoil_p_;
  TH2F* h_delta_z0_vs_recoil_p_;

  TH2F* h_td0_vs_rd0_;
  TH2F* h_tz0_vs_rz0_;
};

}  // namespace reco
}  // namespace tracking

#endif
