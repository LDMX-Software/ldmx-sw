#include "Tracking/Reco/LinearSeedFinder.h"

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Eigen/Dense"
#include "Tracking/Sim/TrackingUtils.h"

namespace tracking {
namespace reco {


LinearSeedFinder::LinearSeedFinder(const std::string& name, framework::Process& process)
: TrackingGeometryUser(name, process) {}

LinearSeedFinder::~LinearSeedFinder() {}

void LinearSeedFinder::onProcessStart() {
    truthMatchingTool_ = std::make_shared<tracking::sim::TruthMatchingTool>();
}

void LinearSeedFinder::configure(framework::config::Parameters& parameters) {
    // Output seed name
    out_seed_collection_ = parameters.getParameter<std::string>("out_seed_collection", getName() + "LinearRecoilSeedTracks");
    
    // Input strip hits
    input_hits_collection_ = parameters.getParameter<std::string>("input_hits_collection", "DigiRecoilSimHits");
    input_recHits_collection_ = parameters.getParameter<std::string>("input_recHits_collection", "EcalRecHits");
    
    //    phicut_ = parameters.getParameter<double>("phicut", 0.1);
    //    thetacut_ = parameters.getParameter<double>("thetacut", 0.2);
    //
    //    loc0cut_ = parameters.getParameter<double>("loc0cut", 0.1);
    //    loc1cut_ = parameters.getParameter<double>("loc1cut", 0.3);
    
    recoil_uncertainty_ = parameters.getParameter<std::vector<double>>("recoil_uncertainty", {0.006, 0.12});
    ecal_uncertainty_ = parameters.getParameter<double>("ecal_uncertainty");
    ecal_distance_threshold_ = parameters.getParameter<double>("ecal_distance_threshold");
}

void LinearSeedFinder::produce(framework::Event& event) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<ldmx::StraightTrack> straight_seed_tracks;
    nevents_++;
    
    const std::vector<ldmx::Measurement> recoilHits = event.getCollection<ldmx::Measurement>(input_hits_collection_);
    const std::vector<ldmx::EcalHit> ecalRecHit = event.getCollection<ldmx::EcalHit>(input_recHits_collection_);
    
    std::vector<std::array<double, 3>> firstLayerEcalRecHits;
    
    for (const auto& x_ecal : ecalRecHit) {
        if (x_ecal.getZPos() < 250) {
            firstLayerEcalRecHits.push_back({x_ecal.getZPos(), x_ecal.getXPos(), x_ecal.getYPos()});
        } //if first layer of Ecal
    } //for positions in ecalRecHit

    // ! Check if we would fit empty seeds !
    if (recoilHits.size() < 2 || firstLayerEcalRecHits.empty() || uniqueSensorsHit(recoilHits) < 2) {
        nmissing_++;
        nseeds_ += straight_seed_tracks.size();
        return;
    }
    
    std::map<int, ldmx::SimParticle> particleMap;
    if (event.exists("SimParticles")) {
      particleMap = event.getMap<int, ldmx::SimParticle>("SimParticles");
      truthMatchingTool_->setup(particleMap, recoilHits);
    }
        
    auto [firstSensor, secondSensor] = combineMultiGlobalHits(recoilHits);
    
    for (const auto& firstPoint : firstSensor) {
        for (const auto& secondPoint : secondSensor) {
            for (const auto& recHit : firstLayerEcalRecHits) {
                ldmx::StraightTrack seedTrack = SeedTracker(firstPoint, secondPoint, recHit);
                if (seedTrack.getChi2() > 0.0) {
                    straight_seed_tracks.push_back(seedTrack);
                }
            } //for recHits
        } //for second recoil tracker
    } //for first recoil tracker
    
    nseeds_ += straight_seed_tracks.size();
    event.add(out_seed_collection_, straight_seed_tracks);
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto diff = end - start;
    processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
    
