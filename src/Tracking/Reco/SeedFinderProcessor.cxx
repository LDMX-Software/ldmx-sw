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
  // Should be the typical behaviour.
  int numPhiNeighbors = 1;
  std::vector<std::pair<int, int>> zBinNeighborsTop;
  std::vector<std::pair<int, int>> zBinNeighborsBottom;

  bottom_bin_finder_ = std::make_shared<Acts::BinFinder<ldmx::LdmxSpacePoint>>(
      Acts::BinFinder<ldmx::LdmxSpacePoint>(zBinNeighborsBottom,
                                            numPhiNeighbors));

  top_bin_finder_ = std::make_shared<Acts::BinFinder<ldmx::LdmxSpacePoint>>(
      Acts::BinFinder<ldmx::LdmxSpacePoint>(zBinNeighborsTop, numPhiNeighbors));

  seed_to_track_maker_ =
      std::make_shared<tracking::sim::SeedToTrackParamMaker>();

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
  // Default configuration

  // Tagger r max
  config_.rMax = 1000.;
  config_.deltaRMin = 3.;
  config_.deltaRMax = 220.;
  config_.collisionRegionMin = -50;
  config_.collisionRegionMax = 50;
  config_.zMin = -300;
  config_.zMax = 300.;
  config_.maxSeedsPerSpM = 5;

  // More or less the max angle is something of the order of 50 / 600 (assuming
  // that the track hits all the layers) Theta for the seeder is like ATLAS eta,
  // so it's 90-lambda. Max lamba is of the order of ~0.1 so cotThetaMax will
  // be 1./tan(pi/2 - 0.1) ~ 1.4.
  config_.cotThetaMax = 1.5;

  // cotThetaMax and deltaRMax matter to choose the binning in z. The bin size
  // is given by cotThetaMax*deltaRMax

  config_.sigmaScattering = 2.25;
  config_.minPt = 500. * Acts::UnitConstants::MeV;
  config_.bFieldInZ = 1.5 * Acts::UnitConstants::T;
  config_.beamPos = {0, 0};  // units mm ?
  config_.impactMax = 40.;

  grid_conf_.bFieldInZ = config_.bFieldInZ;
  grid_conf_.minPt = config_.minPt;
  grid_conf_.rMax = config_.rMax;
  grid_conf_.zMax = config_.zMax;
  grid_conf_.zMin = config_.zMin;
  grid_conf_.deltaRMax = config_.deltaRMax;
  grid_conf_.cotThetaMax = config_.cotThetaMax;
  grid_conf_.impactMax = config_.impactMax;
  grid_conf_.phiBinDeflectionCoverage = 1.;  // renamed from numPhiNeighbors

  // The seed finder needs a seed filter instance
  // In the seed finder there is the correction for the beam axis, which you
  // could ignore if you set the penalty for high impact parameters. So removing
  // that in the seeder config.

  // Seed filter configuration
  seed_filter_cfg_.impactWeightFactor = 0.;

  // For the moment no experiment dependent cuts are assigned to the filter
  config_.seedFilter = std::make_unique<Acts::SeedFilter<ldmx::LdmxSpacePoint>>(
      Acts::SeedFilter<ldmx::LdmxSpacePoint>(seed_filter_cfg_));

  seed_finder_ =
      std::make_shared<Acts::Seedfinder<ldmx::LdmxSpacePoint>>(config_);

  // In ACTS coordinates and BField in Tesla
  bField_(0) = 0.;
  bField_(1) = 0.;
  bField_(2) = config_.bFieldInZ;

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

  /*
  //ldmx_log(info) << "Converted " << ldmxsps.size() << " hits";
  //TODO:: For the moemnt I am only selecting 3 points. In principle the grid
  should be able to find all combinatorics
  //I'll use layer 3,5,7.

  std::vector<const ldmx::LdmxSpacePoint*> spVec;
  Acts::Extent rRangeSPExtent;

  for (size_t isp = 0; isp < ldmxsps.size(); isp++) {

    //std::cout<<"isp:"<<isp<<ldmxsps[isp]->layer()<<std::endl;

    int lyID = (ldmxsps[isp]->layer() / 100) % 10;
    int sID  = (ldmxsps[isp]->layer()) % 2;
    int layerID = (lyID - 1 ) * 2 + sID;

    //std::cout<<(int) isp<<" "<<(ldmxsps[isp]->layer())<<" "<<lyID<<" "<<sID<<"
  "<<layerID<<std::endl;

    if (layerID == 3 || layerID == 7 || layerID == 9) {
      spVec.push_back(ldmxsps[isp]);
      rRangeSPExtent.check({ldmxsps[isp]->x(), ldmxsps[isp]->y(),
  ldmxsps[isp]->z()});
    }
  }

  //ldmx_log(info) <<"Will use spVec::"<<spVec.size()<<" hits ";
  //covariance tool: returns the covariance from the space point
  auto covariance_tool = [=](const ldmx::LdmxSpacePoint& sp, float, float,
                             float) -> std::pair<Acts::Vector3, Acts::Vector2> {
    Acts::Vector3 position{sp.x(), sp.y(), sp.z()};
    Acts::Vector2 covariance{sp.varianceR(), sp.varianceZ()};
    return std::make_pair(position, covariance);
  };

  //std::cout<<"Creating the grid"<<std::endl;

  std::unique_ptr<Acts::SpacePointGrid<ldmx::LdmxSpacePoint> > grid =
  Acts::SpacePointGridCreator::createGrid<ldmx::LdmxSpacePoint>(grid_conf_);

  //std::cout<<"Creating the space point group"<<std::endl;

  // create the space point group
  auto spGroup = Acts::BinnedSPGroup<ldmx::LdmxSpacePoint>(
      spVec.begin(), spVec.end(),
      covariance_tool,
      bottom_bin_finder_, top_bin_finder_,
      std::move(grid), config_);


  // seed vector
  using SeedContainer = std::vector<Acts::Seed<ldmx::LdmxSpacePoint>>;
  SeedContainer seeds;
  seeds.clear();

  Acts::Seedfinder<ldmx::LdmxSpacePoint>::State state;

  // find the seeds
  auto group = spGroup.begin();
  auto group_end = spGroup.end();

  for (; !(group == group_end); ++group) {
    seed_finder_->createSeedsForGroup(state, std::back_inserter(seeds),
  group.bottom(), group.middle(), group.top(),rRangeSPExtent);
  }

  int numSeeds = seeds.size();

  */
  /*

  for (auto& seed : seeds) {

    auto ldmxspvec = seed.sp();

    //Fit the seeds and extract the track parameters

    //Local Surface (u,v,w) frame (wrt ACTS frame) rotation
    Acts::Vector3 uaxis{0,0,1};
    Acts::Vector3 vaxis{0,-1,0};
    Acts::Vector3 waxis{1,0,0};

    Acts::RotationMatrix3 s_rotation;
    s_rotation.col(0) = uaxis;
    s_rotation.col(1) = vaxis;
    s_rotation.col(2) = waxis;

    //Translation to the reference plane origin
    //TODO:: Attach the space points to the surfaces and provide the origin on
  surface.

    //Acts::Vector3 surface_origin{0.,0.,0.};
    //Acts::Translation3 s_trans(0.,0.,0.); //wrong

    Acts::Vector3 g_pos = ldmxspvec[0]->getGlobalPosition();
    Acts::Translation3 s_trans(g_pos);


    //Build the local to global transform
    Acts::Transform3 tP(s_trans * s_rotation);

    //TODO:: Correct the 6th parameter (time)
    auto params = seed_to_track_maker_->estimateTrackParamsFromSeed(tP,
                                                                    ldmxspvec.begin(),ldmxspvec.end(),
                                                                    bField_, 1.
  * Acts::UnitConstants::T, 0.5);

    ldmx::Track trk = ldmx::Track();

    trk.setPerigeeLocation(g_pos(0), g_pos(1), g_pos(2));
    trk.setChi2(0.);
    trk.setNhits(3);
    trk.setNdf(0);
    trk.setNsharedHits(0);

    if (!params)
      return;


    std::vector<double> v_seed_params((*params).data(), (*params).data() +
  (*params).rows() * (*params).cols()); std::vector<double> v_seed_cov;
    tracking::sim::utils::flatCov(Acts::BoundSymMatrix::Identity(), v_seed_cov);
    trk.setPerigeeParameters(v_seed_params);
    trk.setPerigeeCov(v_seed_cov);

    //Get momentum and position - TODO
    seed_tracks.push_back(trk);
  }
  */

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

  int found = 0;

  for (auto& meas : measurements) {
    if (std::find(strategy.begin(), strategy.end(), meas.getLayer()) !=
        strategy.end()) {
      groups_map[meas.getLayer()].push_back(&meas);
      found++;
    }

  }  // loop meas

  if (found < 5)
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
