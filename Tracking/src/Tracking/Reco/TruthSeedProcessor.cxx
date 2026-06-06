#include "Tracking/Reco/TruthSeedProcessor.h"

#include "Tracking/Sim/GeometryContainers.h"

namespace tracking::reco {

TruthSeedProcessor::TruthSeedProcessor(const std::string& name,
                                       framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void TruthSeedProcessor::onNewRun(const ldmx::RunHeader& rh) {
  gctx_ = Acts::GeometryContext();
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
  max_track_id_ = parameters.get<int>("max_track_id", 5);

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

  const auto& scoring_hits = event.getCollection<ldmx::SimTrackerHit>(
      scoring_hits_coll_name_, sp_pass_name_);

  // Retrieve the sim hits in the tagger tracker
  const auto& tagger_sim_hits = event.getCollection<ldmx::SimTrackerHit>(
      tagger_sim_hits_coll_name_, input_pass_name_);

  // Retrieve the sim hits in the recoil tracker
  const auto& recoil_sim_hits = event.getCollection<ldmx::SimTrackerHit>(
      recoil_sim_hits_coll_name_, input_pass_name_);

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

  
  //build vector of Tracks to pass on to TruthTrackProcessor
  std::vector<ldmx::Track> tagger_truth_tracks;
  std::vector<ldmx::Track> recoil_truth_tracks;
  std::vector<ldmx::Track> beam_electrons;

  //make the tagger seeds
  if(!skip_tagger_)
  {
    for (auto& [tid, indices]: tagger_sh_count_map)
    {
      const auto& hit = scoring_hits.at(indices.at(0));
      if(hit_count_map_tagger[hit.getTrackID()].size() > n_min_hits_tagger_)
      {
        ldmx::Track tagger_trk;
        tagger_trk.setTrackID(hit.getTrackID());
        tagger_trk.setPdgID(hit.getPdgID());

        //package seed attributes like momentum and position
        ldmx::Track::TrackState ts;
        ts.pos_ = {hit.getPosition()[0], hit.getPosition()[1], hit.getPosition()[2]};
        ts.mom_ = {hit.getMomentum()[0], hit.getMomentum()[1], hit.getMomentum()[2] };
        ts.ts_type_ = ldmx::AtTarget;
        tagger_trk.addTrackState(ts);

        for(auto idx:hit_count_map_tagger[hit.getTrackID()])
        {
          tagger_trk.addMeasurementIndex(idx);
        }
        tagger_trk.setNhits(hit_count_map_tagger[hit.getTrackID()].size());

        tagger_truth_tracks.push_back(tagger_trk);

        if(hit.getPdgID()==11 && hit.getTrackID() < max_track_id_)
        {
          // Add the truth track state at the beam origin using particle vertex/momentum
          // (SimParticle vertex and momentum are in LDMX global frame)
          ldmx::Track beam_electron_trk = tagger_trk;
          ldmx::Track::TrackState ts_truth_beam_origin;
          auto beam_electron = particle_map[hit.getTrackID()];
          ts_truth_beam_origin.pos_ = {beam_electron.getVertex()[0],
                                 beam_electron.getVertex()[1],
                                 beam_electron.getVertex()[2]};
          ts_truth_beam_origin.mom_ = {beam_electron.getMomentum()[0],
                                 beam_electron.getMomentum()[1],
                                 beam_electron.getMomentum()[2]};
          ts_truth_beam_origin.ts_type_ = ldmx::AtBeamOrigin;
          beam_electron_trk.addTrackState(ts_truth_beam_origin);
          beam_electrons.push_back(beam_electron_trk);
        }
      }
    }
  }

  // Recover the EcalScoring hits
  const auto& ecal_sp_hits = event.getCollection<ldmx::SimTrackerHit>(
      ecal_sp_coll_name_, sp_pass_name_);
  // Select ECAL hits
  std::vector<ldmx::SimTrackerHit> sel_ecal_sp_hits;

  for (auto& sp_hit : ecal_sp_hits) {
    if (sp_hit.getMomentum()[2] > 0 && ((sp_hit.getID() & 0xfff) == 31)) {
      sel_ecal_sp_hits.push_back(sp_hit);
    }
  }

  //make the recoil seeds for TruthTrackProcessor
  if(!skip_recoil_)
  {
    auto ecal_surface = tracking::sim::utils::unboundSurface(240.5);

    for (auto& [tid, indices]: recoil_sh_count_map)
    {
      const auto& hit = scoring_hits.at(indices.at(0));
  
      ldmx::SimTrackerHit ecal_hit;
      bool found_ecal_hit = false;
      for (const auto& ecal_sp_hit : sel_ecal_sp_hits) {
        if (ecal_sp_hit.getTrackID() == hit.getTrackID()) {
          ecal_hit = ecal_sp_hit;
          found_ecal_hit = true;
          break;
        }
      }

      if (!found_ecal_hit) continue;

      if(hit_count_map_recoil[hit.getTrackID()].size() > n_min_hits_recoil_)
      {
        ldmx::Track recoil_trk;
        recoil_trk.setTrackID(hit.getTrackID());
        recoil_trk.setPdgID(hit.getPdgID());

        //package seed attributes like momentum and position
        ldmx::Track::TrackState ts;
        ts.pos_ = {hit.getPosition()[0], hit.getPosition()[1], hit.getPosition()[2]};
        ts.mom_ = {hit.getMomentum()[0], hit.getMomentum()[1], hit.getMomentum()[2] };
        ts.ts_type_ = ldmx::AtTarget;
        recoil_trk.addTrackState(ts);

        // Express truth ECAL state in the bound parametrization of ecal_surface
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
        double q_ecal = particle_map[hit.getTrackID()].getCharge() * Acts::UnitConstants::e;
        auto ecal_free = tracking::sim::utils::toFreeParameters(ep, em, q_ecal);
        auto ecal_bound = Acts::transformFreeToBoundParameters(
            ecal_free, *ecal_surface, gctx_);
        if (ecal_bound.ok()) {
          auto part{Acts::GenericParticleHypothesis(Acts::ParticleHypothesis(
              Acts::PdgParticle(particle_hypothesis_)))};
          Acts::BoundTrackParameters ecal_pars(
              ecal_surface, ecal_bound.value(),
              Acts::BoundSquareMatrix::Identity(), part);
          recoil_trk.addTrackState(tracking::sim::utils::makeTrackState(
              geometryContext(), ecal_pars, ldmx::AtECAL));
        }

        for(auto& idx:hit_count_map_recoil[hit.getTrackID()])
        {
          recoil_trk.addMeasurementIndex(idx);
        }
        recoil_trk.setNhits(hit_count_map_recoil[hit.getTrackID()].size());

        recoil_truth_tracks.push_back(recoil_trk);
      }
    }
  }

  event.add(beam_electrons_collection_, beam_electrons);
  event.add(tagger_truth_collection_, tagger_truth_tracks);
  event.add(recoil_truth_collection_, recoil_truth_tracks);
}
}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::TruthSeedProcessor)
