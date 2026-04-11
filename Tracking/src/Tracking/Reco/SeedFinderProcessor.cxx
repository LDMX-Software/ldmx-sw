#include "Tracking/Reco/SeedFinderProcessor.h"

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Eigen/Dense"
#include "Tracking/Sim/TrackingUtils.h"

/* This processor takes in input a set of 3D space points and builds seedTracks
 * using the ACTS algorithm which is based on the ATLAS 3-space point conformal
 * fit.
 *
 */

using Eigen::MatrixXd;
using Eigen::VectorXd;

namespace tracking {
namespace reco {

SeedFinderProcessor::SeedFinderProcessor(const std::string& name,
                                         framework::Process& process)
    : TrackingGeometryUser(name, process) {
  // TODO REMOVE FROM DEFAULT
  /*
  output_file_ = new TFile("seeder.root", "RECREATE");
  output_tree_ = new TTree("seeder", "seeder");

  output_tree_->Branch("nevents", &nevents_);
  output_tree_->Branch("xhit", &xhit_);
  output_tree_->Branch("yhit", &yhit_);
  output_tree_->Branch("zhit", &zhit_);

  output_tree_->Branch("b0", &b0_);
  output_tree_->Branch("b1", &b1_);
  output_tree_->Branch("b2", &b2_);
  output_tree_->Branch("b3", &b3_);
  output_tree_->Branch("b4", &b4_);
  */
}

void SeedFinderProcessor::onProcessStart() {
  truth_matching_tool_ = std::make_shared<tracking::sim::TruthMatchingTool>();
}

void SeedFinderProcessor::configure(framework::config::Parameters& parameters) {
  // Output seed name
  out_seed_collection_ = parameters.get<std::string>("out_seed_collection",
                                                     getName() + "SeedTracks");

  // Input strip hits
  input_hits_collection_ =
      parameters.get<std::string>("input_hits_collection", "TaggerSimHits");

  // Tagger tracks - only for Recoil Seed finding
  tagger_trks_collection_ =
      parameters.get<std::string>("tagger_trks_collection", "TaggerTracks");

  perigee_location_ =
      parameters.get<std::vector<double>>("perigee_location", {-700, 0., 0.});
  pmin_ = parameters.get<double>("pmin", 0.05 * Acts::UnitConstants::GeV);
  pmax_ = parameters.get<double>("pmax", 8 * Acts::UnitConstants::GeV);
  d0max_ = parameters.get<double>("d0max", -15. * Acts::UnitConstants::mm);
  d0min_ = parameters.get<double>("d0min", -45. * Acts::UnitConstants::mm);
  z0max_ = parameters.get<double>("z0max", 60. * Acts::UnitConstants::mm);
  phicut_ = parameters.get<double>("phicut", 0.1);
  thetacut_ = parameters.get<double>("thetacut", 0.2);
  loc0cut_ = parameters.get<double>("loc0cut", 0.1);
  loc1cut_ = parameters.get<double>("loc1cut", 0.3);
  strategies_ =
      parameters.get<std::vector<std::string>>("strategies", {"0,1,2,3,4"});
  inflate_factors_ = parameters.get<std::vector<double>>(
      "inflate_factors", {10., 10., 10., 10., 10., 10.});
  bfield_ = parameters.get<double>("bfield", 1.5);
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  sim_particles_coll_name_ =
      parameters.get<std::string>("sim_particles_coll_name");
  sim_particles_passname_ =
      parameters.get<std::string>("sim_particles_passname");
  tagger_trks_event_collection_passname_ =
      parameters.get<std::string>("tagger_trks_event_collection_passname");
  sim_particles_event_passname_ =
      parameters.get<std::string>("sim_particles_event_passname");
  u_error_ = parameters.get<double>("u_error");
  v_error_ = parameters.get<double>("v_error");
}

void SeedFinderProcessor::produce(framework::Event& event) {
  // tg is unused, should it be? FIXME
  // const auto& tg{geometry()};
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<ldmx::Track> seed_tracks;

  nevents_++;

  // check if SimParticleMap is available for truth matching
  std::map<int, ldmx::SimParticle> particle_map;

  const std::vector<ldmx::Measurement> measurements =
      event.getCollection<ldmx::Measurement>(input_hits_collection_,
                                             input_pass_name_);

  std::vector<ldmx::Track> tagger_tracks;
  if (event.exists(tagger_trks_collection_,
                   tagger_trks_event_collection_passname_)) {
    tagger_tracks = event.getCollection<ldmx::Track>(tagger_trks_collection_,
                                                     input_pass_name_);
  }

  // Create an unbound surface at the target
  std::shared_ptr<Acts::Surface> tgt_surf =
      tracking::sim::utils::unboundSurface(0.);

  // Create the pseudomeasurements at the target

  ldmx::Measurements target_pseudo_meas;

  for (auto tagtrk : tagger_tracks) {
    // For Track, the perigee parameters are stored at the target surface.
    // Use d0/z0 as local position and the perigee covariance for the
    // pseudo measurement. Only create the pseudo measurement if cov is
    // available.

    // The covariance matrix passed to the pseudo measurement is considered as
    // uncorrelated. This is an approx that considers that loc-u and loc-v from
    // the track have small correlation.

    const auto& perigee_cov = tagtrk.getPerigeeCov();
    if (!perigee_cov.empty()) {
      Acts::BoundSquareMatrix cov =
          tracking::sim::utils::unpackCov(perigee_cov);
      double locu = tagtrk.getD0();
      double locv = tagtrk.getZ0();
      double covuu =
          cov(Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc0);
      double covvv =
          cov(Acts::BoundIndices::eBoundLoc1, Acts::BoundIndices::eBoundLoc1);

      ldmx::Measurement pseudo_meas;
      pseudo_meas.setLocalPosition(locu, locv);
      Acts::Vector3 dummy{0., 0., 0.};
      Acts::Vector2 local_pos{locu, locv};
      Acts::Vector3 global_pos =
          tgt_surf->localToGlobal(geometryContext(), local_pos, dummy);

      pseudo_meas.setGlobalPosition(global_pos(0), global_pos(1),
                                    global_pos(2));
      pseudo_meas.setTime(0.);
      pseudo_meas.setLocalCovariance(covuu, covvv);

      target_pseudo_meas.push_back(pseudo_meas);
    }
  }

  if (event.exists(sim_particles_coll_name_, sim_particles_event_passname_)) {
    particle_map = event.getMap<int, ldmx::SimParticle>(
        sim_particles_coll_name_, sim_particles_passname_);
    truth_matching_tool_->setup(particle_map, measurements);
  }

  ldmx_log(debug) << "Preparing the strategies";

  groups_map_.clear();
  //  set the seeding strategy
  //  strategy is a list of layers from which to  make the seed
  //  this must include 5 layers; layer_ numbering starts at 0.
  //  std::vector<int> strategy = {9,10,11,12,13};
  std::vector<int> strategy = {0, 1, 2, 3, 4};
  bool success = groupStrips(measurements, strategy);
  if (success) findSeedsFromMap(seed_tracks, target_pseudo_meas);

  //  currently, we only use a single strategy but eventually
  //  we will use more.  Below is an example of how to add them
  /*
  groups_map.clear();
  strategy = {9,10,11,12,13};
  success = GroupStrips(measurements,strategy);
  if (success)
    FindSeedsFromMap(seed_tracks, target_pseudo_meas);
  */

  groups_map_.clear();
  // output_tree_->Fill();
  ntracks_ += seed_tracks.size();
  event.add(out_seed_collection_, seed_tracks);

  auto end = std::chrono::high_resolution_clock::now();

  // long long microseconds =
  // std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();

  // Seed finding using 2D Hits
  //  - The hits should keep track if they are already associated to a track or
  //  not. This can be used for subsequent passes of seed-finding

  // This should go into a digitization producer, which takes care of producing
  // measurements from:
  //  - raw hits in data
  //  - sim hits in MC
  // Step 0: Get the sim hits and project them on the surfaces to mimic 2d
  // hits Step 1: Smear the hits and associate an uncertainty to those
  // measurements.

  xhit_.clear();
  yhit_.clear();
  zhit_.clear();

  b0_.clear();
  b1_.clear();
  b2_.clear();
  b3_.clear();
  b4_.clear();

}  // produce

// Seed finder from Robert's in HPS
// https://github.com/JeffersonLab/hps-java/blob/47712878302eb0c0374d077a208a6f8f0e2c3dc6/tracking/src/main/java/org/hps/recon/tracking/kalman/SeedTrack.java
// Adapted to possible 3D hit points.

// yOrigin is the location along the beam about which we fit the seed helix
// perigee_location is where the track parameters will be extracted

// while this takes in a target measurement (from tagger, this is pmeas_tgt)
// this code doesn't do anything with it yet.

ldmx::Track SeedFinderProcessor::seedTracker(
    const ldmx::Measurements& vmeas, double xOrigin,
    const Acts::Vector3& perigee_location,
    const ldmx::Measurements& pmeas_tgt) {
  // Fit a straight line in the non-bending plane and a parabola in the bending
  // plane

  // Each measurement is treated as a 3D point, where the v direction is in the
  // center of the strip with sigma equal to the length of the strip / sqrt(12).
  // In this way it's easier to incorporate the tagger track extrapolation to
  // the fit

  Acts::ActsMatrix<5, 5> a = Acts::ActsMatrix<5, 5>::Zero();
  Acts::ActsVector<5> y = Acts::ActsVector<5>::Zero();

  for (auto meas : vmeas) {
    double xmeas = meas.getGlobalPosition()[0] - xOrigin;

    // Get the surface
    const Acts::Surface* hit_surface = geometry().getSurface(meas.getLayerID());

    // Get the global to local transformation
    auto rot = hit_surface->transform(geometryContext()).rotation();
    auto tr = hit_surface->transform(geometryContext()).translation();

    auto rotl2g = rot.transpose();

    // Only for saving purposes
    Acts::Vector2 loc{meas.getLocalPosition()[0], 0.};

    xhit_.push_back(xmeas);
    yhit_.push_back(meas.getGlobalPosition()[1]);
    zhit_.push_back(meas.getGlobalPosition()[2]);

    Acts::ActsMatrix<2, 5> a_i;

    a_i(0, 0) = rotl2g(0, 1);
    a_i(0, 1) = rotl2g(0, 1) * xmeas;
    a_i(0, 2) = rotl2g(0, 1) * xmeas * xmeas;
    a_i(0, 3) = rotl2g(0, 2);
    a_i(0, 4) = rotl2g(0, 2) * xmeas;

    a_i(1, 0) = rotl2g(1, 1);
    a_i(1, 1) = rotl2g(1, 1) * xmeas;
    a_i(1, 2) = rotl2g(1, 1) * xmeas * xmeas;
    a_i(1, 3) = rotl2g(1, 2);
    a_i(1, 4) = rotl2g(1, 2) * xmeas;

    // Fill the yprime vector
    Acts::Vector2 offset = (rot.transpose() * tr).topRows<2>();
    Acts::Vector2 xoffset = {rotl2g(0, 0) * xmeas, rotl2g(1, 0) * xmeas};

    loc(0) = meas.getLocalPosition()[0];
    loc(1) = 0.;
    // weight matrix
    Acts::ActsMatrix<2, 2> w_i = Acts::ActsMatrix<2, 2>::Zero();

    w_i(0, 0) = 1. / (u_error_ * u_error_);
    w_i(1, 1) = 1. / (v_error_ * v_error_);

    Acts::Vector2 yprime_i = loc + offset - xoffset;
    y += (a_i.transpose()) * w_i * yprime_i;

    Acts::ActsMatrix<2, 5> wa_i = (w_i * a_i);
    a += a_i.transpose() * wa_i;
  }

  Acts::ActsVector<5> b;
  b = a.inverse() * y;

  b0_.push_back(b(0));
  b1_.push_back(b(1));
  b2_.push_back(b(2));
  b3_.push_back(b(3));
  b4_.push_back(b(4));

  // Acts::ActsVector<5> hlx = Acts::ActsVector<5>::Zero();
  Acts::ActsVector<3> ref{0., 0., 0.};

  double relative_perigee_x = perigee_location(0) - xOrigin;

  std::shared_ptr<const Acts::PerigeeSurface> seed_perigee =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
          relative_perigee_x, perigee_location(1), perigee_location(2)));

