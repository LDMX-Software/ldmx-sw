#include "Tracking/Reco/TruthSeedProcessor.h"
#include <chrono>

using namespace framework;

namespace tracking::reco {

TruthSeedProcessor::TruthSeedProcessor(const std::string &name,
                                       framework::Process &process)
    : framework::Producer(name,process) {}

TruthSeedProcessor::~TruthSeedProcessor() {}

void TruthSeedProcessor::onProcessStart() {

  gctx_ = Acts::GeometryContext();
  
}

void TruthSeedProcessor::configure(framework::config::Parameters &parameters) {

  debug_               = parameters.getParameter<bool>("debug",false);
  out_trk_coll_name_   = parameters.getParameter<std::string>("trk_coll_name","TruthSeeds");
  pdgIDs_              = parameters.getParameter<std::vector<int> >("pdgIDs",{11});
  scoring_hits_        = parameters.getParameter<std::string>("scoring_hits","TargetScoringPlaneHits_sim");
  sim_hits_       = parameters.getParameter<std::string>("sim_hits","RecoilSimHits");
  n_min_hits_          = parameters.getParameter<int>("n_min_hits",7);
  z_min_               = parameters.getParameter<double>("z_min",-999); //mm
  track_id_            = parameters.getParameter<int>("track_id",-999);
  pz_cut_              = parameters.getParameter<double>("pz_cut",-9999); //MeV
  p_cut_               = parameters.getParameter<double>("p_cut", 0.);
}


//Look at the scoring planes trackID
  // -> The trackID is the unique identification of the particles in the generated event
  // -> 1 is the primary electron since it's the first one generated
  // -> For brehmsstrahlung / Energy Loss due to Ionization,  the outgoing electron trackID is the same of the incoming
  // --- > The recoil electron will still trackID == 1
  // --- > Get the TargetScoringPlaneHits. Filter on z > 4.5mm, Get the TrackID==1 and get the (x,y,z,px,py,pz).

  // -> For eN interactions G4 is not able to follow the particle through the interaction. New particles will be created
  // --- > Look through the recoil sim hits, get the track ID from those hits and *then* find the TargetScoringPlaneHits with that trackID to do truth matching.
  // --- > There will be other trackIDs in the hits. Pick the ones that are electrons, and pick the highest energy ones.

  // One thing that I can check is the endpoint of the trackID==1 particle to understand what kind of interaction the particle went through.
  

