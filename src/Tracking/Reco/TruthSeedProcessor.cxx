#include "Tracking/Reco/TruthSeedProcessor.h"
#include "Tracking/Sim/GeometryContainers.h"


namespace tracking::reco {

TruthSeedProcessor::TruthSeedProcessor(const std::string &name,
                                       framework::Process &process)
    : TrackingGeometryUser(name, process) {}

void TruthSeedProcessor::onNewRun(const ldmx::RunHeader& rh) {
  gctx_ = Acts::GeometryContext();
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);
  
  Acts::StraightLineStepper stepper;
  Acts::Navigator::Config navCfg{geometry().getTG()};
  const Acts::Navigator navigator(navCfg);
  
  linpropagator_ = std::make_shared<LinPropagator>(stepper,navigator);
  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(*linpropagator_,
                                                                       geometry_context(),
                                                                       magnetic_field_context());
  
}

void TruthSeedProcessor::configure(framework::config::Parameters &parameters) {
  scoring_hits_coll_name_ =
      parameters.getParameter<std::string>("scoring_hits_coll_name");
  recoil_sim_hits_coll_name_ =
      parameters.getParameter<std::string>("recoil_sim_hits_coll_name");
  tagger_sim_hits_coll_name_ =
      parameters.getParameter<std::string>("tagger_sim_hits_coll_name");
  n_min_hits_tagger_ = parameters.getParameter<int>("n_min_hits_tagger", 11);
  n_min_hits_recoil_ = parameters.getParameter<int>("n_min_hits_recoil", 7);
  pdg_ids_           = parameters.getParameter<std::vector<int> >("pdg_ids", {11});
  z_min_             = parameters.getParameter<double>("z_min", -9999);  // mm
  track_id_          = parameters.getParameter<int>("track_id", -9999);
  pz_cut_            = parameters.getParameter<double>("pz_cut", -9999);  // MeV
  p_cut_             = parameters.getParameter<double>("p_cut", 0.);
  p_cut_max_         = parameters.getParameter<double>("p_cut_max", 100000.);  // MeV
  p_cut_ecal_        = parameters.getParameter<double>("p_cut_ecal", -1.);    // MeV
  recoil_sp_         = parameters.getParameter<double>("recoil_sp",true);
  target_sp_         = parameters.getParameter<double>("tagger_sp",true);
  seedSmearing_      = parameters.getParameter<bool>("seedSmearing", false);
  
    
  ldmx_log(info)<<"Seed Smearing is set to "<< seedSmearing_;
  
  // Relative smear factor terms, only used if seedSmearing_ is true.
  rel_smearfactors_ = parameters.getParameter<std::vector<double>>(
      "rel_smearfactors",{0.1, 0.1, 0.1, 0.1, 0.1, 0.1});
  inflate_factors_  = parameters.getParameter<std::vector<double>>(
      "inflate_factors",{10., 10., 10., 10., 10., 10.});
  
}

void TruthSeedProcessor::createTruthTrack(const ldmx::SimParticle &particle,
                                          const ldmx::SimTrackerHit &hit,
                                          ldmx::TruthTrack& trk,
                                          const std::shared_ptr<Acts::Surface>& target_surface) {
  
  std::vector<double> pos{static_cast<double>(hit.getPosition()[0]),
    static_cast<double>(hit.getPosition()[1]),
    static_cast<double>(hit.getPosition()[2])};
  createTruthTrack(pos, hit.getMomentum(), particle.getCharge(),trk,target_surface);
  
  trk.setTrackID(hit.getTrackID());
  trk.setPdgID(hit.getPdgID());
    
}

void TruthSeedProcessor::createTruthTrack(const ldmx::SimParticle &particle,
                                          ldmx::TruthTrack& trk,
                                          const std::shared_ptr<Acts::Surface>& target_surface) {
  createTruthTrack(particle.getVertex(), particle.getMomentum(),
                   particle.getCharge(),trk,target_surface);

  trk.setPdgID(particle.getPdgID());
  
}