  // in mm
  Acts::Vector3 seed_pos{relative_perigee_x,
                         b(0) + b(1) * relative_perigee_x +
                             b(2) * relative_perigee_x * relative_perigee_x,
                         b(3) + b(4) * relative_perigee_x};
  Acts::Vector3 dir{1, b(1) + 2 * b(2) * relative_perigee_x, b(4)};
  dir /= dir.norm();

  // Momentum at xmeas
  // R in meters, p in GeV
  double p = 0.3 * bfield_ * (1. / (2. * abs(b(2)))) * 0.001;
  // std::cout<<"Momentum "<< p*dir << std::endl;

  // Convert it to MeV since that's what TrackUtils assumes
  Acts::Vector3 seed_mom = p * dir / Acts::UnitConstants::MeV;
  Acts::ActsScalar q =
      b(2) < 0 ? -1 * Acts::UnitConstants::e : +1 * Acts::UnitConstants::e;

  // Linear intersection with the perigee line. TODO:: Use propagator instead
  // Project the position on the surface.
  // This is mainly necessary for the perigee surface, where
  // the mean might not fulfill the perigee condition.

  // mg  Aug 2024 .. interect has changed, but just remove boundary check
  //  and change intersection to intersections
  //   auto intersection =
  //     (*seed_perigee).intersect(geometry_context(), seed_pos, dir, false);

