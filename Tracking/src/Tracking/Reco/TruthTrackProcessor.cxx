#include "Tracking/Reco/TruthTrackProcessor.h"


#include "Tracking/Sim/GeometryContainers.h"


namespace tracking::reco {


TruthTrackProcessor::TruthTrackProcessor(const std::string& name,
                                         framework::Process& process)
    : TrackingGeometryUser(name, process) {}


void TruthTrackProcessor::onNewRun(const ldmx::RunHeader& rh) {
  gctx_ = Acts::GeometryContext();
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);

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
  input_tagger_truth_collection_ =
      parameters.get<std::string>("input_tagger_truth_collection");
  input_recoil_truth_collection_ =
      parameters.get<std::string>("input_recoil_truth_collection");
  input_beam_electrons_collection_ =
      parameters.get<std::string>("input_beam_electrons_collection");

  seed_smearing_ = parameters.get<bool>("seedSmearing", false);

  ldmx_log(info) << "Seed Smearing is set to " << seed_smearing_;

  d0smear_ = parameters.get<std::vector<double>>("d0smear", {0.01, 0.01, 0.01});
  z0smear_ = parameters.get<std::vector<double>>("z0smear", {0.1, 0.1, 0.1});
  phismear_ = parameters.get<double>("phismear", 0.001);
  thetasmear_ = parameters.get<double>("thetasmear", 0.001);
  relpsmear_ = parameters.get<double>("relpsmear", 0.1);

  // Relative smear factor terms, only used if seed_smearing_ is true.
  rel_smearfactors_ = parameters.get<std::vector<double>>(
      "rel_smearfactors", {0.1, 0.1, 0.1, 0.1, 0.1, 0.1});
  inflate_factors_ = parameters.get<std::vector<double>>(
      "inflate_factors", {10., 10., 10., 10., 10., 10.});

  // In tracking frame: where do these numbers come from?
  // These numbers come from approximating the path of the beam up
  // until it is about to enter the first detector volume (TriggerPad1).
  // In detector coordinates, (x_,y_,z_) = (-21.7, -883) is
  // where the beam arrives (if no smearing is applied) and we simply
  // reorder these values so that they are in tracking coordinates.
  beam_origin_ = parameters.get<std::vector<double>>("beamOrigin",
                                                     {-883.0, -21.745876, 0.0});

  // Skip the tagger or recoil trackers if wanted
  skip_tagger_ = parameters.get<bool>("skip_tagger", false);
  skip_recoil_ = parameters.get<bool>("skip_recoil", false);
  particle_hypothesis_ = parameters.get<int>("particle_hypothesis");

  beam_electrons_collection_ =
      parameters.get<std::string>("beam_electrons_collection");
  tagger_truth_collection_ =
      parameters.get<std::string>("tagger_truth_collection");
  recoil_truth_collection_ =
      parameters.get<std::string>("recoil_truth_collection");
  tagger_seeds_collection_ =
      parameters.get<std::string>("tagger_seeds_collection");
  recoil_seeds_collection_ =
      parameters.get<std::string>("recoil_seeds_collection");
  // Get the field map for the propagator
  field_map_ = parameters.get<std::string>("field_map");
}