void TruthSeedProcessor::createTruthTrack(const std::vector<double> &pos_vec,
                                          const std::vector<double> &p_vec,
                                          int charge,
                                          ldmx::TruthTrack& trk,
                                          const std::shared_ptr<Acts::Surface>& target_surface) {
  // Get the position and momentum of the particle at the point of creation.
  // This only works for the incident electron when creating a tagger tracker
  // seed. For the recoil tracker, the scoring plane position will need to
  // be used.  For other particles created in the target or tracker planes,
  // this version of the method can also be used.
  // These are just Eigen vectors defined as
  // Eigen::Matrix<double, kSize, 1>;
  Acts::Vector3 pos{pos_vec.data()};
  Acts::Vector3 mom{p_vec.data()};
  double time{0.};

  // Rotate the position and momentum into the ACTS frame.
  pos = tracking::sim::utils::Ldmx2Acts(pos);
  mom = tracking::sim::utils::Ldmx2Acts(mom);

  // Get the charge of the particle.
  // TODO: Add function that uses the PDG ID to calculate this.
  double q{charge * Acts::UnitConstants::e};


  // The idea here is:
  // 1 - Define a bound track state parameters at point P on track. Basically a curvilinear representation
  // 2 - Propagate to target surface to obtain the BoundTrackState there.
  
  
  // Transform the position, momentum and charge to free parameters.
  auto free_params{tracking::sim::utils::toFreeParameters(pos, mom, q)};

  // Create a line surface at the perigee.  The perigee position is extracted
  // from a particle's vertex or the particle's position at a specific
  // scoring plane.
  auto gen_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3(free_params[Acts::eFreePos0], free_params[Acts::eFreePos1],
                    free_params[Acts::eFreePos2]))};

  // Transform the parameters to local positions on the perigee surface.
  auto bound_params{Acts::detail::transformFreeToBoundParameters(
      free_params, *gen_surface, gctx_).value()};
  

  Acts::BoundTrackParameters boundTrkPars(gen_surface,bound_params,std::nullopt);
  
  // Linear propagation to the target surface   
  auto propBoundState = trk_extrap_->extrapolate(boundTrkPars, target_surface);
  
  
  // Create the seed track object.
  Acts::Vector3 ref = target_surface->center(geometry_context());
  //trk.setPerigeeLocation(free_params[Acts::eFreePos0],
  //                       free_params[Acts::eFreePos1],
  //                       free_params[Acts::eFreePos2]);
  
  trk.setPerigeeLocation(ref(0), ref(1), ref(2));
  
  auto propBoundVec = (propBoundState.value()).parameters();
  
  
  trk.setPerigeeParameters(
      tracking::sim::utils::convertActsToLdmxPars(propBoundVec));
  
  trk.setPosition(pos(0),pos(1),pos(2));
  trk.setMomentum(mom(0),mom(1),mom(2));
  
}

