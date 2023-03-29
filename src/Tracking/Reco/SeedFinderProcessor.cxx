#include "Tracking/Reco/SeedFinderProcessor.h"

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Eigen/Dense"

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
    : framework::Producer(name, process) {
  
  outputFile_ = new TFile("seeder.root", "RECREATE");
  outputTree_ = new TTree("seeder", "seeder");

  outputTree_->Branch("nevents", &nevents_);
  outputTree_->Branch("xhit", &xhit_);
  outputTree_->Branch("yhit", &yhit_);
  outputTree_->Branch("zhit", &zhit_);

  outputTree_->Branch("b0", &b0_);
  outputTree_->Branch("b1", &b1_);
  outputTree_->Branch("b2", &b2_);
  outputTree_->Branch("b3", &b3_);
  outputTree_->Branch("b4", &b4_);
}

SeedFinderProcessor::~SeedFinderProcessor() {}

void SeedFinderProcessor::onProcessStart() {
  // Build the tracking geometry
  ldmx_tg = std::make_shared<tracking::reco::TrackersTrackingGeometry>(
      detector_, &gctx_, false);
}

void SeedFinderProcessor::configure(framework::config::Parameters& parameters) {

  // Retrieve the path to the GDML description of the detector
  detector_ = parameters.getParameter<std::string>("detector");
  
  out_seed_collection_ = parameters.getParameter<std::string>(
      "out_seed_collection", getName() + "SeedTracks");
  input_hits_collection_ = parameters.getParameter<std::string>(
      "input_hits_collection", "TaggerSimHits");
  perigee_location_ = parameters.getParameter<std::vector<double>>(
      "perigee_location", {-700, 0., 0.});
  pmin_ =
      parameters.getParameter<double>("pmin", 0.05 * Acts::UnitConstants::GeV);
  pmax_ = parameters.getParameter<double>("pmax", 8 * Acts::UnitConstants::GeV);
  d0max_ =
      parameters.getParameter<double>("d0max", -15. * Acts::UnitConstants::mm);
  d0min_ =
      parameters.getParameter<double>("d0min", -45. * Acts::UnitConstants::mm);
  z0max_ =
      parameters.getParameter<double>("z0max", 60. * Acts::UnitConstants::mm);
  strategies_ = parameters.getParameter<std::vector<std::string>>(
      "strategies", {"0,1,2,3,4"});
  bfield_ = parameters.getParameter<double>("bfield", 1.5);
}