void TruthTrackProcessor::createTruthTrack(
    const ldmx::Track::TrackState& ts,
    ldmx::Track& trk,
    const std::shared_ptr<Acts::Surface>& target_surface) {

  // Rotate the position and momentum into the ACTS frame.
  Acts::Vector3 pos_ldmx{ts.pos_[0], ts.pos_[1], ts.pos_[2]};
  Acts::Vector3 mom_ldmx{ts.mom_[0], ts.mom_[1], ts.mom_[2]};
  Acts::Vector3 pos = tracking::sim::utils::ldmx2Acts(pos_ldmx);
  Acts::Vector3 mom = tracking::sim::utils::ldmx2Acts(mom_ldmx);

  // Get the charge of the particle.
  int pdg = trk.getPdgID();
  //find charge from pdg already attached to track
  int charge = (pdg == 11 || pdg == 13 || pdg == -211 || pdg == -2212) ? -1 : 1;
  double q{charge * Acts::UnitConstants::e};

  // The idea here is:
  // 1 - Define a bound track state parameters at point P on track. Basically a
  // curvilinear representation.
  // 2 - Propagate to target surface to obtain the
  // BoundTrackState there.

  // Transform the position, momentum and charge to free parameters.
  auto free_params{tracking::sim::utils::toFreeParameters(pos, mom, q)};

  // Create a line surface at the perigee.  The perigee position is extracted
  // from a particle's vertex or the particle's position at a specific
  // scoring plane.
  auto gen_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3(free_params[Acts::eFreePos0], free_params[Acts::eFreePos1],
                    free_params[Acts::eFreePos2]))};

  // ldmx_log(trace)<<"PF:: gen_surface"<<free_params[Acts::eFreePos0]<<"
  // " <<free_params[Acts::eFreePos1]<<" " <<free_params[Acts::eFreePos2];

  // Transform the parameters to local positions on the perigee surface.
  auto bound_params{
      Acts::transformFreeToBoundParameters(free_params, *gen_surface, gctx_)
          .value()};
  // Create a particle hypothesis
  auto part{Acts::GenericParticleHypothesis(
      Acts::ParticleHypothesis(Acts::PdgParticle(particle_hypothesis_)))};
  Acts::BoundTrackParameters bound_trk_pars(gen_surface, bound_params,
                                            std::nullopt, part);

  auto prop_bound_state =
      trk_extrap_->extrapolate(bound_trk_pars, target_surface);

  if (!prop_bound_state) {
    ldmx_log(warn) << "Propagation to target surface failed — "
                   << "track may have exited the B-field map.";
    return;
  }

  // Create the seed track object.
  Acts::Vector3 ref = target_surface->center(geometryContext());
  Acts::Vector3 ref_ldmx = tracking::sim::utils::acts2Ldmx(ref);
  trk.setPerigeeLocation(ref_ldmx(0), ref_ldmx(1), ref_ldmx(2));

  auto prop_bound_vec = prop_bound_state->parameters();

  trk.setPerigeeParameters(
      tracking::sim::utils::convertActsToLdmxPars(prop_bound_vec));
}

ldmx::Track TruthTrackProcessor::taggerFullSeed(
    const ldmx::Track& beam_electron, const int trackID,
    const std::shared_ptr<Acts::Surface>& origin_surface,
    const std::shared_ptr<Acts::Surface>& target_surface) {

  //find correct target and beam origin states
  ldmx::Track::TrackState target_state;
  ldmx::Track::TrackState beam_origin_state;
  bool found_target = false;
  bool found_beam_origin = false;

  for (const auto& ts : beam_electron.getTrackStates()) {
    if (ts.ts_type_ == ldmx::AtTarget) {
      target_state = ts;
      found_target = true;
    }
    if (ts.ts_type_ == ldmx::AtBeamOrigin) {
      beam_origin_state = ts;
      found_beam_origin = true;
    }
  }

  if (!found_target) {
    ldmx_log(error) << "Beam electron track at: " << beam_electron.getTrackID() << " missing AtTarget state, skipping";
    return ldmx::Track();
  }

  if (!found_beam_origin) {
    ldmx_log(error) << "Beam electron track at: " << beam_electron.getTrackID() << " missing AtBeamOrigin state, skipping";
    return ldmx::Track();
  }

  ldmx::Track truth_track;
  truth_track.setTrackID(trackID);
  truth_track.setPdgID(beam_electron.getPdgID());

  createTruthTrack(beam_origin_state, truth_track, origin_surface);

  // Smeared track at the beam origin
  ldmx::Track smeared_truth_track = seedFromTruth(truth_track, true);

  // Copy track states and measurement indices from beam_electron to smeared_truth_track
  for (const auto& ts : beam_electron.getTrackStates()) {
    smeared_truth_track.addTrackState(ts);
  }
  for (auto idx : beam_electron.getMeasurementsIdxs()) {
    smeared_truth_track.addMeasurementIndex(idx);
  }

  ldmx_log(debug) << "Truth parameters at beam origin";
  for (auto par : truth_track.getPerigeeParameters())
    ldmx_log(debug) << par << " ";
  ldmx_log(debug);


  ldmx_log(debug) << "Smeared parameters at origin";
  for (auto par : smeared_truth_track.getPerigeeParameters())
    ldmx_log(debug) << par << " ";

  return smeared_truth_track;
}

