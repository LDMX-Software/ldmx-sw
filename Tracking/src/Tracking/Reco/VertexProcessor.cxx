#include "Tracking/Reco/VertexProcessor.h"

#include <chrono>

#include "Acts/MagneticField/ConstantBField.hpp"

using namespace framework;

namespace tracking {
namespace reco {

VertexProcessor::VertexProcessor(const std::string &name,
                                 framework::Process &process)
    : framework::Producer(name, process) {}

void VertexProcessor::onProcessStart() {
  gctx_ = Acts::GeometryContext();
  bctx_ = Acts::MagneticFieldContext();

  h_m_ = new TH1F("m", "m", 100, 0., 1.);
  h_m_truth_filter_ = new TH1F("m_filter", "m", 100, 0., 1.);
  h_m_truth_ = new TH1F("m_truth", "m_truth", 100, 0., 1.);

  /*
   * this is unused, should it be? FIXME
  auto localToGlobalBin_xyz = [](std::array<size_t, 3> bins,
                                 std::array<size_t, 3> sizes) {
    return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] +
            bins[2]);  // xyz - field space
    // return (bins[1] * (sizes[2] * sizes[0]) + bins[2] * sizes[0] + bins[0]);
    // //zxy
  };
  */

  // Setup a interpolated bfield map
  sp_interpolated_b_field_ =
      std::make_shared<InterpolatedMagneticField3>(loadDefaultBField(
          field_map_, defaultTransformPos, defaultTransformBField));

  ldmx_log(info) << "Check if nullptr::" << sp_interpolated_b_field_.get();
}

void VertexProcessor::configure(framework::config::Parameters &parameters) {
  // TODO:: the bfield map should be taken automatically
  field_map_ = parameters.get<std::string>("field_map");

  trk_coll_name_ = parameters.get<std::string>("trk_coll_name", "Tracks");

  input_pass_name_ = parameters.get<std::string>("input_pass_name");
}

void VertexProcessor::produce(framework::Event &event) {
  // TODO:: Move this to an external file
  // And move all this to a single time per processor not for each event!!

  nevents_++;
  auto start = std::chrono::high_resolution_clock::now();
  auto &&stepper = Acts::EigenStepper<>{sp_interpolated_b_field_};

  // Set up propagator with void navigator
  propagator_ = std::make_shared<VoidPropagator>(stepper);

  // Track linearizer in the proximity of the vertex location
  using Linearizer = Acts::HelicalTrackLinearizer;
  Linearizer::Config linearizer_config;
  linearizer_config.bField = sp_interpolated_b_field_;
  linearizer_config.propagator = propagator_;
  Linearizer linearizer(linearizer_config);

  // Set up Billoir Vertex Fitter
  using VertexFitter = Acts::FullBilloirVertexFitter;

  VertexFitter::Config vertex_fitter_cfg;

  VertexFitter billoir_fitter(vertex_fitter_cfg);

  //  VertexFitter::State state(sp_interpolated_bField_->makeCache(bctx_));

  // Unconstrained fit
  // See
  // https://github.com/acts-project/acts/blob/main/Tests/UnitTests/Core/Vertexing/FullBilloirVertexFitterTests.cpp#L149
  // For constraint implementation

  Acts::VertexingOptions vf_options(gctx_, bctx_);

  // Retrieve the track collection
  const std::vector<ldmx::Track> tracks =
      event.getCollection<ldmx::Track>(trk_coll_name_, input_pass_name_);

  // Retrieve the truth seeds
  const std::vector<ldmx::Track> seeds =
      event.getCollection<ldmx::Track>("RecoilTruthSeeds", input_pass_name_);

  if (tracks.size() < 1) return;

  // Transform the EDM ldmx::tracks to the format needed by ACTS
  std::vector<Acts::BoundTrackParameters> billoir_tracks;

  // TODO:: The perigee surface should be common between all tracks.
  // So should only be created once in principle.
  // There should be no perigeeSurface2

  Acts::Vector3 perigee_acts = tracking::sim::utils::ldmx2Acts(Acts::Vector3(
      tracks.front().getPerigeeX(), tracks.front().getPerigeeY(),
      tracks.front().getPerigeeZ()));
  std::shared_ptr<Acts::PerigeeSurface> perigee_surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(perigee_acts);

  for (unsigned int i_track = 0; i_track < tracks.size(); i_track++) {
    Acts::BoundVector param_vec;
    param_vec << tracks.at(i_track).getD0(), tracks.at(i_track).getZ0(),
        tracks.at(i_track).getPhi(), tracks.at(i_track).getTheta(),
        tracks.at(i_track).getQoP(), tracks.at(i_track).getT();

    Acts::BoundSquareMatrix cov_mat =
        tracking::sim::utils::unpackCov(tracks.at(i_track).getPerigeeCov());
    auto part{Acts::GenericParticleHypothesis(Acts::ParticleHypothesis(
        Acts::PdgParticle(tracks.at(i_track).getPdgID())))};
    billoir_tracks.push_back(Acts::BoundTrackParameters(
        perigee_surface, param_vec, std::move(cov_mat), part));
  }

  // Select exactly 2 tracks
  if (billoir_tracks.size() != 2) {
    return;
  }

  if (billoir_tracks.at(0).charge() * billoir_tracks.at(1).charge() > 0) return;

  // Pion mass hypothesis
  double pion_mass = 139.570 * Acts::UnitConstants::MeV;

  TLorentzVector p1, p2;
  p1.SetXYZM(billoir_tracks.at(0).momentum()(0),
             billoir_tracks.at(0).momentum()(1),
             billoir_tracks.at(0).momentum()(2), pion_mass);

  p2.SetXYZM(billoir_tracks.at(1).momentum()(0),
             billoir_tracks.at(1).momentum()(1),
             billoir_tracks.at(1).momentum()(2), pion_mass);

  std::vector<TLorentzVector> pion_seeds;

  if (seeds.size() == 2) {
    for (int i_seed = 0; i_seed < seeds.size(); i_seed++) {
      Acts::Vector3 seed_perigee_acts = tracking::sim::utils::ldmx2Acts(
          Acts::Vector3(seeds.at(i_seed).getPerigeeX(),
                        seeds.at(i_seed).getPerigeeY(),
                        seeds.at(i_seed).getPerigeeZ()));
      std::shared_ptr<Acts::PerigeeSurface> perigee_surface2 =
          Acts::Surface::makeShared<Acts::PerigeeSurface>(seed_perigee_acts);

      Acts::BoundVector param_vec;
      param_vec << seeds.at(i_seed).getD0(), seeds.at(i_seed).getZ0(),
          seeds.at(i_seed).getPhi(), seeds.at(i_seed).getTheta(),
          seeds.at(i_seed).getQoP(), seeds.at(i_seed).getT();

      Acts::BoundSquareMatrix cov_mat =
          tracking::sim::utils::unpackCov(seeds.at(i_seed).getPerigeeCov());
      int pion_pdg_id = 211;  // pi+
      if (seeds.at(i_seed).getCharge() < 0) pion_pdg_id = -211;
      // BoundTrackParameters needs the particle hypothesis
      auto part{Acts::GenericParticleHypothesis(
          Acts::ParticleHypothesis(Acts::PdgParticle(pion_pdg_id)))};
      auto bound_seed_params = Acts::BoundTrackParameters(
          perigee_surface, param_vec, std::move(cov_mat), part);

      TLorentzVector pion4v;
      pion4v.SetXYZM(bound_seed_params.momentum()(0),
                     bound_seed_params.momentum()(1),
                     bound_seed_params.momentum()(2), pion_mass);

      pion_seeds.push_back(pion4v);
    }  // loops on seeds

    h_m_truth_->Fill((pion_seeds.at(0) + pion_seeds.at(1)).M());
  }

  if ((pion_seeds.size() == 2) &&
      (pion_seeds.at(0) + pion_seeds.at(1)).M() > 0.490 &&
      (pion_seeds.at(0) + pion_seeds.at(1)).M() < 0.510) {
    // Check if the tracks have opposite charge
    h_m_truth_filter_->Fill((p1 + p2).M());
  }

  h_m_->Fill((p1 + p2).M());

  auto end = std::chrono::high_resolution_clock::now();
  // long long microseconds =
  // std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
}

void VertexProcessor::onProcessEnd() {
  TFile *outfile = new TFile("VertexingResults.root", "RECREATE");
  outfile->cd();

  h_m_->Write();
  h_m_truth_->Write();
  h_m_truth_filter_->Write();
  outfile->Close();
  delete outfile;

  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(3)
                 << processing_time_ / nevents_ << " ms";
}

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::VertexProcessor)