void SeedFinderProcessor::produce(framework::Event& event) {
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<ldmx::Track> seed_tracks;

  nevents_++;

  // Read in the Sim hits -- TODO choose which collection from config
  const std::vector<ldmx::SimTrackerHit> sim_hits =
      event.getCollection<ldmx::SimTrackerHit>("TaggerSimHits");
  std::vector<ldmx::LdmxSpacePoint*> ldmxsps;

  // Only convert simHits that have at least 0.05 edep
  // ldmx_log(info) << "Converting sim hits";

  for (auto& simHit : sim_hits) {
    // Remove low energy deposit hits
    if (simHit.getEdep() > 0.05) {
      ldmxsps.push_back(
          tracking::sim::utils::convertSimHitToLdmxSpacePoint(simHit));
    }
  }

  const std::vector<ldmx::Measurement> measurements =
      event.getCollection<ldmx::Measurement>(input_hits_collection_);

  groups_map.clear();
  std::vector<int> strategy = {0, 1, 2, 3, 4};
  bool success = GroupStrips(measurements, strategy);
  if (success) FindSeedsFromMap(seed_tracks);

  /*
  groups_map.clear();
  strategy = {3,4,5,6,7};
  success = GroupStrips(measurements,strategy);
  if (success)
    FindSeedsFromMap(seed_tracks);

  */
  groups_map.clear();
  outputTree_->Fill();
  event.add(out_seed_collection_, seed_tracks);
  ntracks_ += seed_tracks.size();

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
  // Step 0: Get the sim hits and project them on the surfaces to mimic 2d hits
  // Step 1: Smear the hits and associate an uncertainty to those measurements.

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

ldmx::Track SeedFinderProcessor::SeedTracker(
    const std::vector<ldmx::Measurement>& vmeas, double xOrigin,
    const Acts::Vector3& perigee_location) {
  // Fit a straight line in the non-bending plane and a parabola in the bending
  // plane
  // TODO:: use the actual errors from the measurement.

  const int N = vmeas.size();
  double uError = 0.006;
  double vError = 40. / sqrt(12);

  // Each measurement is treated as a 3D point, where the v direction is in the
  // center of the strip with sigma equal to the length of the strip / sqrt(12).
  // In this way it's easier to incorporate the tagger track extrapolation to
  // the fit

  Acts::ActsMatrix<5, 5> A = Acts::ActsMatrix<5, 5>::Zero();
  Acts::ActsVector<5> Y = Acts::ActsVector<5>::Zero();

  for (auto meas : vmeas) {
    double xmeas = meas.getGlobalPosition()[0] - xOrigin;

    // Get the surface
    const Acts::Surface* hit_surface = ldmx_tg->getSurface(meas.getLayerID());

    // Get the global to local transformation
    auto rot = hit_surface->transform(gctx_).rotation();
    auto tr = hit_surface->transform(gctx_).translation();

    auto rotl2g = rot.transpose();

    // Only for saving purposes
    Acts::Vector2 loc{meas.getLocalPosition()[0], 0.};

    xhit_.push_back(xmeas);
    yhit_.push_back(meas.getGlobalPosition()[1]);
    zhit_.push_back(meas.getGlobalPosition()[2]);

    Acts::ActsMatrix<2, 5> A_i;

    A_i(0, 0) = rotl2g(0, 1);
    A_i(0, 1) = rotl2g(0, 1) * xmeas;
    A_i(0, 2) = rotl2g(0, 1) * xmeas * xmeas;
    A_i(0, 3) = rotl2g(0, 2);
    A_i(0, 4) = rotl2g(0, 2) * xmeas;

    A_i(1, 0) = rotl2g(1, 1);
    A_i(1, 1) = rotl2g(1, 1) * xmeas;
    A_i(1, 2) = rotl2g(1, 1) * xmeas * xmeas;
    A_i(1, 3) = rotl2g(1, 2);
    A_i(1, 4) = rotl2g(1, 2) * xmeas;

    // Fill the yprime vector
    Acts::Vector2 offset = (rot.transpose() * tr).topRows<2>();
    Acts::Vector2 xoffset = {rotl2g(0, 0) * xmeas, rotl2g(1, 0) * xmeas};

    loc(0) = meas.getLocalPosition()[0];
    loc(1) = 0.;

    Acts::ActsMatrix<2, 2> W_i =
        Acts::ActsMatrix<2, 2>::Zero();  // weight matrix

    W_i(0, 0) = 1. / (uError * uError);
    W_i(1, 1) = 1. / (vError * vError);

    Acts::Vector2 Yprime_i = loc + offset - xoffset;

    Acts::Vector2 yi = W_i * Yprime_i;
    Y += (A_i.transpose()) * W_i * Yprime_i;

    Acts::ActsMatrix<2, 5> WA_i = (W_i * A_i);
    A += A_i.transpose() * WA_i;
  }

  Acts::ActsVector<5> B;
  B = A.inverse() * Y;

  b0_.push_back(B(0));
  b1_.push_back(B(1));
  b2_.push_back(B(2));
  b3_.push_back(B(3));
  b4_.push_back(B(4));

  Acts::ActsVector<5> hlx = Acts::ActsVector<5>::Zero();
  Acts::ActsVector<3> ref{0., 0., 0.};
  LineParabolaToHelix(B, hlx, ref);

  double relativePerigeeX = perigee_location(0) - xOrigin;

  std::shared_ptr<const Acts::PerigeeSurface> seed_perigee =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
          relativePerigeeX, perigee_location(1), perigee_location(2)));

  // in mm
  Acts::Vector3 seed_pos{relativePerigeeX,
                         B(0) + B(1) * relativePerigeeX +
                             B(2) * relativePerigeeX * relativePerigeeX,
                         B(3) + B(4) * relativePerigeeX};
  Acts::Vector3 dir{1, B(1) + 2 * B(2) * relativePerigeeX, B(4)};
  dir /= dir.norm();

  // Momentum at xmeas
  double p =
      0.3 * bfield_ * (1. / (2. * abs(B(2)))) * 0.001;  // R in meters, p in GeV
  // std::cout<<"Momentum "<< p*dir << std::endl;

  // Convert it to MeV since that's what TrackUtils assumes
  Acts::Vector3 seed_mom = p * dir / Acts::UnitConstants::MeV;
  Acts::ActsScalar q =
      B(2) < 0 ? -1 * Acts::UnitConstants::e : +1 * Acts::UnitConstants::e;
  Acts::FreeVector seed_free =
      tracking::sim::utils::toFreeParameters(seed_pos, seed_mom, q);

  auto bound_params = Acts::detail::transformFreeToBoundParameters(
                          seed_free, *seed_perigee, gctx_)
                          .value();

  ldmx_log(debug) << "bound parameters at perigee location" << std::endl
                  << bound_params;

  Acts::BoundVector stddev;
  double sigma_p = 0.75 * p * Acts::UnitConstants::GeV;
  stddev[Acts::eBoundLoc0] = 2 * Acts::UnitConstants::mm;
  stddev[Acts::eBoundLoc1] = 5 * Acts::UnitConstants::mm;
  stddev[Acts::eBoundTime] = 1000 * Acts::UnitConstants::ns;
  stddev[Acts::eBoundPhi] = 5 * Acts::UnitConstants::degree;
  stddev[Acts::eBoundTheta] = 5 * Acts::UnitConstants::degree;
  stddev[Acts::eBoundQOverP] = (1. / p) * (1. / p) * sigma_p;

  Acts::BoundSymMatrix bound_cov = stddev.cwiseProduct(stddev).asDiagonal();

  ldmx::Track trk = ldmx::Track();
  trk.setPerigeeLocation(perigee_location(0), perigee_location(1),
                         perigee_location(2));
  trk.setChi2(0.);
  trk.setNhits(5);
  trk.setNdf(0);
  trk.setNsharedHits(0);
  std::vector<double> v_seed_params(
      (bound_params).data(),
      bound_params.data() + bound_params.rows() * bound_params.cols());
  std::vector<double> v_seed_cov;
  tracking::sim::utils::flatCov(bound_cov, v_seed_cov);
  trk.setPerigeeParameters(v_seed_params);
  trk.setPerigeeCov(v_seed_cov);

  Acts::BoundTrackParameters seedParameters(seed_perigee,
                                            std::move(bound_params), bound_cov);

  return trk;
}

