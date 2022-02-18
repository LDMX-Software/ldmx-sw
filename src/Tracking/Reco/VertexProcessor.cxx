#include "Tracking/Reco/VertexProcessor.h"
#include "Acts/MagneticField/ConstantBField.hpp"


#include <chrono>

using namespace framework;

namespace tracking {
namespace reco {

VertexProcessor::VertexProcessor(const std::string &name,
                                 framework::Process &process)
    : framework::Producer(name,process) {}

VertexProcessor::~VertexProcessor() {}

void VertexProcessor::onProcessStart(){

  gctx_ = Acts::GeometryContext();
  bctx_ = Acts::MagneticFieldContext();
  
  h_m_             =  new TH1F("m","m",100,0.,1.);
  h_m_truthFilter_ =  new TH1F("m_filter","m",100,0.,1.);
  h_m_truth_       =  new TH1F("m_truth","m_truth",100,0.,1.); 
  
  auto localToGlobalBin_xyz = [](std::array<size_t, 3> bins,
                                 std::array<size_t, 3> sizes) {
    return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] + bins[2]);  //xyz - field space
    //return (bins[1] * (sizes[2] * sizes[0]) + bins[2] * sizes[0] + bins[0]);    //zxy
    
  };
  
  InterpolatedMagneticField3 map = makeMagneticFieldMapXyzFromText(std::move(localToGlobalBin_xyz), bfieldMap_,
                                                                   1. * Acts::UnitConstants::mm, //default scale for axes length
                                                                   1000. * Acts::UnitConstants::T, //The map is in kT, so scale it to T
                                                                   false, //not symmetrical
                                                                   true //rotate the axes to tracking frame
                                                                   );
  
  sp_interpolated_bField_ = std::make_shared<InterpolatedMagneticField3>(std::move(map));;
  
  std::cout<<"Check if nullptr::"<<sp_interpolated_bField_.get()<<std::endl;
  
  
}

void VertexProcessor::configure(
    framework::config::Parameters &parameters){

  debug_                = parameters.getParameter<bool>("debug",false);

  //TODO:: the bfield map should be taken automatically
  bfieldMap_            = parameters.getParameter<std::string>("bfieldMap_",
                                                               "/Users/pbutti/sw/data_ldmx/BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat");

  trk_coll_name_       = parameters.getParameter<std::string>("trk_coll_name","Tracks");

}