ldmx::Track TruthTrackProcessor::seedFromTruth(const ldmx::Track& tt,
                                              bool seed_smearing) {
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

  if (seed_smearing) {
    ldmx_log(debug) << "Smear track and inflate covariance";

    /*
      double sigma_d0     = rel_smearfactors_[Acts::eBoundLoc0]   * tt.getD0();
      double sigma_z0     = rel_smearfactors_[Acts::eBoundLoc1]   * tt.getZ0();
      double sigma_phi    = rel_smearfactors_[Acts::eBoundPhi]    * tt.getPhi();
      double sigma_theta  = rel_smearfactors_[Acts::eBoundTheta]  *
      tt.getTheta(); double sigma_p      = rel_smearfactors_[Acts::eBoundQOverP]
      * abs(1/tt.getQoP()); double sigma_t      =
      rel_smearfactors_[Acts::eBoundTime]   * tt.getT();
    */

    double sigma_d0 = d0smear_[0];
    double sigma_z0 = z0smear_[0];
    double sigma_phi = phismear_;
    double sigma_theta = thetasmear_;
    double sigma_p = relpsmear_ * abs(1 / tt.getQoP());
    double sigma_t = 1. * Acts::UnitConstants::ns;

    double smear = (*normal_)(generator_);
    double d0smear = tt.getD0() + smear * sigma_d0;

    smear = (*normal_)(generator_);
    double z0smear = tt.getZ0() + smear * sigma_z0;

    smear = (*normal_)(generator_);
    double phismear = tt.getPhi() + smear * sigma_phi;

    smear = (*normal_)(generator_);
    double thetasmear = tt.getTheta() + smear * sigma_theta;

    double p = std::abs(1. / tt.getQoP());
    smear = (*normal_)(generator_);
    double psmear = p + smear * sigma_p;

    double q = tt.getQoP() < 0 ? -1. : 1.;
    double qo_psmear = q / psmear;

    smear = (*normal_)(generator_);
    double tsmear = tt.getT() + smear * sigma_t;

    bound_params << d0smear, z0smear, phismear, thetasmear, qo_psmear, tsmear;

    stddev[Acts::eBoundLoc0] =
        inflate_factors_[Acts::eBoundLoc0] * sigma_d0 * Acts::UnitConstants::mm;
    stddev[Acts::eBoundLoc1] =
        inflate_factors_[Acts::eBoundLoc1] * sigma_z0 * Acts::UnitConstants::mm;
    stddev[Acts::eBoundPhi] = inflate_factors_[Acts::eBoundPhi] * sigma_phi;
    stddev[Acts::eBoundTheta] =
        inflate_factors_[Acts::eBoundTheta] * sigma_theta;
    stddev[Acts::eBoundQOverP] =
        inflate_factors_[Acts::eBoundQOverP] * (1. / p) * (1. / p) * sigma_p;
    stddev[Acts::eBoundTime] =
        inflate_factors_[Acts::eBoundTime] * sigma_t * Acts::UnitConstants::ns;

    ldmx_log(debug) << stddev;

    std::vector<double> v_seed_params(
        (bound_params).data(),
        bound_params.data() + bound_params.rows() * bound_params.cols());

    Acts::BoundSquareMatrix bound_cov =
        stddev.cwiseProduct(stddev).asDiagonal();
    std::vector<double> v_seed_cov;
    tracking::sim::utils::flatCov(bound_cov, v_seed_cov);
    seed.setPerigeeParameters(v_seed_params);
    seed.setPerigeeCov(v_seed_cov);

  } else {
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

    Acts::BoundSquareMatrix bound_cov =
        stddev.cwiseProduct(stddev).asDiagonal();
    std::vector<double> v_seed_cov;
    tracking::sim::utils::flatCov(bound_cov, v_seed_cov);
    seed.setPerigeeParameters(v_seed_params);
    seed.setPerigeeCov(v_seed_cov);
  }

  for (const auto& ts : tt.getTrackStates()) {
    seed.addTrackState(ts);
  }
  for (auto idx : tt.getMeasurementsIdxs()) {
    seed.addMeasurementIndex(idx);
  }

  return seed;
}