ldmx::Track TruthSeedProcessor::seedFromTruth(const ldmx::TruthTrack& tt) {

  ldmx::Track seed = ldmx::Track();
    seed.setPerigeeLocation(tt.getPerigeeLocation()[0],
                            tt.getPerigeeLocation()[1],
                            tt.getPerigeeLocation()[2]);
    seed.setChi2(0.);
    seed.setNhits(tt.getNhits());
    seed.setNdf(0);
    seed.setNsharedHits(0);
    seed.setTrackID(tt.getTrackID());
    seed.setPdgID(tt.getPdgID());
    seed.setTruthProb(1.);
    
    Acts::BoundVector bound_params;
    Acts::BoundVector stddev;
 
    if (seedSmearing_) {
    
      double sigma_d0     = rel_smearfactors_[Acts::eBoundLoc0]   * tt.getD0();
      double sigma_z0     = rel_smearfactors_[Acts::eBoundLoc1]   * tt.getZ0();
      double sigma_phi    = rel_smearfactors_[Acts::eBoundPhi]    * tt.getPhi();
      double sigma_theta  = rel_smearfactors_[Acts::eBoundTheta]  * tt.getTheta();
      double sigma_p      = rel_smearfactors_[Acts::eBoundQOverP] * abs(1/tt.getQoP());
      double sigma_t      = rel_smearfactors_[Acts::eBoundTime]   * tt.getT(); 
    
      double smear     = (*normal_)(generator_);
      double d0smear    = tt.getD0()         + smear * sigma_d0;
    
      smear = (*normal_)(generator_);
      double z0smear    = tt.getZ0()         + smear * sigma_z0;
    
      smear = (*normal_)(generator_);
      double Phismear   = tt.getPhi()        + smear * sigma_phi;
    
      smear = (*normal_)(generator_);
      double Thetasmear = tt.getTheta()      + smear * sigma_theta;
      
      double p = std::abs(1. / tt.getQoP());
      smear = (*normal_)(generator_);
      double Psmear     = p + smear * sigma_p;
      
      double Q          = tt.getQoP() < 0 ? -1. : 1.;
      double QoPsmear   = Q / Psmear;
      
      smear = (*normal_)(generator_);
      double Tsmear = tt.getT()              + smear * sigma_t; 
      
    
      bound_params << d0smear, z0smear, Phismear, Thetasmear,
          QoPsmear, Tsmear;

      stddev[Acts::eBoundLoc0] =
          inflate_factors_[Acts::eBoundLoc0] * sigma_d0 * Acts::UnitConstants::mm;
      stddev[Acts::eBoundLoc1] =
          inflate_factors_[Acts::eBoundLoc1] * sigma_z0 * Acts::UnitConstants::mm;
      stddev[Acts::eBoundPhi] =
          inflate_factors_[Acts::eBoundPhi] * sigma_phi;
      stddev[Acts::eBoundTheta] =
          inflate_factors_[Acts::eBoundTheta] * sigma_theta;
      stddev[Acts::eBoundQOverP] =
          inflate_factors_[Acts::eBoundQOverP] * (1. / p) * (1. / p) * sigma_p;
      stddev[Acts::eBoundTime] =
          inflate_factors_[Acts::eBoundTime] * sigma_t * Acts::UnitConstants::ns;


      std::vector<double> v_seed_params(
          (bound_params).data(),
          bound_params.data() + bound_params.rows() * bound_params.cols());

      Acts::BoundSymMatrix bound_cov = stddev.cwiseProduct(stddev).asDiagonal();
      std::vector<double> v_seed_cov;
      tracking::sim::utils::flatCov(bound_cov, v_seed_cov);
      seed.setPerigeeParameters(v_seed_params);
      seed.setPerigeeCov(v_seed_cov);
      
      
    }
    else  {
      // Do not smear the seed
      
      bound_params << tt.getD0(), tt.getZ0(), tt.getPhi(), tt.getTheta(),
          tt.getQoP(), tt.getT();


      std::vector<double> v_seed_params(
          (bound_params).data(),
          bound_params.data() + bound_params.rows() * bound_params.cols());


      double p = std::abs(1. / tt.getQoP());
      double sigma_p = 0.75 * p * Acts::UnitConstants::GeV;
      stddev[Acts::eBoundLoc0] = 2 * Acts::UnitConstants::mm;
      stddev[Acts::eBoundLoc1] = 5 * Acts::UnitConstants::mm;
      stddev[Acts::eBoundTime] = 1000 * Acts::UnitConstants::ns;
      stddev[Acts::eBoundPhi] = 5 * Acts::UnitConstants::degree;
      stddev[Acts::eBoundTheta] = 5 * Acts::UnitConstants::degree;
      stddev[Acts::eBoundQOverP] = (1. / p) * (1. / p) * sigma_p;
      
      Acts::BoundSymMatrix bound_cov = stddev.cwiseProduct(stddev).asDiagonal();
      std::vector<double> v_seed_cov;
      tracking::sim::utils::flatCov(bound_cov, v_seed_cov);
      seed.setPerigeeParameters(v_seed_params);
      seed.setPerigeeCov(v_seed_cov);
      
    }


    return seed;
}


//TODO::There is a problem with tagger hits
void TruthSeedProcessor::makeHitCountMap(const std::vector<ldmx::SimTrackerHit>& sim_hits,
                                         std::map<int,std::vector<ldmx::SimTrackerHit>>& hit_count_map) {
  
  for (auto &sim_hit : sim_hits) {
        
    // This track never left a hit before
    if (!hit_count_map.count(sim_hit.getTrackID())) {
      hit_count_map[sim_hit.getTrackID()].push_back(sim_hit);
    }
    
    // This track left a hit before.
    // Check if it's on a different sensor than the others
 
    else {
      int sensorID = tracking::sim::utils::getSensorID(sim_hit);
      bool foundHit = false;
      
      for (auto& rhit : hit_count_map[sim_hit.getTrackID()]) {
        
        int tmp_sensorID = tracking::sim::utils::getSensorID(rhit);
        
        if (sensorID == tmp_sensorID) {
          foundHit = true;
          break;
        }
      } // loop on the already recorded hits

      
      if (!foundHit) {
        hit_count_map[sim_hit.getTrackID()].push_back(sim_hit);
      }
    }
  }
}