    firstLayerEcalRecHits.clear();
    straight_seed_tracks.clear();
    
} //produce

ldmx::StraightTrack LinearSeedFinder::SeedTracker(const std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement> recoilOne, const std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement> recoilTwo, const std::array<double, 3> ecalOne) {
        
    auto [merged1, layer1, layer2] = recoilOne;
    auto [merged2, layer3, layer4] = recoilTwo;
    
    std::vector<ldmx::Measurement> allPoints = {layer1, layer2, layer3, layer4};
    
    auto [ax, bx, ay, by] = fit3DLine(merged1, merged2, ecalOne);
    std::array<double, 3> tempExtrapolatedPoint = {ecalOne[0], ax * ecalOne[0] + bx, ay * ecalOne[0] + by};
    double tempDistance = calculateDistance(tempExtrapolatedPoint, ecalOne);

    ldmx::StraightTrack trk = ldmx::StraightTrack();

    if (tempDistance < ecal_distance_threshold_) {
        trk.setSlopeX(ax);
        trk.setInterceptX(bx);
        trk.setSlopeY(ay);
        trk.setInterceptY(by);
        
        trk.setAllSensorPoints(allPoints);
        trk.setFirstSensorPosition(merged1);
        trk.setSecondSensorPosition(merged2);
        trk.setFirstLayerEcalRecHit(ecalOne);
        trk.setDistancetoRecHit(tempDistance);
        
        trk.setTargetLocation(0.0, bx, by);
        trk.setEcalLayer1Location(tempExtrapolatedPoint);
        trk.setChi2(globalChiSquare(merged1, merged2, ecalOne, ax, ay, bx, by));
        trk.setNhits(3);
        trk.setNdf(1);
        
        if (truthMatchingTool_->configured()) {
            auto truthInfo = truthMatchingTool_->TruthMatch(allPoints);
            ldmx_log(debug) << "setTrackID: " << truthInfo.trackID << "/";
            ldmx_log(debug) << "setPdgID: " << truthInfo.pdgID << "/";
            ldmx_log(debug) << "setTruthProb: " << truthInfo.truthProb << "/";
            ldmx_log(debug) << "Distance to RecHit: " << tempDistance << "/";
            
            trk.setTrackID(truthInfo.trackID);
            trk.setPdgID(truthInfo.pdgID);
            trk.setTruthProb(truthInfo.truthProb);
        }
        
        return trk;
    } //check whether the track is close enough to EcalRecHit
    else {
        trk.setChi2(-1);
        return trk;
    } //does not pass the threshold
}

void LinearSeedFinder::onProcessEnd() { 
    ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
    << processing_time_ / nevents_ << " ms";
    ldmx_log(info) << "Total Seeds/Events: " << nseeds_ << "/" << nevents_;
    ldmx_log(info) << "not enough seed points " << nmissing_;

} //onProcessEnd

std::pair< std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement>>,
           std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement>> >
LinearSeedFinder::combineMultiGlobalHits(const std::vector<ldmx::Measurement>& hitCollection) {
    std::vector<ldmx::Measurement> layer1, layer2, layer3, layer4;

    // Split hits into layers based on z position
    for (const auto& point : hitCollection) {
        if (point.getGlobalPosition()[0] < 12) layer1.push_back(point);
        else if (point.getGlobalPosition()[0] < 20) layer2.push_back(point);
        else if (point.getGlobalPosition()[0] < 28) layer3.push_back(point);
        else layer4.push_back(point);
    }

    // Perform weighted averages and convert to 3D arrays (z, x, y)
    auto firstSensorMergedHits = weightedAverage(layer1, layer2);
    auto secondSensorMergedHits = weightedAverage(layer3, layer4);

    return {firstSensorMergedHits, secondSensorMergedHits};
} //combineMultiGlobalHits

std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement>> LinearSeedFinder::weightedAverage(const std::vector<ldmx::Measurement>& layer1, const std::vector<ldmx::Measurement>& layer2) {
    
    std::vector<std::tuple<std::array<double, 3>, ldmx::Measurement, ldmx::Measurement>> mergedHits;

    for (const auto& p1 : layer1) {
        for (const auto& p2 : layer2) {
            double edepL1 = p1.getEdep();
            double edepL2 = p2.getEdep();
            double zAvg = (p1.getGlobalPosition()[0] * edepL1 + p2.getGlobalPosition()[0] * edepL2) / (edepL1 + edepL2);
            double xAvg = (p1.getGlobalPosition()[1] * edepL1 + p2.getGlobalPosition()[1] * edepL2) / (edepL1 + edepL2);
            double yAvg = (p1.getGlobalPosition()[2] * edepL1 + p2.getGlobalPosition()[2] * edepL2) / (edepL1 + edepL2);
            mergedHits.push_back(std::make_tuple(std::array<double, 3>{zAvg, xAvg, yAvg}, p1, p2));
        }
    }
    return mergedHits;
}

std::tuple<double, double, double, double> LinearSeedFinder::fit3DLine(const std::array<double, 3> &firstRecoil, const std::array<double, 3> &secondRecoil, const std::array<double, 3> &ECal) {
    double z1 = firstRecoil[0], x1 = firstRecoil[1], y1 = firstRecoil[2];
    double z2 = secondRecoil[0], x2 = secondRecoil[1], y2 = secondRecoil[2];
    double z3 = ECal[0], x3 = ECal[1], y3 = ECal[2];
    
    std::array<double, 6> weights = {
        1 / pow(recoil_uncertainty_[0], 2), 1 / pow(recoil_uncertainty_[1], 2),
        1 / pow(recoil_uncertainty_[0], 2), 1 / pow(recoil_uncertainty_[1], 2),
        1 / pow(ecal_uncertainty_, 2), 1 / pow(ecal_uncertainty_, 2)
    };
    
    Eigen::Matrix<double, 6, 4> A;
    Eigen::Matrix<double, 6, 1> d, w;
    A << z1, 1, 0, 0,
    0, 0, z1, 1,
    z2, 1, 0, 0,
    0, 0, z2, 1,
    z3, 1, 0, 0,
    0, 0, z3, 1;
    d << x1, y1, x2, y2, x3, y3;
    w = Eigen::Matrix<double, 6, 1>(weights.data());
    
    Eigen::MatrixXd At_W_A = A.transpose() * w.asDiagonal() * A;
    Eigen::MatrixXd At_W_d = A.transpose() * w.asDiagonal() * d;
    Eigen::VectorXd m = At_W_A.ldlt().solve(At_W_d);
    
    return {m(0), m(1), m(2), m(3)};
} //fit3DLine

double LinearSeedFinder::calculateDistance(const std::array<double, 3> &point1, const std::array<double, 3> &point2) {
    return sqrt(pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2));
} //calculateDistance