  // Acts::FreeVector seed_free = tracking::sim::utils::toFreeParameters(
  //     intersection.intersection.position, seed_mom, q);

  auto intersection =
      (*seed_perigee).intersect(geometryContext(), seed_pos, dir);

  Acts::FreeVector seed_free = tracking::sim::utils::toFreeParameters(
      intersection.intersections()[0].position(), seed_mom, q);

  auto bound_params = Acts::transformFreeToBoundParameters(
                          seed_free, *seed_perigee, geometryContext())
                          .value();

  ldmx_log(trace) << "bound parameters at perigee location" << bound_params;

  Acts::BoundVector stddev;
  // sigma set to 75% of momentum
  double sigma_p = 0.75 * p * Acts::UnitConstants::GeV;
  stddev[Acts::eBoundLoc0] =
      inflate_factors_[Acts::eBoundLoc0] * 2 * Acts::UnitConstants::mm;
  stddev[Acts::eBoundLoc1] =
      inflate_factors_[Acts::eBoundLoc1] * 5 * Acts::UnitConstants::mm;
  stddev[Acts::eBoundPhi] =
      inflate_factors_[Acts::eBoundPhi] * 5 * Acts::UnitConstants::degree;
  stddev[Acts::eBoundTheta] =
      inflate_factors_[Acts::eBoundTheta] * 5 * Acts::UnitConstants::degree;
  stddev[Acts::eBoundQOverP] =
      inflate_factors_[Acts::eBoundQOverP] * (1. / p) * (1. / p) * sigma_p;
  stddev[Acts::eBoundTime] =
      inflate_factors_[Acts::eBoundTime] * 1000 * Acts::UnitConstants::ns;

