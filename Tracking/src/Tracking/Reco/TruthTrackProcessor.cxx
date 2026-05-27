#include "Tracking/Reco/TruthTrackProcessor.h"


#include "Tracking/Sim/GeometryContainers.h"


namespace tracking::reco {


TruthTrackProcessor::TruthTrackProcessor(const std::string& name,
                                         framework::Process& process)
    : TrackingGeometryUser(name, process) {}


void TruthTrackProcessor::onNewRun(const ldmx::RunHeader& rh) {
  gctx_ = Acts::GeometryContext();


  // Custom transformation of the interpolated bfield map
  auto transform_pos = [](const Acts::Vector3& pos_) {
    Acts::Vector3 rot_pos;
    rot_pos(0) = pos_(1);
    rot_pos(1) = pos_(2);
    rot_pos(2) = pos_(0) + DIPOLE_OFFSET;
    return rot_pos;
  };


  auto transform_b_field = [](const Acts::Vector3& field,
                              const Acts::Vector3& /*pos_*/) {
    Acts::Vector3 rot_field;
    rot_field(0) = field(2);
    rot_field(1) = field(0);
    rot_field(2) = field(1);
    return rot_field;
  };


  // Setup the interpolated bfield map
  const auto map = std::make_shared<InterpolatedMagneticField3>(
      loadDefaultBField(field_map_, transform_pos, transform_b_field));


  // Setup the stepper and navigator
  const auto stepper = Acts::EigenStepper<>{map};
  Acts::Navigator::Config nav_cfg{geometry().getTG()};
  nav_cfg.resolveMaterial = true;
  nav_cfg.resolvePassive = true;
  nav_cfg.resolveSensitive = true;
  const Acts::Navigator navigator(nav_cfg);


  propagator_ = std::make_unique<TruthPropagator>(
      stepper, navigator,
      Acts::getDefaultLogger("TruthPropagator", Acts::Logging::FATAL));
  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(
      *propagator_, geometryContext(), magneticFieldContext());
  trk_extrap_->setMaxStepSize(200);  // mm
  trk_extrap_->setPathLimit(3000);   // mm
}


void TruthTrackProcessor::configure(framework::config::Parameters& parameters) {
  input_pass_name_ = parameters.get<std::string>("input_pass_name");


  skip_tagger_ = parameters.get<bool>("skip_tagger", false);
  skip_recoil_ = parameters.get<bool>("skip_recoil", false);
  particle_hypothesis_ = parameters.get<int>("particle_hypothesis");


  tagger_seeds_collection_ =
      parameters.get<std::string>("tagger_seeds_collection");
  recoil_seeds_collection_ =
      parameters.get<std::string>("recoil_seeds_collection");
  tagger_tracks_collection_ =
      parameters.get<std::string>("tagger_tracks_collection");
  recoil_tracks_collection_ =
      parameters.get<std::string>("recoil_tracks_collection");


  n_min_hits_tagger_ = parameters.get<int>("n_min_hits_tagger", 11);
  n_min_hits_recoil_ = parameters.get<int>("n_min_hits_recoil", 7);
  pz_cut_ = parameters.get<double>("pz_cut", -9999.0);
  p_cut_ = parameters.get<double>("p_cut", 0.0);
  p_cut_max_ = parameters.get<double>("p_cut_max", 100000.0);


  field_map_ = parameters.get<std::string>("field_map");
  ecal_sp_coll_name_ = parameters.get<std::string>("ecal_sp_coll_name", "EcalScoringPlaneHits");
  sp_pass_name_ = parameters.get<std::string>("sp_pass_name", "");
  sim_particles_coll_name_ = parameters.get<std::string>("sim_particles_coll_name", "SimParticles");
  sim_particles_passname_ = parameters.get<std::string>("sim_particles_passname", "");
}


void TruthTrackProcessor::produce(framework::Event& event) {
  const std::vector<ldmx::Track> tagger_seeds =
      event.getCollection<ldmx::Track>(tagger_seeds_collection_,
                                       input_pass_name_);
  const std::vector<ldmx::Track> recoil_seeds =
      event.getCollection<ldmx::Track>(recoil_seeds_collection_,
                                       input_pass_name_);

  auto target_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3(0., 0., 0.))};


  const std::vector<ldmx::SimTrackerHit> ecal_sp_hits =
      event.getCollection<ldmx::SimTrackerHit>(ecal_sp_coll_name_,
                                               sp_pass_name_);
  std::vector<ldmx::SimTrackerHit> sel_ecal_sp_hits;
  for (auto sp_hit : ecal_sp_hits) {
    if (sp_hit.getMomentum()[2] > 0 && ((sp_hit.getID() & 0xfff) == 31)) {
      sel_ecal_sp_hits.push_back(sp_hit);
    }
  }


  std::map<int, ldmx::SimParticle> particle_map;
  if (event.exists(sim_particles_coll_name_, sim_particles_passname_)) {
    particle_map = event.getMap<int, ldmx::SimParticle>(
        sim_particles_coll_name_, sim_particles_passname_);
  }


  auto ecal_surface = tracking::sim::utils::unboundSurface(240.5);
  std::vector<ldmx::Track> tagger_tracks;
  std::vector<ldmx::Track> recoil_tracks;


  if (!skip_tagger_) {
    for (const auto& seed : tagger_seeds) {
      if (seed.getNhits() <= n_min_hits_tagger_) continue;


      auto seed_mom = seed.getMomentumAtTarget();
      //only continue extrapolation based on these cuts
      if (seed.getPerigeeParameters().empty()) continue;

      if (seed_mom.empty()) continue;

      Acts::Vector3 p_vec{seed_mom[0], seed_mom[1], seed_mom[2]};
       
      // p cut
      if (p_cut_ >= 0. && p_vec.norm() < p_cut_) continue;

      // p cut Max
      if (p_cut_ < 100000. && p_vec.norm() > p_cut_max_) continue;

      // pz cut
       if (pz_cut_ > -9999 && p_vec(2) < pz_cut_) continue;
      //if (p_vec(2) < 0.0) continue;


      Acts::BoundVector bound_params;
      bound_params << seed.getD0(), seed.getZ0(), seed.getPhi(),
          seed.getTheta(), seed.getQoP(), seed.getT();


      auto part{Acts::GenericParticleHypothesis(
          Acts::ParticleHypothesis(Acts::PdgParticle(particle_hypothesis_)))};


      Acts::Vector3 perigee_ldmx(seed.getPerigeeLocation()[0],
                                 seed.getPerigeeLocation()[1],
                                 seed.getPerigeeLocation()[2]);
  Acts::Vector3 perigee_acts = tracking::sim::utils::ldmx2Acts(perigee_ldmx);


      auto perigee_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(perigee_acts)};
Acts::BoundSquareMatrix cov = tracking::sim::utils::unpackCov(seed.getPerigeeCov());


      Acts::BoundTrackParameters seed_pars(perigee_surface, bound_params, cov,
                                           part);


      Acts::Vector3 gen_pos = perigee_surface->center(gctx_);
      Acts::Vector3 target_pos = target_surface->center(gctx_);


      std::optional<Acts::BoundTrackParameters> prop_state;
      if ((gen_pos - target_pos).norm() < 1e-4) {
        prop_state = seed_pars;
      } else {
        prop_state = trk_extrap_->extrapolate(seed_pars, target_surface);
      }


      if (!prop_state) {
        ldmx_log(warn) << "Tagger extrapolation failed for seed with track ID "
                       << seed.getTrackID();
        continue;
      }


      ldmx::Track track;
      track.setTrackID(seed.getTrackID());
      track.setPdgID(seed.getPdgID());
      Acts::Vector3 ref_ldmx = tracking::sim::utils::acts2Ldmx(target_pos);
      track.setPerigeeLocation(ref_ldmx(0), ref_ldmx(1), ref_ldmx(2));
      track.setPerigeeParameters(
          tracking::sim::utils::convertActsToLdmxPars(
              prop_state->parameters()));
      if (prop_state->covariance().has_value()) {
        std::vector<double> cov_vec;
        tracking::sim::utils::flatCov(prop_state->covariance().value(),
                                      cov_vec);
        track.setPerigeeCov(cov_vec);
      } else {
        track.setPerigeeCov(seed.getPerigeeCov());
      }
      track.setNhits(seed.getNhits());
      track.setChi2(0.);
      track.setNdf(0);
      track.setNsharedHits(0);
      track.setTruthProb(1.);


      for (const auto& ts : seed.getTrackStates()) {
        track.addTrackState(ts);
      }


      for (auto sim_hit_idx : seed.getMeasurementsIdxs()) {
        track.addMeasurementIndex(sim_hit_idx);
      }


      tagger_tracks.push_back(track);
    }
  }


  if (!skip_recoil_) {
    for (const auto& seed : recoil_seeds) {
      ldmx::Track track;
      if (seed.getNhits() <= n_min_hits_recoil_) continue;

      //continue as soon as possible if seed is not found
      auto part_it = particle_map.find(seed.getTrackID());
      if (part_it == particle_map.end()) continue;

      ldmx::SimTrackerHit ecal_hit;
      bool found_ecal_hit = false;
      for (const auto& ecal_sp_hit : sel_ecal_sp_hits) {
        if (ecal_sp_hit.getTrackID() == seed.getTrackID()) {
          ecal_hit = ecal_sp_hit;
          found_ecal_hit = true;
          break;
        }
      }
      if (!found_ecal_hit) continue;


      auto seed_mom = seed.getMomentumAtTarget();
      //only continue extrapolation based on these cuts
      if (seed.getPerigeeParameters().empty()) continue;

      if (seed_mom.empty()) continue;

      Acts::Vector3 p_vec{seed_mom[0], seed_mom[1], seed_mom[2]};
       
      // p cut
      if (p_cut_ >= 0. && p_vec.norm() < p_cut_) continue;

      // p cut Max
      if (p_cut_ < 100000. && p_vec.norm() > p_cut_max_) continue;

      // pz cut
       if (pz_cut_ > -9999 && p_vec(2) < pz_cut_) continue;
      //if (p_vec(2) < 0.0) continue;

      Acts::BoundVector bound_params;
      bound_params << seed.getD0(), seed.getZ0(), seed.getPhi(),
          seed.getTheta(), seed.getQoP(), seed.getT();


      auto part{Acts::GenericParticleHypothesis(
          Acts::ParticleHypothesis(Acts::PdgParticle(particle_hypothesis_)))};


      Acts::Vector3 perigee_ldmx(seed.getPerigeeLocation()[0],
                                 seed.getPerigeeLocation()[1],
                                 seed.getPerigeeLocation()[2]);
      auto perigee_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
          tracking::sim::utils::ldmx2Acts(perigee_ldmx))};


      Acts::BoundSquareMatrix cov =
          tracking::sim::utils::unpackCov(seed.getPerigeeCov());


      Acts::BoundTrackParameters seed_pars(perigee_surface, bound_params, cov,
                                           part);


      Acts::Vector3 gen_pos = perigee_surface->center(gctx_);
      Acts::Vector3 target_pos = target_surface->center(gctx_);


      std::optional<Acts::BoundTrackParameters> prop_state;
      if ((gen_pos - target_pos).norm() < 1e-4) {
        prop_state = seed_pars;
      } else {
        prop_state = trk_extrap_->extrapolate(seed_pars, target_surface);
      }


      if (!prop_state) {
        ldmx_log(warn) << "Recoil extrapolation failed for seed with track ID "
                       << seed.getTrackID();
        continue;
      }

      track.setTrackID(seed.getTrackID());
      track.setPdgID(seed.getPdgID());
      Acts::Vector3 ref_ldmx = tracking::sim::utils::acts2Ldmx(target_pos);
      track.setPerigeeLocation(ref_ldmx(0), ref_ldmx(1), ref_ldmx(2));
      track.setPerigeeParameters(
          tracking::sim::utils::convertActsToLdmxPars(
              prop_state->parameters()));
      if (prop_state->covariance().has_value()) {
        std::vector<double> cov_vec;
        tracking::sim::utils::flatCov(prop_state->covariance().value(),
                                      cov_vec);
        track.setPerigeeCov(cov_vec);
      } else {
        track.setPerigeeCov(seed.getPerigeeCov());
      }
      track.setNhits(seed.getNhits());
      track.setChi2(0.);
      track.setNdf(0);
      track.setNsharedHits(0);
      track.setTruthProb(1.);


      for (const auto& ts : seed.getTrackStates()) {
        track.addTrackState(ts);
      }

      for (auto sim_hit_idx : seed.getMeasurementsIdxs()) {
        track.addMeasurementIndex(sim_hit_idx);
      }

  const ldmx::SimParticle& phit = part_it->second;
 
  // Express truth ECAL state in the bound parametrization of ecal_surface
  // (same surface definition used by CKFProcessor) rather than storing raw
  // scoring plane hit coordinates.
    Acts::Vector3 ep{ecal_hit.getPosition()[0], ecal_hit.getPosition()[1],
                     ecal_hit.getPosition()[2]};
    Acts::Vector3 em{ecal_hit.getMomentum()[0], ecal_hit.getMomentum()[1],
                     ecal_hit.getMomentum()[2]};
    ep = tracking::sim::utils::ldmx2Acts(ep);
    em = tracking::sim::utils::ldmx2Acts(em);
    // Linearly extrapolate transverse coordinates to ACTS x = 240.5 mm
    // (= LDMX z = 240.5 mm), correcting for the track slope over the small
    // z-offset between the scoring plane and the ECAL surface.
    if (std::abs(em[0]) > 0) {
      double delta = 240.5 - ep[0];
      ep[1] += delta * em[1] / em[0];
      ep[2] += delta * em[2] / em[0];
      ep[0] = 240.5;
    }
    double q_ecal = phit.getCharge() * Acts::UnitConstants::e;
    auto ecal_free = tracking::sim::utils::toFreeParameters(ep, em, q_ecal);
    auto ecal_bound =
        Acts::transformFreeToBoundParameters(ecal_free, *ecal_surface, gctx_);
    if (ecal_bound.ok()) {
      auto part_ecal{Acts::GenericParticleHypothesis(
          Acts::ParticleHypothesis(Acts::PdgParticle(particle_hypothesis_)))};
      Acts::BoundTrackParameters ecal_pars(ecal_surface, ecal_bound.value(),
                                           Acts::BoundSquareMatrix::Identity(),
                                           part_ecal);
      track.addTrackState(tracking::sim::utils::makeTrackState(
          geometryContext(), ecal_pars, ldmx::AtECAL));
          recoil_tracks.push_back(track);
      }
    }
  }


  event.add(tagger_tracks_collection_, tagger_tracks);
  event.add(recoil_tracks_collection_, recoil_tracks);
}


}  // namespace tracking::reco


DECLARE_PRODUCER(tracking::reco::TruthTrackProcessor)