// To augment with the covariance matrix.
// The following formulas only works if the center if x0 where the
// linear+parabola function is evaluated is 0. It also assumes the axes
// orientation according to the ACTS needs. phi0 is the angle of the tangent of
// the track at the perigee. Positive counterclockwise. Theta is positive from
// the z axis to the bending plane. (pi/2 for tracks along the x axis)

void SeedFinderProcessor::LineParabolaToHelix(
    const Acts::ActsVector<5> parameters, Acts::ActsVector<5>& helix_parameters,
    Acts::Vector3 ref) {
  double R = 0.5 / abs(parameters(2));
  double xc = R * parameters(1);
  double p2 = 0.5 * parameters(1) * parameters(1);
  double factor = parameters(2) < 0 ? -1 : 1;
  double yc =
      parameters(0) +
      factor *
          (R * (1 - p2));  //+ or minus solution. I chose beta0 - R ( ... ),
                           // because the curvature is negative for electrons.
                           // The sign need to be chosen by the sign of B(2)
  double theta0 = atan2(yc, xc);
  double k = parameters(2) < 0 ? (-1. / R) : 1. / R;
  double d0 = R - xc / cos(theta0);
  double phi0 = theta0 + 1.57079632679;
  double tanL = parameters(4) * cos(theta0);
  double theta = 1.57079632679 - atan(tanL);
  double z0 = parameters(3) + (d0 * tanL * tan(theta0));
  double qOp = factor / (0.3 * bfield_ * R * 0.001);

  // std::cout<<d0<<" "<<z0<<" "<<phi0<<" "<<theta<<" "<<qOp<<std::endl;
}