  //Truth based initial track parameters

//In the case of Recoil tracking take only TrackID==1 and the generation surface is the obtained from the TargetScoringPlane



void TruthSeedProcessor::produce(framework::Event &event) {

  nevents_++;
  auto start = std::chrono::high_resolution_clock::now();

  //Retrieve the scoring plane hits
  const std::vector<ldmx::SimTrackerHit> scoring_hits =
      event.getCollection<ldmx::SimTrackerHit>(scoring_hits_);
  
  
  if (scoring_hits.size() < 1)
    return;

  //Retrieve the sim hits in the tracker of interest
  const std::vector<ldmx::SimTrackerHit> sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(sim_hits_);
  
  
  //TODO:: change to indices instead objects
  std::vector<ldmx::SimTrackerHit> selected_sp_hits;

    
  for (auto & t_sp_hit : scoring_hits) {

    //Clean some of the hits we don't want
    if (t_sp_hit.getPosition()[2] <  z_min_)
      continue;

    //Check if the track_id was requested
    if (track_id_ > 0 && t_sp_hit.getTrackID() != track_id_)
      continue;

    //Check if we are requesting particular particles
    if (std::find(pdgIDs_.begin(), pdgIDs_.end(), t_sp_hit.getPdgID()) == pdgIDs_.end())
      continue;

    Acts::Vector3 t_sp_p{t_sp_hit.getMomentum()[0],t_sp_hit.getMomentum()[1], t_sp_hit.getMomentum()[2]};
    
    //p cut
    if (p_cut_ >= 0. && t_sp_p.norm() < p_cut_)
      continue;

    //pz cut
    if (pz_cut_ > -9999 && t_sp_p(2) < pz_cut_)
      continue;
    
    //add the point
    selected_sp_hits.push_back(t_sp_hit);
  }

  if (debug_)
    std::cout<<"Selected scoring hits::"<<selected_sp_hits.size()<<std::endl;

  std::vector<ldmx::Track> truth_seeds_{};
  
  for (auto& sel_hit : selected_sp_hits) {

    int seed_particle_hits = 0;

    //Let's check how many hits this particle leaves in the recoil
    for (auto& sim_hit : sim_hits)
      if (sim_hit.getTrackID() == sel_hit.getTrackID())
        seed_particle_hits++;
    
    
    if (seed_particle_hits < n_min_hits_)
      continue;
    
    
    
    Acts::Vector3 gen_pos{sel_hit.getPosition()[0], sel_hit.getPosition()[1], sel_hit.getPosition()[2]};
    Acts::Vector3 gen_mom{sel_hit.getMomentum()[0], sel_hit.getMomentum()[1], sel_hit.getMomentum()[2]};

    //Rotate into the ACTS Frame
    gen_pos = tracking::sim::utils::Ldmx2Acts(gen_pos);
    gen_mom = tracking::sim::utils::Ldmx2Acts(gen_mom);
    
    //Check the pdg id 
    Acts::ActsScalar q = -1 * Acts::UnitConstants::e;

    //Electrons or muons
    if (abs(sel_hit.getPdgID()) == 11 || abs(sel_hit.getPdgID() == 13)) {
      
      q = sel_hit.getPdgID() > 0 ? -1 * Acts::UnitConstants::e : Acts::UnitConstants::e;
    }
    
    //Whatever else: pi, K ...
    else {
      q = sel_hit.getPdgID() > 0 ? Acts::UnitConstants::e : -1 * Acts::UnitConstants::e;
    }

    if (debug_) {
      std::cout<<"Preparing seed from hit:"<<std::endl;
      std::cout<<sel_hit.getPdgID()<<" " << sel_hit.getPosition()[0]<<","<< sel_hit.getPosition()[1]<<","<< sel_hit.getPosition()[2]<<std::endl;
      std::cout<<q<<" " << sel_hit.getMomentum()[0]<<","<< sel_hit.getMomentum()[1]<<","<< sel_hit.getMomentum()[2]<<std::endl;
    }

    Acts::FreeVector free_params = tracking::sim::utils::toFreeParameters(gen_pos, gen_mom, q);
    std::shared_ptr<const Acts::PerigeeSurface> gen_surface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(
            Acts::Vector3(free_params[Acts::eFreePos0],
                          free_params[Acts::eFreePos1],
                          free_params[Acts::eFreePos2]));
    auto bound_params = Acts::detail::transformFreeToBoundParameters(free_params, *gen_surface, gctx_).value();

    //Blown up covariance matrix
    Acts::BoundSymMatrix bound_cov = 100. * Acts::BoundSymMatrix::Identity();
    
    //Form the seed track for the event bus

    ldmx::Track seedTrack = ldmx::Track();
    seedTrack.setPerigeeLocation(free_params[Acts::eFreePos0],
                                 free_params[Acts::eFreePos1],
                                 free_params[Acts::eFreePos2]);
    seedTrack.setChi2(0.);
    seedTrack.setNhits(seed_particle_hits);
    seedTrack.setNdf(0);
    seedTrack.setNsharedHits(0.);
        
    std::vector<double> ldmx_cov;
    tracking::sim::utils::flatCov(bound_cov,ldmx_cov);
    seedTrack.setPerigeeParameters(tracking::sim::utils::convertActsToLdmxPars(bound_params));
    seedTrack.setPerigeeCov(ldmx_cov);
        
    truth_seeds_.push_back(seedTrack);
    
    
  }//selected hits

  if (debug_)
    std::cout<<"Adding "<<out_trk_coll_name_<<" seeds" <<std::endl;
  
  event.add(out_trk_coll_name_, truth_seeds_);
  
  auto end = std::chrono::high_resolution_clock::now();
  //long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  auto diff = end-start;
  processing_time_ += std::chrono::duration <double, std::milli> (diff).count();
  
}

void TruthSeedProcessor::onProcessEnd() {

  std::cout<<"PROCESSOR:: "<<this->getName()<<"   AVG Time/Event: " <<processing_time_ / nevents_ << " ms"<<std::endl;
}


} //tracking::reco

DECLARE_PRODUCER_NS(tracking::reco, TruthSeedProcessor)