  ldmx_log(debug)
      << "Making covariance matrix as diagonal matrix with inflated terms";
  Acts::BoundSquareMatrix bound_cov = stddev.cwiseProduct(stddev).asDiagonal();

  ldmx_log(debug) << "...now putting together the seed track ...";

  ldmx::Track trk = ldmx::Track();
  Acts::Vector3 perigee_ldmx =
      tracking::sim::utils::acts2Ldmx(perigee_location);
  trk.setPerigeeLocation(perigee_ldmx(0), perigee_ldmx(1), perigee_ldmx(2));
  trk.setChi2(0.);
  trk.setNhits(5);
  trk.setNdf(0);
  trk.setNsharedHits(0);
  trk.setCharge(q < 0 ? -1 : 1);
  std::vector<double> v_seed_params(
      (bound_params).data(),
      bound_params.data() + bound_params.rows() * bound_params.cols());
  std::vector<double> v_seed_cov;
  tracking::sim::utils::flatCov(bound_cov, v_seed_cov);
  trk.setPerigeeParameters(v_seed_params);
  trk.setPerigeeCov(v_seed_cov);

  ldmx_log(debug)
      << "...making the ParticleHypothesis ...assume electron for now";
  auto part_hypo{Acts::SinglyChargedParticleHypothesis::electron()};

