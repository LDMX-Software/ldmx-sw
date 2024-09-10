#include "Tracking/Reco/Vertexer.h"

#include <chrono>

#include "TFile.h"
using namespace framework;

// This producer takes in input two track collections and forms all possible
// vertices from those It can be used to match the tagger and recoil tracks at
// the target and form the beamspot

// TODO Move all the performance monitor to another processor

namespace tracking {
namespace reco {

Vertexer::Vertexer(const std::string& name, framework::Process& process)
    : framework::Producer(name, process) {}

void Vertexer::onProcessStart() {
  // Monitoring plots

  double d0min = -2;
  double d0max = 2;
  double z0min = -2;
  double z0max = 2;

  h_delta_d0 = new TH1F("h_delta_d0", "h_delta_d0", 400, d0min, d0max);
  h_delta_z0 = new TH1F("h_delta_z0", "h_delta_z0", 200, z0min, z0max);
  h_delta_p = new TH1F("h_delta_p", "h_delta_p", 200, -1, 4);
  // h_delta_pT_vsP    = new TH2D("h_delta_pT_vs_p","h_delta_pT_v_p",200,)
  h_delta_phi = new TH1F("h_delta_phi", "h_delta_phi", 400, -0.2, 0.2);
  h_delta_theta = new TH1F("h_delta_theta", "h_delta_theta", 200, -0.1, 0.1);

  h_delta_d0_vs_recoil_p =
      new TH2F("h_delta_d0_vs_recoil_p", "h_delta_d0_vs_recoil_p", 200, 0, 5,
               400, -1, 1);
  h_delta_z0_vs_recoil_p =
      new TH2F("h_delta_z0_vs_recoil_p", "h_delta_z0_vs_recoil_p", 200, 0, 5,
               400, -1, 1);

  h_td0_vs_rd0 =
      new TH2F("h_td0_vs_rd0", "h_td0_vs_rd0", 100, -40, 40, 100, -40, 40);
  h_tz0_vs_rz0 =
      new TH2F("h_tz0_vs_rz0", "h_tz0_vs_rz0", 100, -40, 40, 100, -40, 40);

  gctx_ = Acts::GeometryContext();
  bctx_ = Acts::MagneticFieldContext();

  /*
   * This is unused now, should it be?
  auto localToGlobalBin_xyz = [](std::array<size_t, 3> bins,
                                 std::array<size_t, 3> sizes) {
    return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] +
            bins[2]);  // xyz - field space
    // return (bins[1] * (sizes[2] * sizes[0]) + bins[2] * sizes[0] + bins[0]);
    // //zxy
  };
  */

  sp_interpolated_bField_ =
      std::make_shared<InterpolatedMagneticField3>(loadDefaultBField(
          field_map_, default_transformPos, default_transformBField));

  // There is a sign issue between the vertexing and the perigee representation
  Acts::Vector3 b_field(0., 0., -1.5 * Acts::UnitConstants::T);
  bField_ = std::make_shared<Acts::ConstantBField>(b_field);

  std::cout << "Check if nullptr::" << sp_interpolated_bField_.get()
            << std::endl;

  // Set up propagator with void navigator
  // auto&& stepper = Acts::EigenStepper<>{sp_interpolated_bField_};
  // propagator_ = std::make_shared<VoidPropagator>(stepper);

  auto&& stepper_const = Acts::EigenStepper<>{bField_};
  propagator_ = std::make_shared<VoidPropagator>(stepper_const);
}

void Vertexer::configure(framework::config::Parameters& parameters) {
  // TODO:: the bfield map should be taken automatically
  field_map_ = parameters.getParameter<std::string>("field_map");

  trk_c_name_1 =
      parameters.getParameter<std::string>("trk_c_name_1", "TaggerTracks");
  trk_c_name_2 =
      parameters.getParameter<std::string>("trk_c_name_2", "RecoilTracks");
}

void Vertexer::produce(framework::Event& event) {
  nevents_++;
  // auto start = std::chrono::high_resolution_clock::now();

  // Track linearizer in the proximity of the vertex location
  using Linearizer = Acts::HelicalTrackLinearizer<VoidPropagator>;
  // Linearizer::Config linearizerConfig(sp_interpolated_bField_,propagator_);
  Linearizer::Config linearizerConfig(bField_, propagator_);
  Linearizer linearizer(linearizerConfig);

  // Set up Billoir Vertex Fitter
  using VertexFitter =
      Acts::FullBilloirVertexFitter<Acts::BoundTrackParameters, Linearizer>;

  // Alternatively one can use
  // using VertexFitter =
  //  Acts::FullBilloirVertexFitter<tracking::sim::utils::boundTrackParameters,Linearizer>;

  VertexFitter::Config vertexFitterCfg;
  VertexFitter billoirFitter(vertexFitterCfg);

  VertexFitter::State state(sp_interpolated_bField_->makeCache(bctx_));

  // Unconstrained fit
  // See
  // https://github.com/acts-project/acts/blob/main/Tests/UnitTests/Core/Vertexing/FullBilloirVertexFitterTests.cpp#L149
  // For constraint implementation

  Acts::VertexingOptions<Acts::BoundTrackParameters> vfOptions(gctx_, bctx_);

  // Retrive the two track collections

  const std::vector<ldmx::Track> tracks_1 =
      event.getCollection<ldmx::Track>(trk_c_name_1);
  const std::vector<ldmx::Track> tracks_2 =
      event.getCollection<ldmx::Track>(trk_c_name_2);

  ldmx_log(debug) << "Retrieved track collections" << std::endl
                  << "Track 1 size:" << tracks_1.size() << std::endl
                  << "Track 2 size:" << tracks_2.size() << std::endl;

  if (tracks_1.size() < 1 || tracks_2.size() < 1) return;

  std::vector<Acts::BoundTrackParameters> billoir_tracks_1, billoir_tracks_2;

  // TODO:: The perigee surface should be common between all tracks.

  std::shared_ptr<Acts::PerigeeSurface> perigeeSurface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
          tracks_1.front().getPerigeeX(), tracks_1.front().getPerigeeY(),
          tracks_1.front().getPerigeeZ()));