void VertexProcessor::produce(framework::Event &event) {
  
  //TODO:: Move this to an external file
  //And move all this to a single time per processor not for each event!!

  
  nevents_++;
  auto start = std::chrono::high_resolution_clock::now();
  auto&& stepper = Acts::EigenStepper<>{sp_interpolated_bField_};
  
  // Set up propagator with void navigator
  propagator_ = std::make_shared<VoidPropagator>(stepper);


  //Track linearizer in the proximity of the vertex location
  using Linearizer = Acts::HelicalTrackLinearizer<VoidPropagator>;
  Linearizer::Config linearizerConfig(sp_interpolated_bField_,propagator_);
  Linearizer linearizer(linearizerConfig);

  if (debug_)
    std::cout<<"Vertexing processor for event::"<< nevents_ <<std::endl;

  
  
  // Set up Billoir Vertex Fitter
  using VertexFitter =
      Acts::FullBilloirVertexFitter<Acts::BoundTrackParameters, Linearizer>;

  
  VertexFitter::Config vertexFitterCfg;

  
  VertexFitter billoirFitter(vertexFitterCfg);

  VertexFitter::State state(sp_interpolated_bField_->makeCache(bctx_));
  
  //Unconstrained fit
  //See https://github.com/acts-project/acts/blob/main/Tests/UnitTests/Core/Vertexing/FullBilloirVertexFitterTests.cpp#L149
  //For constraint implementation


  
  Acts::VertexingOptions<Acts::BoundTrackParameters> vfOptions(gctx_,
                                                   bctx_);
  
  
  //Retrieve the track collection
  const std::vector<ldmx::Track> tracks = event.getCollection<ldmx::Track>(trk_coll_name_);

  //Retrieve the truth seeds
  const std::vector<ldmx::Track> seeds = event.getCollection<ldmx::Track>("RecoilTruthSeeds");
  
  if (tracks.size() < 1)
    return;
  
  //Transform the EDM ldmx::tracks to the format needed by ACTS
  std::vector<Acts::BoundTrackParameters> billoir_tracks;
  
  //TODO:: The perigee surface should be common between all tracks.
  //So should only be created once in principle. 
  
  std::shared_ptr<Acts::PerigeeSurface> perigeeSurface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(tracks.front().getPerigeeX(),
                                                                    tracks.front().getPerigeeY(),
                                                                    tracks.front().getPerigeeZ()));
  
  for (unsigned int iTrack = 0; iTrack < tracks.size() ; iTrack++) {
    
    Acts::BoundVector paramVec;
    paramVec <<
        tracks.at(iTrack).getD0(),
        tracks.at(iTrack).getZ0(),
        tracks.at(iTrack).getPhi(),
        tracks.at(iTrack).getTheta(),
        tracks.at(iTrack).getQoP(),
        tracks.at(iTrack).getT();
    
    Acts::BoundSymMatrix covMat =
        tracking::sim::utils::unpackCov(tracks.at(iTrack).getPerigeeCov());
    
    
    billoir_tracks.push_back(
        Acts::BoundTrackParameters(perigeeSurface, paramVec, std::move(covMat)));
  }

  //Select exactly 2 tracks
  if (billoir_tracks.size() != 2) {
    return;
  }

  if (billoir_tracks.at(0).charge() *  billoir_tracks.at(1).charge() > 0 )
    return;

  //Pion mass hypothesis
  double pion_mass = 139.570 * Acts::UnitConstants::MeV;
  
  TLorentzVector p1, p2;
  p1.SetXYZM(billoir_tracks.at(0).momentum()(0),
             billoir_tracks.at(0).momentum()(1),
             billoir_tracks.at(0).momentum()(2),
             pion_mass);
  
  p2.SetXYZM(billoir_tracks.at(1).momentum()(0),
             billoir_tracks.at(1).momentum()(1),
             billoir_tracks.at(1).momentum()(2),
             pion_mass);


  std::vector<TLorentzVector> pion_seeds;
  
  if (seeds.size() == 2) {
      
    for (int iSeed=0; iSeed < seeds.size(); iSeed++) {
      
      std::shared_ptr<Acts::PerigeeSurface> perigeeSurface =
          Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(seeds.at(iSeed).getPerigeeX(),
                                                                        seeds.at(iSeed).getPerigeeY(),
                                                                        seeds.at(iSeed).getPerigeeZ()));
      
      Acts::BoundVector paramVec;
      paramVec <<
          seeds.at(iSeed).getD0(),
          seeds.at(iSeed).getZ0(),
          seeds.at(iSeed).getPhi(),
          seeds.at(iSeed).getTheta(),
          seeds.at(iSeed).getQoP(),
          seeds.at(iSeed).getT();
      
      
      Acts::BoundSymMatrix covMat =
          tracking::sim::utils::unpackCov(seeds.at(iSeed).getPerigeeCov());
      
      
      auto boundSeedParams = 
          Acts::BoundTrackParameters(perigeeSurface, paramVec, std::move(covMat));
      
      TLorentzVector pion4v;
      pion4v.SetXYZM(boundSeedParams.momentum()(0),
                     boundSeedParams.momentum()(1),
                     boundSeedParams.momentum()(2),
                     pion_mass);

      pion_seeds.push_back(pion4v);
    } //loops on seeds
    
    h_m_truth_       ->Fill((pion_seeds.at(0) + pion_seeds.at(1)).M());
    
  }
  
  
  
  if ((pion_seeds.size() == 2) && (pion_seeds.at(0) + pion_seeds.at(1)).M() > 0.490 && (pion_seeds.at(0) + pion_seeds.at(1)).M() < 0.510 ) {
    
    //Check if the tracks have opposite charge
    h_m_truthFilter_ ->Fill((p1+p2).M());
  }
  
  h_m_->Fill((p1+p2).M());
  
  auto end = std::chrono::high_resolution_clock::now();
  //long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  auto diff = end-start;
  processing_time_ += std::chrono::duration <double, std::milli> (diff).count();
  
}

void VertexProcessor::onProcessEnd(){

  TFile* outfile = new TFile("VertexingResults.root","RECREATE");
  outfile->cd();

  h_m_       -> Write();
  h_m_truth_ -> Write();
  h_m_truthFilter_ -> Write();
  outfile->Close();
  delete outfile;
  
  
  
  std::cout<<"PROCESSOR:: "<<this->getName()<<"   AVG Time/Event: " <<processing_time_ / nevents_ << " ms"<<std::endl;  
}


}//tracking
}//reco

DECLARE_PRODUCER_NS(tracking::reco, VertexProcessor)