bool TruthSeedProcessor::scoringPlaneHitFilter(
    const ldmx::SimTrackerHit &hit,
    const std::vector<ldmx::SimTrackerHit> &ecal_sp_hits) {
  // Clean some of the hits we don't want
  if (hit.getPosition()[2] < z_min_) return false;

  // Check if the track_id was requested
  if (track_id_ > 0 && hit.getTrackID() != track_id_) return false;

  // Check if we are requesting particular particles
  if (std::find(pdg_ids_.begin(), pdg_ids_.end(), hit.getPdgID()) ==
      pdg_ids_.end())
    return false;

  Acts::Vector3 p_vec{hit.getMomentum()[0], hit.getMomentum()[1],
                      hit.getMomentum()[2]};

  // p cut
  if (p_cut_ >= 0. && p_vec.norm() < p_cut_) return false;

  // p cut Max
  if (p_cut_ < 100000. && p_vec.norm() > p_cut_max_) return false;

  // pz cut
  if (pz_cut_ > -9999 && p_vec(2) < pz_cut_) return false;

  // Check the ecal scoring plane
  bool pass_ecal_scoring_plane = true;

  if (p_cut_ecal_ > 0) {  // only check if we care about it.

    for (auto &e_sp_hit : ecal_sp_hits) {
      if (e_sp_hit.getTrackID() == hit.getTrackID() &&
          e_sp_hit.getPdgID() == hit.getPdgID()) {
        Acts::Vector3 e_sp_p{e_sp_hit.getMomentum()[0],
                             e_sp_hit.getMomentum()[1],
                             e_sp_hit.getMomentum()[2]};

        if (e_sp_p.norm() < p_cut_ecal_) pass_ecal_scoring_plane = false;

        // Skip the rest of the scoring plane hits since we already found the
        // track we care about
        break;

      }  // check that the hit belongs to the inital particle from the target
         // scoring plane hit
    }    // loop on Ecal scoring plane hits
  }      // pcutEcal

  if (!pass_ecal_scoring_plane) return false;

  return true;
}

