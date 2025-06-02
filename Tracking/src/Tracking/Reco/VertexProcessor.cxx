#include "Tracking/Reco/VertexProcessor.h"
#include "Tracking/Event/Vertex.h"
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
  h_m_truthFilter_ = new TH1F("m_filter", "m", 100, 0., 1.);
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
  sp_interpolated_bField_ =
      std::make_shared<InterpolatedMagneticField3>(loadDefaultBField(
          field_map_, default_transformPos, default_transformBField));

  ldmx_log(info) << "Check if nullptr::" << sp_interpolated_bField_.get();
}

void VertexProcessor::configure(framework::config::Parameters &parameters) {
  // TODO:: the bfield map should be taken automatically
  field_map_ = parameters.getParameter<std::string>("field_map");

  trk_coll_name_ =
      parameters.getParameter<std::string>("trk_coll_name", "Tracks");
  out_vtx_collection_ =
    parameters.getParameter<std::string>("vtx_coll_name", "Vertices");
  input_pass_name_ = parameters.getParameter<std::string>("input_pass_name");
}

void VertexProcessor::produce(framework::Event &event) {
  // TODO:: Move this to an external file
  // And move all this to a single time per processor not for each event!!
  
  nevents_++;
  auto start = std::chrono::high_resolution_clock::now();
  auto &&stepper = Acts::EigenStepper<>{sp_interpolated_bField_};
  
  // Set up propagator with void navigator
  propagator_ = std::make_shared<VoidPropagator>(stepper);

  // Track linearizer in the proximity of the vertex location
  using Linearizer = Acts::HelicalTrackLinearizer;
  Linearizer::Config linearizerConfig;
  linearizerConfig.bField = sp_interpolated_bField_;
  linearizerConfig.propagator = propagator_;
  Linearizer linearizer(linearizerConfig);

  // Set up Billoir Vertex Fitter
  using VertexFitter = Acts::FullBilloirVertexFitter;

  VertexFitter::Config vertexFitterCfg;
  vertexFitterCfg.extractParameters.connect<&Acts::InputTrack::extractParameters>(); 
  vertexFitterCfg.trackLinearizer.connect<&Acts::HelicalTrackLinearizer::linearizeTrack>(&linearizer);
  ldmx_log(info)<<"Making billoirFitter"; 
  VertexFitter billoirFitter(vertexFitterCfg);
  ldmx_log(info)<<"Made fitter"; 
  auto fieldCache =sp_interpolated_bField_->makeCache(bctx_);
  //  VertexFitter::State state(sp_interpolated_bField_->makeCache(bctx_));

  // Unconstrained fit
  // See
  // https://github.com/acts-project/acts/blob/main/Tests/UnitTests/Core/Vertexing/FullBilloirVertexFitterTests.cpp#L149
  // For constraint implementation

  Acts::VertexingOptions vfOptions(gctx_, bctx_);

  // Retrieve the track collection
  const std::vector<ldmx::Track> tracks =
      event.getCollection<ldmx::Track>(trk_coll_name_, input_pass_name_);

  // Retrieve the truth seeds
  const std::vector<ldmx::Track> seeds =
      event.getCollection<ldmx::Track>("RecoilTruthSeeds", input_pass_name_);

  if (tracks.size() < 1) return;

  // Transform the EDM ldmx::tracks to the format needed by ACTS
  //  std::vector<Acts::BoundTrackParameters> billoir_tracks;
  std::vector<Acts::BoundTrackParameters> billoir_tracks;

  // TODO:: The perigee surface should be common between all tracks.
  // So should only be created once in principle.
  // There should be no perigeeSurface2

  std::shared_ptr<Acts::PerigeeSurface> perigeeSurface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
          tracks.front().getPerigeeX(), tracks.front().getPerigeeY(),
          tracks.front().getPerigeeZ()));
  int pionPdgId = 211;  // pi+

  for (unsigned int iTrack = 0; iTrack < tracks.size(); iTrack++) {
    Acts::BoundVector paramVec;
    paramVec << tracks.at(iTrack).getD0(), tracks.at(iTrack).getZ0(),
        tracks.at(iTrack).getPhi(), tracks.at(iTrack).getTheta(),
        tracks.at(iTrack).getQoP(), tracks.at(iTrack).getT();

    Acts::BoundSquareMatrix covMat =
        tracking::sim::utils::unpackCov(tracks.at(iTrack).getPerigeeCov());
    //use pion hypothsis for now. 
    auto part{Acts::GenericParticleHypothesis(Acts::ParticleHypothesis(Acts::PdgParticle(pionPdgId)))};
    //    Acts::PdgParticle(tracks.at(iTrack).getPdgID())))};

    billoir_tracks.push_back(Acts::BoundTrackParameters(perigeeSurface, paramVec, std::move(covMat), part));
  }

  // Select exactly 2 tracks
  //  if (billoir_tracks.size() != 2) {

  //check the number of tracks
  if (billoir_tracks.size() >10 || billoir_tracks.size() <2) {
    ldmx_log(info)<<" bailing because we found "<<billoir_tracks.size()
		  <<" tracks";
    return;
  }

  //loop over all pairs or tracks
  std::vector<ldmx::Vertex> foundVerts; 
  for (int iBtp = 0; iBtp<billoir_tracks.size(); iBtp++){
    for (int kBtp = iBtp+1; kBtp<billoir_tracks.size(); kBtp++){

      if (billoir_tracks.at(iBtp).charge() * billoir_tracks.at(kBtp).charge() > 0) {
	ldmx_log(info)<<" bailing on this pair because tracks have same sign "; 
	continue;
      }

      std::vector<Acts::InputTrack> in_tracks;

      in_tracks.push_back(Acts::InputTrack(&billoir_tracks.at(iBtp))); 
      in_tracks.push_back(Acts::InputTrack(&billoir_tracks.at(kBtp))); 
      ldmx_log(info)<<"Fitting vertex of two tracks";
      Acts::Result<Acts::Vertex> vertRes=billoirFitter.fit(in_tracks,vfOptions, fieldCache);

      if (vertRes.ok()){
	Acts::Vertex vert=vertRes.value();
	ldmx_log(info)<<"done with vertex"; 
	ldmx_log(info)<<"vertex position (x,y,z) = ("
		      <<vert.position()[0]<<","
		      <<vert.position()[1]<<","
		      <<vert.position()[2]<<")";
	ldmx_log(info)<<"vertex chi2/NDF = "
		      <<vert.fitQuality().first<<"/"
		      <<vert.fitQuality().second;
	//fill in the ldmx::vertex
	ldmx::Vertex ldmxVert=ldmx::Vertex();
	ldmxVert.setPosition(std::vector<double>{vert.position()[0],vert.position()[1], vert.position()[2]}); 
	ldmxVert.setTime(vert.time());
	ldmxVert.setChi2(vert.fitQuality().first); 
	ldmxVert.setNDF(vert.fitQuality().second); 
	//check if TrackAtVertex are the fitted tracks
	foundVerts.push_back(ldmxVert);
      } else{
	ldmx_log(info)<<"vertex fit failed"; 
      }
      //  h_m_->Fill((p1 + p2).M());
    }
  }
  
  // Add the tracks to the event
  ldmx_log(info)<<"adding "<<foundVerts.size()<<" to event in collection name "<<out_vtx_collection_; 
  event.add(out_vtx_collection_, foundVerts);
  // Pion mass hypothesis
  /*
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
    for (int iSeed = 0; iSeed < seeds.size(); iSeed++) {
      std::shared_ptr<Acts::PerigeeSurface> perigeeSurface2 =
          Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
              seeds.at(iSeed).getPerigeeX(), seeds.at(iSeed).getPerigeeY(),
              seeds.at(iSeed).getPerigeeZ()));

      Acts::BoundVector paramVec;
      paramVec << seeds.at(iSeed).getD0(), seeds.at(iSeed).getZ0(),
          seeds.at(iSeed).getPhi(), seeds.at(iSeed).getTheta(),
          seeds.at(iSeed).getQoP(), seeds.at(iSeed).getT();

      Acts::BoundSquareMatrix covMat =
          tracking::sim::utils::unpackCov(seeds.at(iSeed).getPerigeeCov());
      int pionPdgId = 211;  // pi+
      if (seeds.at(iSeed).q() < 0) pionPdgId = -211;
      // BoundTrackParameters needs the particle hypothesis
      auto part{Acts::GenericParticleHypothesis(
          Acts::ParticleHypothesis(Acts::PdgParticle(pionPdgId)))};
      auto boundSeedParams = Acts::BoundTrackParameters(
          perigeeSurface, paramVec, std::move(covMat), part);

      TLorentzVector pion4v;
      pion4v.SetXYZM(boundSeedParams.momentum()(0),
                     boundSeedParams.momentum()(1),
                     boundSeedParams.momentum()(2), pion_mass);

      pion_seeds.push_back(pion4v);
    }  // loops on seeds

    h_m_truth_->Fill((pion_seeds.at(0) + pion_seeds.at(1)).M());
  }
  */
  /*
    if ((pion_seeds.size() == 2) &&
    (pion_seeds.at(0) + pion_seeds.at(1)).M() > 0.490 &&
    (pion_seeds.at(0) + pion_seeds.at(1)).M() < 0.510) {
    // Check if the tracks have opposite charge
    h_m_truthFilter_->Fill((p1 + p2).M());
    }
  */
  

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
  h_m_truthFilter_->Write();
  outfile->Close();
  delete outfile;

  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(3)
                 << processing_time_ / nevents_ << " ms";
}

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::VertexProcessor)