double LinearSeedFinder::globalChiSquare(const std::array<double, 3> &firstSensor, const std::array<double, 3> &secondSensor, const std::array<double, 3> &ecalHit, double ax, double ay, double bx, double by) {
    double chi2_x = 0, chi2_y = 0;
    chi2_x += pow((ax * firstSensor[0] + bx - firstSensor[1]) / recoil_uncertainty_[0], 2);
    chi2_y += pow((ay * firstSensor[0] + by - firstSensor[2]) / recoil_uncertainty_[1], 2);
    
    chi2_x += pow((ax * secondSensor[0] + bx - secondSensor[1]) / recoil_uncertainty_[0], 2);
    chi2_y += pow((ay * secondSensor[0] + by - secondSensor[2]) / recoil_uncertainty_[1], 2);
    
    chi2_x += pow((ax * ecalHit[0] + bx - ecalHit[1]) / ecal_uncertainty_, 2);
    chi2_y += pow((ay * ecalHit[0] + by - ecalHit[2]) / ecal_uncertainty_, 2);
    
    return chi2_x + chi2_y;
} //globalChiSquare

int LinearSeedFinder::uniqueSensorsHit(const std::vector<ldmx::Measurement>& digiPoints) {
    // Step 1: Create a copy of digiPoints to allow modifications
    std::vector<ldmx::Measurement> sortedPoints = digiPoints;

    // Step 2: Sort by x-coordinate
    std::sort(sortedPoints.begin(), sortedPoints.end(),
              [](const ldmx::Measurement& m1, const ldmx::Measurement& m2) {
                  return m1.getGlobalPosition()[0] < m2.getGlobalPosition()[0];
              });

    // Step 3: Remove duplicates using std::unique
    auto last = std::unique(sortedPoints.begin(), sortedPoints.end(),
                            [](const ldmx::Measurement& m1, const ldmx::Measurement& m2) {
                                return m1.getGlobalPosition()[0] == m2.getGlobalPosition()[0];
                            });

    // Step 4: Calculate the number of unique elements
    return std::distance(sortedPoints.begin(), last);
} // uniqueSensorsHit

} //namespace reco
} //namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, LinearSeedFinder);