void TruthSeedProcessor::produce(framework::Event &event) {

  //Retrieve the particleMap
  auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};

  // Retrieve the target scoring hits
  // Information is extracted using the
  // scoring plane hit left by the particle at the target.
  
  const std::vector<ldmx::SimTrackerHit> scoring_hits{
    event.getCollection<ldmx::SimTrackerHit>(scoring_hits_coll_name_)};
  
  // Retrieve the scoring plane hits at the ECAL
  const std::vector<ldmx::SimTrackerHit> scoring_hits_ecal{
    event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits")};

  // Retrieve the sim hits in the tagger tracker
  const std::vector<ldmx::SimTrackerHit> tagger_sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(tagger_sim_hits_coll_name_);
  

  // Retrieve the sim hits in the recoil tracker
  const std::vector<ldmx::SimTrackerHit> recoil_sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(recoil_sim_hits_coll_name_);

  
  std::map<int,std::vector<ldmx::SimTrackerHit>> hit_count_map_recoil;
  makeHitCountMap(recoil_sim_hits, hit_count_map_recoil);

  std::map<int,std::vector<ldmx::SimTrackerHit>> hit_count_map_tagger;
  makeHitCountMap(tagger_sim_hits, hit_count_map_tagger);
  
  
  // to keep track of how many sim particles leave hits on the scoring plane
  std::vector<int> recoil_sh_idxs;
  std::unordered_map<int,std::vector<int> > recoil_sh_count_map;

  // We are only interested in the beam electron
  int idx_taggerhit = -1;


  //Target scoring hits for Tagger will have Z<0, Recoil scoring hits will have Z>0
  for (unsigned int i_sh=0; i_sh < scoring_hits.size(); i_sh++) {
    
    const ldmx::SimTrackerHit& hit  = scoring_hits.at(i_sh);
    Acts::Vector3 p_vec{hit.getMomentum()[0],hit.getMomentum()[1],hit.getMomentum()[2]};

    double tagger_p_max  = 0.;
         
    //Check if it is a tagger track going fwd that passes basic cuts
    if (hit.getPosition()[2] < 0.) {
      //Tagger selection cuts
      //Negative scoring plane hit, with momentum > p_cut
      if (p_vec(2)         < 0.      ||
          p_vec.norm()     < p_cut_  ||
          hit.getPdgID()   != 11     ||
          hit.getTrackID() != 1) 
        continue;

      if (p_vec.norm() > tagger_p_max) {
        idx_taggerhit = i_sh;
        tagger_p_max = p_vec.norm();
      }
    }//Tagger loop
    
    //Check the recoil hits
    else {

      //Recoil selection cuts
      //Positive scoring plane hit, forward direction with momentum > p_cut
      if (p_vec(2) < 0.         ||
          p_vec.norm() < p_cut_ )
        continue;

      //Check that the hit was left by a charged particle
      if  (abs(particleMap[hit.getTrackID()].getCharge()) < 1e-8)
        continue;
      
      recoil_sh_count_map[hit.getTrackID()].push_back(i_sh);
      
      
    }//Recoil
  }// loop on Target scoring plane hits


  for (std::pair<int,std::vector<int>> element : recoil_sh_count_map) {
        
    std::sort(element.second.begin(),element.second.end(),
              [&](const int idx1, int idx2) -> bool {
                
                const ldmx::SimTrackerHit& hit1 = scoring_hits.at(idx1);
                const ldmx::SimTrackerHit& hit2 = scoring_hits.at(idx2);
                
                Acts::Vector3 phit1{hit1.getMomentum()[0],hit1.getMomentum()[1],hit1.getMomentum()[2]};
                Acts::Vector3 phit2{hit2.getMomentum()[0],hit2.getMomentum()[1],hit2.getMomentum()[2]};
                
                return phit1.norm() > phit2.norm();
              }
              );
  }
  
  std::vector<ldmx::TruthTrack> tagger_truth_tracks;
  std::vector<ldmx::Track>      tagger_truth_seeds;
  std::vector<ldmx::TruthTrack> recoil_truth_tracks;
  std::vector<ldmx::Track>      recoil_truth_seeds;

  // TODO:: The target should be taken from some conditions DB in the future.
  // Define the perigee_surface at 0.0.0
  auto targetSurface{Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0.,0.,0.))};
  
  //Define the target_surface
  auto targetUnboundSurface = tracking::sim::utils::unboundSurface(0.);
  
  if (idx_taggerhit != -1) {
    
    const ldmx::SimTrackerHit& hit = scoring_hits.at(idx_taggerhit);
    const ldmx::SimParticle& phit  = particleMap[hit.getTrackID()];

    if (hit_count_map_tagger[hit.getTrackID()].size() > n_min_hits_tagger_) {
      
      ldmx::TruthTrack truth_tagger_track;
      createTruthTrack(phit, hit, truth_tagger_track, targetSurface);
      truth_tagger_track.setNhits(hit_count_map_tagger[hit.getTrackID()].size());
      //get track state at the generation point
      //ldmx::TruthTrack::TrackState ts_beamOrigin(phit,"beam_origin");
      //propagate track to target
      //trackExtrapolator(truth_tagger_track, perigee_surface);
      //truth_tagger_track.addTrackState(ts_beamOrigin);
      tagger_truth_tracks.push_back(truth_tagger_track);
    }
  }
  
  
  // Recoil target surface for truth and seed tracks is the target
  
  
  for (std::pair<int,std::vector<int>> element : recoil_sh_count_map) {
                                                        
    //Only take the first entry of the vector: it should be the scoring plane hit with the highest momentum. 
    const ldmx::SimTrackerHit& hit  = scoring_hits.at(element.second.at(0));
    const ldmx::SimParticle&   phit = particleMap[hit.getTrackID()];
    
    //Findable particle selection
    if (hit_count_map_recoil[hit.getTrackID()].size() > n_min_hits_recoil_) {
      
      ldmx::TruthTrack truth_recoil_track;
      createTruthTrack(phit,hit,truth_recoil_track,targetSurface);
      truth_recoil_track.setNhits(hit_count_map_recoil[hit.getTrackID()].size());
      recoil_truth_tracks.push_back(truth_recoil_track);
    }
  }
  
  // Form a truth seed from a truth track

  for (auto& tt : tagger_truth_tracks) {
    ldmx::Track seed = seedFromTruth(tt);
    tagger_truth_seeds.push_back(seed);
  }
  
  for ( auto& tt: recoil_truth_tracks) {
    ldmx::Track seed =  seedFromTruth(tt);
    recoil_truth_seeds.push_back(seed);
  }

  
  
  event.add("TaggerTruthTracks",tagger_truth_tracks);
  event.add("RecoilTruthTracks",recoil_truth_tracks);
  event.add("TaggerTruthSeeds", tagger_truth_seeds);
  event.add("RecoilTruthSeeds" ,recoil_truth_seeds);
  
  
}
}  // namespace tracking::reco

DECLARE_PRODUCER_NS(tracking::reco, TruthSeedProcessor)