  // Monitoring of tagger and recoil tracks
  TaggerRecoilMonitoring(tracks_1, tracks_2);

  // Start the vertex formation
  // Form a vertex for each combination of tracks found in the same event
  // between the two track collections

  for (auto& trk : tracks_1) {
    billoir_tracks_1.push_back(
        tracking::sim::utils::boundTrackParameters(trk, perigeeSurface));
  }

  for (auto& trk : tracks_2) {
    billoir_tracks_2.push_back(
        tracking::sim::utils::boundTrackParameters(trk, perigeeSurface));
  }

  std::vector<Acts::Vertex<Acts::BoundTrackParameters> > fit_vertices;

  for (auto& b_trk_1 : billoir_tracks_1) {
    std::vector<const Acts::BoundTrackParameters*> fit_tracks_ptr;

    for (auto& b_trk_2 : billoir_tracks_2) {
      fit_tracks_ptr.push_back(&b_trk_1);
      fit_tracks_ptr.push_back(&b_trk_2);

      ldmx_log(debug) << "Calling vertex fitter" << std::endl
                      << "Track 1 parameters" << std::endl
                      << b_trk_1 << std::endl
                      << "Track 2 parameters" << std::endl
                      << b_trk_2 << std::endl;

      //  std::cout << "Perigee Surface" << std::endl;
      //  perigeeSurface->toStream(gctx_, std::cout);
      //  std::cout << std::endl;

    }  // loop on second set of tracks

    nreconstructable_++;
    try {
      // Acts::Vertex<Acts::BoundTrackParameters> fitVtx =
      // billoirFitter.fit(fit_tracks_ptr, linearizer, vfOptions,
      // state).value(); fit_vertices.push_back(fitVtx);
      nvertices_++;

    } catch (...) {
      ldmx_log(warn) << "Vertex fit failed" << std::endl;
    }

  }  // loop on first set

  // Convert the vertices in the ldmx EDM and store them
}

void Vertexer::onProcessEnd() {
  ldmx_log(info) << "Reconstructed " << nvertices_ << " vertices over "
                 << nreconstructable_ << " reconstructable" << std::endl;

  TFile* outfile_ = new TFile((getName() + ".root").c_str(), "RECREATE");
  outfile_->cd();

  h_delta_d0->Write();
  h_delta_z0->Write();
  h_delta_p->Write();
  h_delta_phi->Write();
  h_delta_theta->Write();

  h_delta_d0_vs_recoil_p->Write();
  h_delta_z0_vs_recoil_p->Write();

  h_td0_vs_rd0->Write();
  h_tz0_vs_rz0->Write();

  outfile_->Close();
  delete outfile_;
}

void Vertexer::TaggerRecoilMonitoring(
    const std::vector<ldmx::Track>& tagger_tracks,
    const std::vector<ldmx::Track>& recoil_tracks) {
  // For the moment only check that I have 1 tagger track and one recoil track
  // To avoid trying to match them
  // TODO update this logic

  if (tagger_tracks.size() != 1 || recoil_tracks.size() != 1) return;

  ldmx::Track t_trk = tagger_tracks.at(0);
  ldmx::Track r_trk = recoil_tracks.at(0);

  double t_p, r_p;
  // these are unsed, should they be? FIXME
  // double t_d0, r_d0;
  // double tt_p_phi, r_phi;
  // double t_theta, r_theta;
  // double t_z0, r_z0;

  t_p = t_trk.q() / t_trk.getQoP();
  r_p = r_trk.q() / r_trk.getQoP();

  h_delta_d0->Fill(t_trk.getD0() - r_trk.getD0());
  h_delta_z0->Fill(t_trk.getZ0() - r_trk.getZ0());
  h_delta_p->Fill(t_p - r_p);
  h_delta_phi->Fill(t_trk.getPhi() - r_trk.getPhi());
  h_delta_theta->Fill(t_trk.getTheta() - r_trk.getTheta());

  // differential plots

  h_delta_d0_vs_recoil_p->Fill(r_p, t_trk.getD0() - r_trk.getD0());
  h_delta_z0_vs_recoil_p->Fill(r_p, t_trk.getZ0() - r_trk.getZ0());

  //"beamspot"
  h_td0_vs_rd0->Fill(r_trk.getD0(), t_trk.getD0());
  h_tz0_vs_rz0->Fill(r_trk.getZ0(), t_trk.getZ0());

  //"pT"
  // TODO Transverse momentum should obtained orthogonal to the B-Field
  // direction This assumes to be along Z (which is not very accurate)

  // std::vector<double> r_mom = r_trk.getMomentum();
  // std::vector<double> t_mom = t_trk.getMomentum();

  // I assume to have a single photon being emitted in the target: I use
  // momentum conservation p_photon = p_beam - p_recoil

  // h_gamma_px->Fill(t_mom[0] - r_mom[0]);
  // h_gamma_py->Fill(t_mom[1] - r_mom[1]);
  // h_gamma_pz->Fill(t_mom[2] - r_mom[2]);
}

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, Vertexer)
