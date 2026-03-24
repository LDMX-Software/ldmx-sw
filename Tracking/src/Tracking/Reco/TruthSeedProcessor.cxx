#include "Tracking/Reco/TruthSeedProcessor.h"

#include "Tracking/Sim/GeometryContainers.h"

namespace tracking::reco {

TruthSeedProcessor::TruthSeedProcessor(const std::string& name,
                                       framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void TruthSeedProcessor::onNewRun(const ldmx::RunHeader& rh) {
  gctx_ = Acts::GeometryContext();
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);

  Acts::StraightLineStepper stepper;
  Acts::Navigator::Config nav_cfg{geometry().getTG()};
  const Acts::Navigator navigator(nav_cfg);

  linpropagator_ = std::make_shared<LinPropagator>(stepper, navigator);
  trk_extrap_ = std::make_shared<std::decay_t<decltype(*trk_extrap_)>>(
      *linpropagator_, geometryContext(), magneticFieldContext());
}

void TruthSeedProcessor::configure(framework::config::Parameters& parameters) {
  scoring_hits_coll_name_ =
      parameters.get<std::string>("scoring_hits_coll_name");
  ecal_sp_coll_name_ = parameters.get<std::string>("ecal_sp_coll_name");
  sp_pass_name_ = parameters.get<std::string>("sp_pass_name");
  recoil_sim_hits_coll_name_ =
      parameters.get<std::string>("recoil_sim_hits_coll_name");
  tagger_sim_hits_coll_name_ =
      parameters.get<std::string>("tagger_sim_hits_coll_name");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  sim_particles_coll_name_ =
      parameters.get<std::string>("sim_particles_coll_name");
  sim_particles_passname_ =
      parameters.get<std::string>("sim_particles_passname");

  n_min_hits_tagger_ = parameters.get<int>("n_min_hits_tagger", 11);
  n_min_hits_recoil_ = parameters.get<int>("n_min_hits_recoil", 7);
  pdg_ids_ = parameters.get<std::vector<int>>("pdg_ids", {11});
  z_min_ = parameters.get<double>("z_min", -9999);  // mm
  track_id_ = parameters.get<int>("track_id", -9999);
  pz_cut_ = parameters.get<double>("pz_cut", -9999);  // MeV
  p_cut_ = parameters.get<double>("p_cut", 0.);
  p_cut_max_ = parameters.get<double>("p_cut_max", 100000.);  // MeV
  p_cut_ecal_ = parameters.get<double>("p_cut_ecal", -1.);    // MeV
  recoil_sp_ = parameters.get<double>("recoil_sp", true);
  target_sp_ = parameters.get<double>("tagger_sp", true);
  seed_smearing_ = parameters.get<bool>("seedSmearing", false);
  max_track_id_ = parameters.get<int>("max_track_id", 5);

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
}

void TruthSeedProcessor::createTruthTrack(
    const ldmx::SimParticle& particle, const ldmx::SimTrackerHit& hit,
    ldmx::Track& trk, const std::shared_ptr<Acts::Surface>& target_surface) {
  std::vector<double> pos{static_cast<double>(hit.getPosition()[0]),
                          static_cast<double>(hit.getPosition()[1]),
                          static_cast<double>(hit.getPosition()[2])};
  createTruthTrack(pos, hit.getMomentum(), particle.getCharge(), trk,
                   target_surface);

  trk.setTrackID(hit.getTrackID());
  trk.setPdgID(hit.getPdgID());
}

void TruthSeedProcessor::createTruthTrack(
    const ldmx::SimParticle& particle, ldmx::Track& trk,
    const std::shared_ptr<Acts::Surface>& target_surface) {
  createTruthTrack(particle.getVertex(), particle.getMomentum(),
                   particle.getCharge(), trk, target_surface);

  trk.setPdgID(particle.getPdgID());
}

void TruthSeedProcessor::createTruthTrack(
    const std::vector<double>& pos_vec, const std::vector<double>& p_vec,
    int charge, ldmx::Track& trk,
    const std::shared_ptr<Acts::Surface>& target_surface) {
  // Get the position and momentum of the particle at the point of creation.
  // This only works for the incident electron when creating a tagger tracker
  // seed. For the recoil tracker, the scoring plane position will need to
  // be used.  For other particles created in the target or tracker planes,
  // this version of the method can also be used.
  // These are just Eigen vectors defined as
  // Eigen::Matrix<double, kSize, 1>;
  Acts::Vector3 pos{pos_vec[0], pos_vec[1], pos_vec[2]};
  Acts::Vector3 mom{p_vec[0], p_vec[1], p_vec[2]};

  // Rotate the position and momentum into the ACTS frame.
  pos = tracking::sim::utils::ldmx2Acts(pos);
  mom = tracking::sim::utils::ldmx2Acts(mom);

  // Get the charge of the particle.
  // TODO: Add function that uses the PDG ID to calculate this.
  double q{charge * Acts::UnitConstants::e};

  // The idea here is:
  // 1 - Define a bound track state parameters at point P on track. Basically a
  // curvilinear representation. 2 - Propagate to target surface to obtain the
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

  // CAUTION:: The target surface should be close to the gen surface
  // Linear propagation to the target surface. I assume 1mm of tolerance
  Acts::Vector3 tgt_surf_center = target_surface->center(geometryContext());
  Acts::Vector3 gen_surf_center = gen_surface->center(geometryContext());
  // Tolerance
  double tol = 1;  // mm

  if (abs(tgt_surf_center(0) - gen_surf_center(0)) > tol)
    ldmx_log(error) << "Linear extrapolation to a far away surface in B field."
                    << "  This will cause inaccuracies in track parameters"
                    << "  Distance extrapolated = "
                    << (tgt_surf_center(0) - gen_surf_center(0));

  auto prop_bound_state =
      trk_extrap_->extrapolate(bound_trk_pars, target_surface);

  // Create the seed track object.
  Acts::Vector3 ref = target_surface->center(geometryContext());
  Acts::Vector3 ref_ldmx = tracking::sim::utils::acts2Ldmx(ref);
  trk.setPerigeeLocation(ref_ldmx(0), ref_ldmx(1), ref_ldmx(2));

  auto prop_bound_vec = (prop_bound_state.value()).parameters();

  trk.setPerigeeParameters(
      tracking::sim::utils::convertActsToLdmxPars(prop_bound_vec));
}

// origin_surface is the perigee
// target_surface is the ecal
ldmx::Track TruthSeedProcessor::recoilFullSeed(
    const ldmx::SimParticle& particle, const int trackID,
    const ldmx::SimTrackerHit& hit, const ldmx::SimTrackerHit& ecal_hit,
    const std::map<int, std::vector<int>>& hit_count_map,
    const std::shared_ptr<Acts::Surface>& origin_surface,
    const std::shared_ptr<Acts::Surface>& target_surface,
    const std::shared_ptr<Acts::Surface>& ecal_surface) {
  ldmx::Track truth_recoil_track;
  createTruthTrack(particle, hit, truth_recoil_track, origin_surface);
  truth_recoil_track.setTrackID(trackID);

  // Seed at the target location
  ldmx::Track smeared_truth_track = seedFromTruth(truth_recoil_track, false);

  // Add truth track state at the target: use scoring plane hit (already LDMX
  // frame)
  ldmx::Track::TrackState ts_truth_target;
  ts_truth_target.pos_ = {hit.getPosition()[0], hit.getPosition()[1],
                          hit.getPosition()[2]};
  ts_truth_target.mom_ = {hit.getMomentum()[0], hit.getMomentum()[1],
                          hit.getMomentum()[2]};
  ts_truth_target.ts_type_ = ldmx::AtTarget;
  smeared_truth_track.addTrackState(ts_truth_target);

  // Express truth ECAL state in the bound parametrization of ecal_surface
  // (same surface definition used by CKFProcessor) rather than storing raw
  // scoring plane hit coordinates.
  {
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
    double q_ecal = particle.getCharge() * Acts::UnitConstants::e;
    auto ecal_free = tracking::sim::utils::toFreeParameters(ep, em, q_ecal);
    auto ecal_bound =
        Acts::transformFreeToBoundParameters(ecal_free, *ecal_surface, gctx_);
    if (ecal_bound.ok()) {
      auto part{Acts::GenericParticleHypothesis(
          Acts::ParticleHypothesis(Acts::PdgParticle(particle_hypothesis_)))};
      Acts::BoundTrackParameters ecal_pars(ecal_surface, ecal_bound.value(),
                                           Acts::BoundSquareMatrix::Identity(),
                                           part);
      smeared_truth_track.addTrackState(tracking::sim::utils::makeTrackState(
          geometryContext(), ecal_pars, ldmx::AtECAL));
    }
  }

  // Add the hits
  int nhits = 0;

  for (auto sim_hit_idx : hit_count_map.at(smeared_truth_track.getTrackID())) {
    smeared_truth_track.addMeasurementIndex(sim_hit_idx);
    nhits += 1;
  }

  smeared_truth_track.setNhits(nhits);

  return smeared_truth_track;
}

ldmx::Track TruthSeedProcessor::taggerFullSeed(
    const ldmx::SimParticle& beam_electron, const int trackID,
    const ldmx::SimTrackerHit& hit,
    const std::map<int, std::vector<int>>& hit_count_map,
    const std::shared_ptr<Acts::Surface>& origin_surface,
    const std::shared_ptr<Acts::Surface>& target_surface) {
  ldmx::Track truth_track;

  createTruthTrack(beam_electron, truth_track, origin_surface);
  truth_track.setTrackID(trackID);

  // Smeared track at the beam origin
  ldmx::Track smeared_truth_track = seedFromTruth(truth_track, true);

  ldmx_log(debug) << "Truth parameters at beam origin";
  for (auto par : truth_track.getPerigeeParameters())
    ldmx_log(debug) << par << " ";
  ldmx_log(debug);

  // Add the truth track state at the target using the scoring plane hit
  // (position and momentum are already in LDMX global frame)
  ldmx::Track::TrackState ts_truth_target;
  ts_truth_target.pos_ = {hit.getPosition()[0], hit.getPosition()[1],
                          hit.getPosition()[2]};
  ts_truth_target.mom_ = {hit.getMomentum()[0], hit.getMomentum()[1],
                          hit.getMomentum()[2]};
  ts_truth_target.ts_type_ = ldmx::AtTarget;
  smeared_truth_track.addTrackState(ts_truth_target);

  ldmx_log(debug) << "Truth position at target: " << hit.getPosition()[0] << " "
                  << hit.getPosition()[1] << " " << hit.getPosition()[2];

  // Add the truth track state at the beam origin using particle vertex/momentum
  // (SimParticle vertex and momentum are in LDMX global frame)
  ldmx::Track::TrackState ts_truth_beam_origin;
  ts_truth_beam_origin.pos_ = {beam_electron.getVertex()[0],
                               beam_electron.getVertex()[1],
                               beam_electron.getVertex()[2]};
  ts_truth_beam_origin.mom_ = {beam_electron.getMomentum()[0],
                               beam_electron.getMomentum()[1],
                               beam_electron.getMomentum()[2]};
  ts_truth_beam_origin.ts_type_ = ldmx::AtBeamOrigin;
  smeared_truth_track.addTrackState(ts_truth_beam_origin);

  ldmx_log(debug) << "Smeared parameters at origin";
  for (auto par : smeared_truth_track.getPerigeeParameters())
    ldmx_log(debug) << par << " ";

  // assign the sim hit indices
  // TODO this is not fully correct as the sim hits
  // might be duplicated on sensors
  // and should be merged if that is the case

  int nhits = 0;

  for (auto sim_hit_idx : hit_count_map.at(smeared_truth_track.getTrackID())) {
    smeared_truth_track.addMeasurementIndex(sim_hit_idx);
    nhits += 1;
  }

  smeared_truth_track.setNhits(nhits);

  return smeared_truth_track;
}

ldmx::Track TruthSeedProcessor::seedFromTruth(const ldmx::Track& tt,
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

  return seed;
}

void TruthSeedProcessor::makeHitCountMap(
    const std::vector<ldmx::SimTrackerHit>& sim_hits,
    std::map<int, std::vector<int>>& hit_count_map) {
  for (int i_sim_hit = 0; i_sim_hit < sim_hits.size(); i_sim_hit++) {
    auto& sim_hit = sim_hits.at(i_sim_hit);
    // This track never left a hit before
    if (!hit_count_map.count(sim_hit.getTrackID())) {
      hit_count_map[sim_hit.getTrackID()].push_back(i_sim_hit);
    }

    // This track left a hit before.
    // Check if it's on a different sensor than the others

    else {
      int sensor_id = tracking::sim::utils::getSensorID(sim_hit);
      bool found_hit = false;

      for (auto& i_rhit : hit_count_map[sim_hit.getTrackID()]) {
        int tmp_sensor_id =
            tracking::sim::utils::getSensorID(sim_hits.at(i_rhit));

        if (sensor_id == tmp_sensor_id) {
          found_hit = true;
          break;
        }
      }  // loop on the already recorded hits

      if (!found_hit) {
        hit_count_map[sim_hit.getTrackID()].push_back(i_sim_hit);
      }
    }
  }  // loop on sim hits
}

bool TruthSeedProcessor::scoringPlaneHitFilter(
    const ldmx::SimTrackerHit& hit,
    const std::vector<ldmx::SimTrackerHit>& ecal_sp_hits) {
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

    for (auto& e_sp_hit : ecal_sp_hits) {
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
    }  // loop on Ecal scoring plane hits
  }  // pcutEcal

