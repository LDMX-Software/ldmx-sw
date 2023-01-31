#include "Tracking/Reco/TruthSeedProcessor.h"

namespace tracking::reco {

TruthSeedProcessor::TruthSeedProcessor(const std::string &name,
                                       framework::Process &process)
    : framework::Producer(name, process) {}

void TruthSeedProcessor::onProcessStart() { gctx_ = Acts::GeometryContext(); }

void TruthSeedProcessor::configure(framework::config::Parameters &parameters) {
  pdg_ids_ = parameters.getParameter<std::vector<int> >("pdg_ids", {11});
  scoring_hits_coll_name_ =
      parameters.getParameter<std::string>("scoring_hits_coll_name");
  recoil_sim_hits_coll_name_ =
      parameters.getParameter<std::string>("recoil_sim_hits_coll_name");
  n_min_hits_ = parameters.getParameter<int>("n_min_hits", 7);
  z_min_ = parameters.getParameter<double>("z_min", -9999);  // mm
  track_id_ = parameters.getParameter<int>("track_id", -9999);
  pz_cut_ = parameters.getParameter<double>("pz_cut", -9999);  // MeV
  p_cut_ = parameters.getParameter<double>("p_cut", 0.);
  p_cut_max_ = parameters.getParameter<double>("p_cut_max", 100000.);  // MeV
  p_cut_ecal_ = parameters.getParameter<double>("p_cut_ecal", -1.);    // MeV
}

ldmx::Track TruthSeedProcessor::createSeed(const ldmx::SimParticle &particle,
                                           const ldmx::SimTrackerHit &hit) {
  std::vector<double> pos{static_cast<double>(hit.getPosition()[0]),
                          static_cast<double>(hit.getPosition()[1]),
                          static_cast<double>(hit.getPosition()[2])};
  return createSeed(pos, hit.getMomentum(), particle.getCharge());
}

ldmx::Track TruthSeedProcessor::createSeed(const ldmx::SimParticle &particle) {
  return createSeed(particle.getVertex(), particle.getMomentum(),
                    particle.getCharge());
}

ldmx::Track TruthSeedProcessor::createSeed(const std::vector<double> &pos_vec,
                                           const std::vector<double> &p_vec,
                                           int charge) {
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
                        free_params, *gen_surface, gctx_)
                        .value()};

  // Build the covariance matrix. First, guess some reasonable errors on the
  // position, time, and track angles.
  Acts::BoundVector stddev;
  stddev[Acts::eBoundLoc0] = 500 * Acts::UnitConstants::um;
  stddev[Acts::eBoundLoc1] = 1 * Acts::UnitConstants::mm;
  stddev[Acts::eBoundTime] = 1000 * Acts::UnitConstants::ns;
  stddev[Acts::eBoundPhi] = 2 * Acts::UnitConstants::degree;
  stddev[Acts::eBoundTheta] = 2 * Acts::UnitConstants::degree;

  // 50% of uncertainty on momentum from seed fit // Passing 2 GeV, Expected:
  // 500 MeV for 4 GeV electrons
  auto p_mag{sqrt(mom(0) * mom(0) + mom(1) * mom(1) + mom(2) * mom(2)) *
             Acts::UnitConstants::MeV};
  auto sigma_p{0.5 * p_mag * Acts::UnitConstants::GeV};
  auto sigma_qop_tagger{(1. / p_mag) * (1. / p_mag) * sigma_p};
  stddev[Acts::eBoundQOverP] = sigma_qop_tagger;

  // Using the above parameters, create the covariance matrix.
  Acts::BoundSymMatrix bound_cov = stddev.cwiseProduct(stddev).asDiagonal();

  // Create the seed track object.
  auto seed_track = ldmx::Track();
  seed_track.setPerigeeLocation(free_params[Acts::eFreePos0],
                                free_params[Acts::eFreePos1],
                                free_params[Acts::eFreePos2]);

  std::vector<double> ldmx_cov;
  tracking::sim::utils::flatCov(bound_cov, ldmx_cov);
  seed_track.setPerigeeParameters(
      tracking::sim::utils::convertActsToLdmxPars(bound_params));
  seed_track.setPerigeeCov(ldmx_cov);

  return seed_track;
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
  // nevents_++;

  // auto start = std::chrono::high_resolution_clock::now();

  // Start by creating a tagger tracker seed for the incident electron. It's
  // being assumed that the incident electron will have a track ID = 1.  This
  // is the case for the most common LDMX samples.
  auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};
  ldmx::SimParticle gen_e = particleMap[1];

  // Use the truth information extracted from the seed particle to create
  // a seed track.
  ldmx::Track trk{createSeed(gen_e)};
  std::vector<ldmx::Track> tagger_truth_seeds_{createSeed(gen_e)};

  // Add the seed to the event.
  event.add("TaggerTruthSeeds", tagger_truth_seeds_);

  // For the recoil tracker, vertex information needs to be extracted using the
  // scoring plane hit left by the particle at the target. Some additional
  // cuts are also to clean up scoring plane hits left by low energy particles
  // curling near the target.
  const std::vector<ldmx::SimTrackerHit> scoring_hits{
      event.getCollection<ldmx::SimTrackerHit>(scoring_hits_coll_name_)};

  // Retrieve the scoring plane hits at the ECAL
  const std::vector<ldmx::SimTrackerHit> scoring_hits_ecal{
      event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits")};

  // TODO:: change to indices instead objects
  std::vector<ldmx::SimTrackerHit> selected_sp_hits;

  std::for_each(scoring_hits.begin(), scoring_hits.end(),
                [&](const ldmx::SimTrackerHit &hit) {
                  if (scoringPlaneHitFilter(hit, scoring_hits_ecal))
                    selected_sp_hits.push_back(hit);
                });

  ldmx_log(debug) << "Selected scoring hits::" << selected_sp_hits.size();

  if (selected_sp_hits.empty()) return;

  // Retrieve the sim hits in the tracker of interest
  const std::vector<ldmx::SimTrackerHit> sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(recoil_sim_hits_coll_name_);

  // Create a mapping from the selected scoring plane hit objects to the number
  // of hits they associated particle creates in the tracker.
  std::unordered_map<int, int> hit_count_map;
  for (auto &sim_hit : sim_hits) {
    if (!hit_count_map.count(sim_hit.getTrackID()))
      hit_count_map[sim_hit.getTrackID()] = 0;

    hit_count_map[sim_hit.getTrackID()]++;
  }

  std::vector<ldmx::Track> truth_seeds{};

  std::for_each(
      selected_sp_hits.begin(), selected_sp_hits.end(),
      [&](const ldmx::SimTrackerHit &hit) {
        if (hit_count_map[hit.getTrackID()] >= n_min_hits_)
          truth_seeds.push_back(createSeed(particleMap[hit.getTrackID()], hit));
      });

  event.add("RecoilTruthSeeds", truth_seeds);

  // Do the k0 selection.
  // std::vector<ldmx::Track> k0_decay_pions;

  // if (k0_sel_) {
  //   if (truth_seeds_.size() == 2 && truth_seeds_.at(0).charge() *
  //   truth_seeds_.at(1).charge() < 0) {
  //     TLorentzVector p1, p2;
  //    p1.SetXYZM(truth_seeds_.at(0).momentum()(0),
  //                truth_seeds_.at(0).momentum()(1),
  //                truth_seeds_.at(0).momentum()(2),
  //                pion_mass);
  //
  //     p2.SetXYZM(truth_seeds_.at(1).momentum()(0),
  //                truth_seeds_.at(1).momentum()(1),
  //                truth_seeds_.at(1).momentum()(2),
  //                pion_mass);
  //   }
  // }// k0 selection
}
}  // namespace tracking::reco

DECLARE_PRODUCER_NS(tracking::reco, TruthSeedProcessor)