void SeedFinderProcessor::onProcessEnd() {
  outputFile_->cd();
  outputTree_->Write();
  outputFile_->Close();
  std::cout << "PROCESSOR:: " << this->getName()
            << "   AVG Time/Event: " << processing_time_ / nevents_ << " ms"
            << std::endl;
  std::cout << "PROCESSOR:: " << this->getName()
            << "   Total Seeds/Events: " << ntracks_ << "/" << nevents_
            << std::endl;
  std::cout << "PROCESSOR:: " << this->getName()
            << "   Seeds discarded due to multiple hits on layers " << ndoubles_
            << std::endl;
  std::cout << "PROCESSOR:: " << this->getName() << "   not enough seed points "
            << nmissing_ << std::endl;
  std::cout << "PROCESSOR:: " << this->getName()
            << "   nfailpmin=" << nfailpmin_ << std::endl;
  std::cout << "PROCESSOR:: " << this->getName()
            << "   nfailpmax=" << nfailpmax_ << std::endl;
  std::cout << "PROCESSOR:: " << this->getName()
            << "   nfaild0max=" << nfaild0max_ << std::endl;
  std::cout << "PROCESSOR:: " << this->getName()
            << "   nfaild0min=" << nfaild0min_ << std::endl;
  std::cout << "PROCESSOR:: " << this->getName()
            << "   nfailz0max=" << nfailz0max_ << std::endl;
}

// Given a strategy, group the hits according to some options
// Not a good algorithm. The best would be to organize all the hits in sensors
// *first* then only select the hits that we are interested into. TODO!

bool SeedFinderProcessor::GroupStrips(
    const std::vector<ldmx::Measurement>& measurements,
    const std::vector<int> strategy) {
  // std::cout<<"Using stratedy"<<std::endl;
  // for (auto& e : strategy) {
  //  std::cout<<e<<" ";
  //}
  // std::cout<<std::endl;

  for (auto& meas : measurements) {
    if (std::find(strategy.begin(), strategy.end(), meas.getLayer()) !=
        strategy.end()) {
      groups_map[meas.getLayer()].push_back(&meas);
    }

  }  // loop meas

  if (groups_map.size() < 5)
    return false;
  else
    return true;
}

// For each strategy, form all the possible combinatorics and form a seedTrack
// for each of those This will reshuffle all points. (issue?) Will sort the
// meas_for_seed vector

void SeedFinderProcessor::FindSeedsFromMap(std::vector<ldmx::Track>& seeds) {
  std::vector<ldmx::Measurement> meas_for_seeds;
  meas_for_seeds.reserve(5);

  std::map<int, std::vector<const ldmx::Measurement*>>::iterator groups_iter =
      groups_map.begin();
  std::vector<const ldmx::Measurement*>::iterator meas_iter;

  // Vector of iterators

  constexpr size_t K = 5;
  std::vector<std::vector<const ldmx::Measurement*>::iterator> it;
  it.reserve(K);

  unsigned int ikey = 0;
  for (auto& key : groups_map) {
    it[ikey] = key.second.begin();
    ikey++;
  }

  // K vectors in an array v[0],v[1].... v[K-1]

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

    for (int j = 0; j < K; j++) {
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

    ldmx::Track seedTrack =
        SeedTracker(meas_for_seeds, meas_for_seeds.at(2).getGlobalPosition()[0],
                    Acts::Vector3(perigee_location_[0], perigee_location_[1],
                                  perigee_location_[2]));

    bool fail = false;
    // Remove failed fits

    if (1. / abs(seedTrack.getQoP()) < pmin_) {
      nfailpmin_++;
      fail = true;
    } else if (1. / abs(seedTrack.getQoP()) > pmax_) {
      nfailpmax_++;
      fail = true;
    } else if (abs(seedTrack.getZ0()) > z0max_) {
      nfailz0max_++;
      fail = true;
    } else if (seedTrack.getD0() < d0min_) {
      nfaild0min_++;
      fail = true;
    } else if (seedTrack.getD0() > d0max_) {
      nfaild0max_++;
      fail = true;
    }

    if (!fail)
      seeds.push_back(seedTrack);

    else {
      b0_.pop_back();
      b1_.pop_back();
      b2_.pop_back();
      b3_.pop_back();
      b4_.pop_back();
    }

    // Go to next combination
    ++it[K - 1];
    for (int i = K - 1;
         (i > 0) && (it[i] == (std::next(groups_iter, i))->second.end()); --i) {
      it[i] = std::next(groups_iter, i)->second.begin();
      ++it[i - 1];
    }
  }
}  // find seeds

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, SeedFinderProcessor)