  if (!pass_ecal_scoring_plane) return false;

  return true;
}

void TruthSeedProcessor::produce(framework::Event& event) {
  // Retrieve the particleMap
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      sim_particles_coll_name_, sim_particles_passname_)};

  // Retrieve the target scoring hits
  // Information is extracted using the
  // scoring plane hit left by the particle at the target.

  const std::vector<ldmx::SimTrackerHit> scoring_hits{
      event.getCollection<ldmx::SimTrackerHit>(scoring_hits_coll_name_,
                                               sp_pass_name_)};

  // Retrieve the scoring plane hits at the ECAL
  const std::vector<ldmx::SimTrackerHit> scoring_hits_ecal{
      event.getCollection<ldmx::SimTrackerHit>(ecal_sp_coll_name_,
                                               sp_pass_name_)};

  // Retrieve the sim hits in the tagger tracker
  const std::vector<ldmx::SimTrackerHit> tagger_sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(tagger_sim_hits_coll_name_,
                                               input_pass_name_);

  // Retrieve the sim hits in the recoil tracker
  const std::vector<ldmx::SimTrackerHit> recoil_sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(recoil_sim_hits_coll_name_,
                                               input_pass_name_);

  // If sim hit collections are empty throw a warning
  if (tagger_sim_hits.size() == 0 && !skip_tagger_) {
    ldmx_log(error) << "Tagger sim hits collection empty for event ";
  }
  if (recoil_sim_hits.size() == 0 && !skip_recoil_) {
    ldmx_log(error) << "Recoil sim hits collection empty for event ";
  }

  // The map stores which track leaves which sim hits
  std::map<int, std::vector<int>> hit_count_map_recoil;
  makeHitCountMap(recoil_sim_hits, hit_count_map_recoil);

  std::map<int, std::vector<int>> hit_count_map_tagger;
  makeHitCountMap(tagger_sim_hits, hit_count_map_tagger);

  // to keep track of how many sim particles leave hits on the scoring plane
  std::vector<int> recoil_sh_idxs;
  std::unordered_map<int, std::vector<int>> recoil_sh_count_map;

  std::vector<int> tagger_sh_idxs;
  std::unordered_map<int, std::vector<int>> tagger_sh_count_map;

  // Target scoring hits for Tagger will have Z<0, Recoil scoring hits will
  // have Z>0
  for (unsigned int i_sh = 0; i_sh < scoring_hits.size(); i_sh++) {
    const ldmx::SimTrackerHit& hit = scoring_hits.at(i_sh);
    double zhit = hit.getPosition()[2];

    Acts::Vector3 p_vec{hit.getMomentum()[0], hit.getMomentum()[1],
                        hit.getMomentum()[2]};
    double tagger_p_max = 0.;

    // Check if it is a tagger track going fwd that passes basic cuts
    if (zhit < 0.) {
      // Tagger selection cuts
      // Negative scoring plane hit, with momentum > p_cut
      if (p_vec(2) < 0. || p_vec.norm() < p_cut_) continue;

      // Check that the hit was left by a charged particle
      if (abs(particle_map[hit.getTrackID()].getCharge()) < 1e-8) continue;

      if (p_vec.norm() > tagger_p_max) {
        tagger_sh_count_map[hit.getTrackID()].push_back(i_sh);
      }
    }  // Tagger loop

    // Check the recoil hits
    else {
      // Recoil selection cuts
      // Positive scoring plane hit, forward direction with momentum > p_cut
      if (p_vec(2) < 0. || p_vec.norm() < p_cut_) continue;

      // Check that the hit was left by a charged particle
      if (abs(particle_map[hit.getTrackID()].getCharge()) < 1e-8) continue;

      recoil_sh_count_map[hit.getTrackID()].push_back(i_sh);

    }  // Recoil
  }  // loop on Target scoring plane hits

  for (std::pair<int, std::vector<int>> element : recoil_sh_count_map) {
    std::sort(
        element.second.begin(), element.second.end(),
        [&](const int idx1, int idx2) -> bool {
          const ldmx::SimTrackerHit& hit1 = scoring_hits.at(idx1);
          const ldmx::SimTrackerHit& hit2 = scoring_hits.at(idx2);

          Acts::Vector3 phit1{hit1.getMomentum()[0], hit1.getMomentum()[1],
                              hit1.getMomentum()[2]};
          Acts::Vector3 phit2{hit2.getMomentum()[0], hit2.getMomentum()[1],
                              hit2.getMomentum()[2]};

          return phit1.norm() > phit2.norm();
        });
  }

  // Sort tagger hits.
  for (auto& [_track_id, hit_indices] : tagger_sh_count_map) {
    std::sort(
        hit_indices.begin(), hit_indices.end(),
        [&](const int idx1, int idx2) -> bool {
          const ldmx::SimTrackerHit& hit1 = scoring_hits.at(idx1);
          const ldmx::SimTrackerHit& hit2 = scoring_hits.at(idx2);

          Acts::Vector3 phit1{hit1.getMomentum()[0], hit1.getMomentum()[1],
                              hit1.getMomentum()[2]};
          Acts::Vector3 phit2{hit2.getMomentum()[0], hit2.getMomentum()[1],
                              hit2.getMomentum()[2]};

          return phit1.norm() > phit2.norm();
        });
  }

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

  // ecal
  auto ecal_surface = tracking::sim::utils::unboundSurface(240.5);

  auto beam_origin_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
      Acts::Vector3(beam_origin_[0], beam_origin_[1], beam_origin_[2]))};

  if (!skip_tagger_) {
    for (const auto& [track_id, hit_indices] : tagger_sh_count_map) {
      const ldmx::SimTrackerHit& hit = scoring_hits.at(hit_indices.at(0));
      const ldmx::SimParticle& phit = particle_map[hit.getTrackID()];

      if (hit_count_map_tagger[hit.getTrackID()].size() > n_min_hits_tagger_) {
        ldmx::Track truth_tagger_track;
        createTruthTrack(phit, hit, truth_tagger_track, target_surface);
        truth_tagger_track.setNhits(
            hit_count_map_tagger[hit.getTrackID()].size());
        // Add AtTarget state from the scoring plane hit (pos in mm, mom in MeV,
        // both already in LDMX global frame)
        ldmx::Track::TrackState ts_target;
        ts_target.pos_ = {hit.getPosition()[0], hit.getPosition()[1],
                          hit.getPosition()[2]};
        ts_target.mom_ = {hit.getMomentum()[0], hit.getMomentum()[1],
                          hit.getMomentum()[2]};
        ts_target.ts_type_ = ldmx::AtTarget;
        truth_tagger_track.addTrackState(ts_target);
        tagger_truth_tracks.push_back(truth_tagger_track);

        if (hit.getPdgID() == 11 && hit.getTrackID() < max_track_id_) {
          ldmx::Track beam_e_truth_seed =
              taggerFullSeed(particle_map[hit.getTrackID()], hit.getTrackID(),
                             hit, hit_count_map_tagger, beam_origin_surface,
                             target_unbound_surface);
          beam_electrons.push_back(beam_e_truth_seed);
        }
      }
    }
  }

  // Recover the EcalScoring hits
  std::vector<ldmx::SimTrackerHit> ecal_sp_hits =
      event.getCollection<ldmx::SimTrackerHit>(ecal_sp_coll_name_,
                                               sp_pass_name_);
  // Select ECAL hits
  std::vector<ldmx::SimTrackerHit> sel_ecal_sp_hits;

  for (auto sp_hit : ecal_sp_hits) {
    if (sp_hit.getMomentum()[2] > 0 && ((sp_hit.getID() & 0xfff) == 31)) {
      sel_ecal_sp_hits.push_back(sp_hit);
    }
  }

  // Recoil target surface for truth and seed tracks is the target

  for (std::pair<int, std::vector<int>> element : recoil_sh_count_map) {
    // Only take the first entry of the vector: it should be the scoring plane
    // hit with the highest momentum.
    const ldmx::SimTrackerHit& hit = scoring_hits.at(element.second.at(0));
    const ldmx::SimParticle& phit = particle_map[hit.getTrackID()];
    ldmx::SimTrackerHit ecal_hit;

    bool found_ecal_hit = false;
    for (auto ecal_sp_hit : sel_ecal_sp_hits) {
      if (ecal_sp_hit.getTrackID() == hit.getTrackID()) {
        ecal_hit = ecal_sp_hit;
        found_ecal_hit = true;
        break;
      }
    }

    // Findable particle selection
    // ldmx_log(trace) << "!!! n_recoil_sim_hits found: " <<
    // hit_count_map_recoil[hit.getTrackID()].size();
    if (hit_count_map_recoil[hit.getTrackID()].size() > n_min_hits_recoil_ &&
        found_ecal_hit && !skip_recoil_) {
      ldmx::Track truth_recoil_track;
      createTruthTrack(phit, hit, truth_recoil_track, target_surface);
      truth_recoil_track.setTrackID(hit.getTrackID());

      // AtTarget state from target scoring plane hit (pos mm, mom MeV, LDMX
      // frame)
      ldmx::Track::TrackState ts_target;
      ts_target.pos_ = {hit.getPosition()[0], hit.getPosition()[1],
                        hit.getPosition()[2]};
      ts_target.mom_ = {hit.getMomentum()[0], hit.getMomentum()[1],
                        hit.getMomentum()[2]};
      ts_target.ts_type_ = ldmx::AtTarget;
      truth_recoil_track.addTrackState(ts_target);

      // Express truth ECAL state in the bound parametrization of ecal_surface
      // (same surface definition used by CKFProcessor).
      {
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
          auto part{Acts::GenericParticleHypothesis(
              Acts::ParticleHypothesis(Acts::PdgParticle(particle_hypothesis_)))};
          Acts::BoundTrackParameters ecal_pars(
              ecal_surface, ecal_bound.value(),
              Acts::BoundSquareMatrix::Identity(), part);
          truth_recoil_track.addTrackState(tracking::sim::utils::makeTrackState(
              geometryContext(), ecal_pars, ldmx::AtECAL));
        }
      }

      // Attach sim hit indices
      int nhits = 0;
      for (auto sim_hit_idx : hit_count_map_recoil.at(hit.getTrackID())) {
        truth_recoil_track.addMeasurementIndex(sim_hit_idx);
        nhits++;
      }
      truth_recoil_track.setNhits(nhits);

      recoil_truth_tracks.push_back(truth_recoil_track);
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

DECLARE_PRODUCER(tracking::reco::TruthSeedProcessor)