  ldmx_log(debug) << "Making BoundTrackParameters seedParameters";
  Acts::BoundTrackParameters seed_parameters(
      seed_perigee, std::move(bound_params), bound_cov, part_hypo);

  ldmx_log(debug) << "Returning seed track";
  return trk;
}

void SeedFinderProcessor::onProcessEnd() {
  // output_file_->cd();
  // output_tree_->Write();
  // output_file_->Close();
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / nevents_ << " ms";
  ldmx_log(info) << "Total Seeds/Events: " << ntracks_ << "/" << nevents_;
  ldmx_log(info) << "Seeds discarded due to multiple hits on layers "
                 << ndoubles_;
  ldmx_log(info) << "not enough seed points " << nmissing_;
  ldmx_log(info) << " nfailpmin=" << nfailpmin_;
  ldmx_log(info) << "   nfailpmax=" << nfailpmax_;
  ldmx_log(info) << "   nfaild0max=" << nfaild0max_;
  ldmx_log(info) << "   nfaild0min=" << nfaild0min_;
  ldmx_log(info) << "   nfailphicut=" << nfailphi_;
  ldmx_log(info) << "   nfailthetacut=" << nfailtheta_;
  ldmx_log(info) << "   nfailz0max=" << nfailz0max_;
}

// Given a strategy, group the hits according to some options
// Not a good algorithm. The best would be to organize all the hits in sensors
// *first* then only select the hits that we are interested into. TODO!

bool SeedFinderProcessor::groupStrips(
    const std::vector<ldmx::Measurement>& measurements,
    const std::vector<int> strategy) {
  //    std::cout<<"Using stratedy"<<std::endl;
  // for (auto& e : strategy) {
  //  std::cout<<e<<" ";
  //}
  // std::cout<<std::endl;

  for (auto& meas : measurements) {
    ldmx_log(trace) << meas;

    if (std::find(strategy.begin(), strategy.end(), meas.getLayer()) !=
        strategy.end()) {
      ldmx_log(debug) << "Adding measurement from layer_ = " << meas.getLayer();
      groups_map_[meas.getLayer()].push_back(&meas);
    }

  }  // loop meas

  if (groups_map_.size() < 5)
    return false;
  else
    return true;
}

// For each strategy, form all the possible combinatorics and form a seedTrack
// for each of those This will reshuffle all points. (issue?) Will sort the
// meas_for_seed vector