void TruthTrackProcessor::produce(framework::Event& event) {
  // Retrieve the tagger seeds from TruthSeedProcessor
  const auto& tagger_seeds = event.getCollection<ldmx::Track>(
      input_tagger_truth_collection_, input_pass_name_);

  // Retrieve the recoil seeds from TruthSeedProcessor
  const auto& recoil_seeds = event.getCollection<ldmx::Track>(
      input_recoil_truth_collection_, input_pass_name_);

  // Retrieve the beam electrons from TruthSeedProcessor
  const auto& beam_electrons_in = event.getCollection<ldmx::Track>(
      input_beam_electrons_collection_, input_pass_name_);

  // Building of the event truth information and the truth seeds
  // TODO remove the truthtracks in the future as the truth seeds are enough

  std::vector<ldmx::Track> tagger_truth_tracks;
  std::vector<ldmx::Track> tagger_truth_seeds;
  std::vector<ldmx::Track> recoil_truth_tracks;
  std::vector<ldmx::Track> recoil_truth_seeds;
  std::vector<ldmx::Track> beam_electrons;

  // TODO:: The target should be taken from some conditions DB in the future.
  // Define the perigee_surface at 0.0.0
  auto target_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3(0., 0., 0.))};

  // Define the target_surface
  auto target_unbound_surface = tracking::sim::utils::unboundSurface(0.);

  auto beam_origin_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3(beam_origin_[0], beam_origin_[1], beam_origin_[2]))};


  if (!skip_tagger_) {
    for (const auto& trk : tagger_seeds) {
      //find the track state at the target surface
      bool found_target_state=false;
      ldmx::Track::TrackState target_state;

      for(const auto& ts: trk.getTrackStates())
      {
        if(ts.ts_type_ == ldmx::AtTarget)
        {
          target_state = ts;
          found_target_state = true;
          break;
        }
      }

      if(!found_target_state)
      {
        ldmx_log(warn) << "track ID" << trk.getTrackID() << "missing AtTarget state, skipping.";
        continue;
      }

      ldmx::Track truth_trk = trk;
      createTruthTrack(target_state, truth_trk, target_surface);
      tagger_truth_tracks.push_back(truth_trk);
    }

    for (const auto& trk : beam_electrons_in) {
      bool found_target_state=false;
      ldmx::Track::TrackState target_state;

      for(const auto& ts: trk.getTrackStates())
      {
        if(ts.ts_type_ == ldmx::AtTarget)
        {
          target_state = ts;
          found_target_state = true;
          break;
        }
      }

      if(!found_target_state)
      {
        ldmx_log(warn) << "beam electron track ID" << trk.getTrackID() << "missing AtTarget state, skipping.";
        continue;
      }

      ldmx::Track truth_trk = trk;
      createTruthTrack(target_state, truth_trk, target_surface);

      ldmx::Track beam_e_truth_seed =
          taggerFullSeed(truth_trk, trk.getTrackID(),
                         beam_origin_surface,
                         target_unbound_surface);
      beam_electrons.push_back(beam_e_truth_seed);
    }
  }

    // Findable particle selection
    // ldmx_log(trace) << "!!! n_recoil_sim_hits found: " <<
    // hit_count_map_recoil[hit.getTrackID()].size();
    if (!skip_recoil_) {
      for(const auto& trk: recoil_seeds)
      {
        bool found_target_state=false;
        ldmx::Track::TrackState target_state;

        for(const auto& ts: trk.getTrackStates())
        {
          if(ts.ts_type_ == ldmx::AtTarget)
          {
            target_state = ts;
            found_target_state = true;
            break;
          }
        }

        if(!found_target_state)
        {
          ldmx_log(warn) << "track ID" << trk.getTrackID() << "missing AtTarget state, skipping.";
          continue;
        }
        ldmx::Track truth_trk = trk;
        createTruthTrack(target_state, truth_trk, target_surface);

        recoil_truth_tracks.push_back(truth_trk);
      }
    }

  /*
    for (std::pair<int,std::vector<int>> element : recoil_sh_count_map) {

    const ldmx::SimTrackerHit& hit  = scoring_hits.at(element.second.at(0));
    const ldmx::SimParticle&   phit = particleMap[hit.getTrackID()];

    if (hit_count_map_recoil[hit.getTrackID()].size() > n_min_hits_recoil_) {
    ldmx::Track truth_recoil_track;
    createTruthTrack(phit,hit,truth_recoil_track,targetSurface);
    truth_recoil_track.setNhits(hit_count_map_recoil[hit.getTrackID()].size());
    recoil_truth_tracks.push_back(truth_recoil_track);
    }
    }
  */

  // Form a truth seed from a truth track

  for (auto& tt : tagger_truth_tracks) {
    ldmx::Track seed = seedFromTruth(tt, seed_smearing_);

    tagger_truth_seeds.push_back(seed);
  }

  ldmx_log(debug) << "Forming seeds from truth";
  for (auto& tt : recoil_truth_tracks) {
    ldmx_log(debug) << "Smearing truth track";

    ldmx::Track seed = seedFromTruth(tt, seed_smearing_);

    recoil_truth_seeds.push_back(seed);
  }

  // even if skip_tagger/recoil_ is true, still make the collections in the
  // event
  event.add(beam_electrons_collection_, beam_electrons);
  event.add(tagger_truth_collection_, tagger_truth_tracks);
  event.add(recoil_truth_collection_, recoil_truth_tracks);
  event.add(tagger_seeds_collection_, tagger_truth_seeds);
  event.add(recoil_seeds_collection_, recoil_truth_seeds);
}

}  // namespace tracking::reco


DECLARE_PRODUCER(tracking::reco::TruthTrackProcessor)