void SeedFinderProcessor::findSeedsFromMap(std::vector<ldmx::Track>& seeds,
                                           const ldmx::Measurements& pmeas) {
  std::map<int, std::vector<const ldmx::Measurement*>>::iterator groups_iter =
      groups_map_.begin();
  // Vector of iterators
  constexpr size_t k = 5;
  std::vector<std::vector<const ldmx::Measurement*>::iterator> it;
  it.reserve(k);

  unsigned int ikey = 0;
  for (auto& key : groups_map_) {
    it[ikey] = key.second.begin();
    ikey++;
  }

  // K vectors in an array v[0],v[1].... v[K-1]

  // Loop over all combinations
  while (it[0] != groups_iter->second.end()) {
    // process the pointed-to elements

    /*
      for (int j=0; j<K; j++) {
      const ldmx::Measurement* meas = (*(it[j]));
      std::cout<<meas->getGlobalPosition()[0]<<","
      <<meas->getGlobalPosition()[1]<<","
      <<meas->getGlobalPosition()[2]<<","<<std::endl;
      }
    */

    std::vector<ldmx::Measurement> meas_for_seeds;
    meas_for_seeds.reserve(5);

    ldmx_log(debug) << " Grouping ";

    for (int j = 0; j < k; j++) {
      const ldmx::Measurement* meas = (*(it[j]));
      meas_for_seeds.push_back(*meas);
    }

    std::sort(meas_for_seeds.begin(), meas_for_seeds.end(),
              [](const ldmx::Measurement& m1, const ldmx::Measurement& m2) {
                return m1.getGlobalPosition()[0] < m2.getGlobalPosition()[0];
              });

    if (meas_for_seeds.size() < 5) {
      nmissing_++;
      return;
    }

    ldmx_log(debug) << "making seedTrack";

    Acts::Vector3 perigee{perigee_location_[0], perigee_location_[1],
                          perigee_location_[2]};

    ldmx::Track seed_track =
        seedTracker(meas_for_seeds, meas_for_seeds.at(2).getGlobalPosition()[0],
                    perigee, pmeas);

    bool fail = false;

    // Remove failed fits
    if (1. / abs(seed_track.getQoP()) < pmin_) {
      nfailpmin_++;
      fail = true;
    } else if (1. / abs(seed_track.getQoP()) > pmax_) {
      nfailpmax_++;
      fail = true;
    }

    // Remove large part of fake tracks and duplicates with the following cuts
    // for various compatibility checks.

    else if (abs(seed_track.getZ0()) > z0max_) {
      nfailz0max_++;
      fail = true;
    } else if (seed_track.getD0() < d0min_) {
      nfaild0min_++;
      fail = true;
    } else if (seed_track.getD0() > d0max_) {
      nfaild0max_++;
      fail = true;
    } else if (abs(seed_track.getPhi()) > phicut_) {
      fail = true;
      nfailphi_++;
    } else if (abs(seed_track.getTheta() - piover2_) > thetacut_) {
      fail = true;
      nfailtheta_++;
    }

    // If I didn't use the target pseudo measurements in the track finding
    // I can use them for compatibility with the tagger track

    // TODO this should protect against running this check on tagger seeder.
    // This is true only if this seeder is not run twice on the tagger after
    // already having tagger tracks available.
    if (pmeas.size() > 0) {
      // I can have multiple target pseudo measurements
      // A seed is rejected if it is found incompatible with all the target
      // extrapolations

      // This is set but unused, eventually we will use tagger track position at
      // target to inform recoil tracking bool tgt_compatible = false;
      for (auto tgt_pseudomeas : pmeas) {
        // The d0/z0 are in a frame with the same orientation of the target
        // surface
        double delta_loc0 =
            seed_track.getD0() - tgt_pseudomeas.getLocalPosition()[0];
        double delta_loc1 =
            seed_track.getZ0() - tgt_pseudomeas.getLocalPosition()[1];

        if (abs(delta_loc0) < loc0cut_ && abs(delta_loc1) < loc1cut_) {
          // found at least 1 compatible target location
          // tgt_compatible = true;
          break;
        }
      }
    }  // pmeas > 0

    if (!fail) {
      if (truth_matching_tool_->configured()) {
        auto truth_info = truth_matching_tool_->truthMatch(meas_for_seeds);
        seed_track.setTrackID(truth_info.track_id_);
        seed_track.setPdgID(truth_info.pdg_id_);
        seed_track.setTruthProb(truth_info.truth_prob_);
      }

      seeds.push_back(seed_track);
    }

    else {
      b0_.pop_back();
      b1_.pop_back();
      b2_.pop_back();
      b3_.pop_back();
      b4_.pop_back();
    }

    // Go to next combination
    ldmx_log(debug) << "Go to the next combination";

    ++it[k - 1];
    for (int i = k - 1;
         (i > 0) && (it[i] == (std::next(groups_iter, i))->second.end()); --i) {
      it[i] = std::next(groups_iter, i)->second.begin();
      ++it[i - 1];
    }
  }
}  // find seeds

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::SeedFinderProcessor)
