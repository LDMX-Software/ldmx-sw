#include "Tracking/Reco/ReducedSeedFinder.h"

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Eigen/Dense"
#include "Tracking/Sim/TrackingUtils.h"

namespace tracking {
namespace reco {


ReducedSeedFinder::ReducedSeedFinder(const std::string& name, framework::Process& process)
: TrackingGeometryUser(name, process) {}

ReducedSeedFinder::~ReducedSeedFinder() {}

void ReducedSeedFinder::onProcessStart() {}

void ReducedSeedFinder::configure(framework::config::Parameters& parameters) {
    // Output seed name
    out_seed_collection_ = parameters.getParameter<std::string>("out_seed_collection", getName() + "ReducedSeedTracks");
    
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

void ReducedSeedFinder::produce(framework::Event& event) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<ldmx::ReducedTrack> reduced_seed_tracks;
    nevents_++;
    
    const std::vector<ldmx::Measurement> recoilHits = event.getCollection<ldmx::Measurement>(input_hits_collection_);
    const std::vector<ldmx::EcalHit> ecalRecHit = event.getCollection<ldmx::EcalHit>(input_recHits_collection_);
    
    for (const auto& x_digi : recoilHits) {
        zpos_digi_tot_.push_back(x_digi.getGlobalPosition()[0]);
        xpos_digi_tot_.push_back(x_digi.getGlobalPosition()[1]);
        ypos_digi_tot_.push_back(x_digi.getGlobalPosition()[2]);
        edep_digi_.push_back(x_digi.getEdep());
    } //for loop to create digiPoints array
    
    // Combine (z, x, y, edep) recoil hits into a 3D position vector
    for (size_t i = 0; i < zpos_digi_tot_.size(); ++i) {
        digiPoints_.push_back({zpos_digi_tot_[i], xpos_digi_tot_[i], ypos_digi_tot_[i], edep_digi_[i]});
    } //for positions in recoil tracker
    
    for (const auto& x_ecal : ecalRecHit) {
        if (x_ecal.getZPos() < 250) {
            ecal_end_x_.push_back(x_ecal.getXPos());
            ecal_end_y_.push_back(x_ecal.getYPos());
            ecal_end_z_.push_back(x_ecal.getZPos());
        } //if first layer of Ecal
    } //for positions in ecalRecHit
    
    // Combine (z, x, y) ecalRecHits into a 3D position vector
    for (size_t i = 0; i < ecal_end_z_.size(); ++i) {
        firstLayerEcalRecHits_.push_back({ecal_end_z_[i], ecal_end_x_[i], ecal_end_y_[i]});
    } //for loop to create ecal_endpoint array
    
    // ! Check if we would fit empty seeds !
    if (digiPoints_.size() < 2 || firstLayerEcalRecHits_.empty() || uniqueSensorsHit(digiPoints_) < 2) {
        nmissing_++;
        ntracks_ += reduced_seed_tracks.size();
        event.add(out_seed_collection_, reduced_seed_tracks);
        return;
    }
        
    auto [firstSensor, secondSensor] = combineMultiGlobalHits(digiPoints_);
    
    for (const auto& firstPoint : firstSensor) {
        for (const auto& secondPoint : secondSensor) {
            for (const auto& recHit : firstLayerEcalRecHits_) {
                ldmx::ReducedTrack seedTrack = SeedTracker(firstPoint, secondPoint, recHit);
                reduced_seed_tracks.push_back(seedTrack);
            } //for recHits
        } //for second recoil tracker
    } //for first recoil tracker
    
    ntracks_ += reduced_seed_tracks.size();
    event.add(out_seed_collection_, reduced_seed_tracks);
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto diff = end - start;
    processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
    
    digiPoints_.clear();
    firstLayerEcalRecHits_.clear();
    
    zpos_digi_tot_.clear();
    xpos_digi_tot_.clear();
    ypos_digi_tot_.clear();
    edep_digi_.clear();
    
    ecal_end_x_.clear();
    ecal_end_y_.clear();
    ecal_end_z_.clear();
    
} //produce

ldmx::ReducedTrack ReducedSeedFinder::SeedTracker(const std::array<double, 3> recoilOne, const std::array<double, 3> recoilTwo, const std::array<double, 3> ecalOne) {
    
    auto [ax, bx, ay, by] = fit3DLine(recoilOne, recoilTwo, ecalOne);
    std::array<double, 3> tempExtrapolatedPoint = {ecalOne[0], ax * ecalOne[0] + bx, ay * ecalOne[0] + by};
    double tempDistance = calculateDistance(tempExtrapolatedPoint, ecalOne);
    
    ldmx::ReducedTrack trk = ldmx::ReducedTrack();

    if (tempDistance < ecal_distance_threshold_) {
        trk.setAX(ax);
        trk.setBX(bx);
        trk.setAY(ay);
        trk.setBY(by);
        
        trk.setFirstSensorPosition(recoilOne);
        trk.setSecondSensorPosition(recoilTwo);
        trk.setEcalLayer1Location(ecalOne);
        trk.setDistancetoEcalRecHit(tempDistance);
        
        trk.setTargetLocation(0.0, bx, by);
        trk.setEcalLayer1Location(tempExtrapolatedPoint);
        trk.setChi2(globalChiSquare(recoilOne, recoilTwo, ecalOne, ax, ay, bx, by));
        trk.setNhits(3);
        trk.setNdf(1);
        trk.setNsharedHits(0);
    } //check whether the track is close enough to EcalRecHit
    
    return trk;
}

void ReducedSeedFinder::onProcessEnd() { //HAVE TO FIX THESE VALUES
    ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
    << processing_time_ / nevents_ << " ms";
    ldmx_log(info) << "Total Seeds/Events: " << ntracks_ << "/" << nevents_;
    //  ldmx_log(info) << "Seeds discarded due to multiple hits on layers "
    //                 << ndoubles_;
    ldmx_log(info) << "not enough seed points " << nmissing_;
    //  ldmx_log(info) << "   nfailphicut=" << nfailphi_;
    //  ldmx_log(info) << "   nfailthetacut=" << nfailtheta_;
} //onProcessEnd

std::pair<std::vector<std::array<double, 3>>, std::vector<std::array<double, 3>>> ReducedSeedFinder::combineMultiGlobalHits(const std::vector<std::array<double, 4>>& hitCollection) {
    std::vector<std::array<double, 4>> layer1, layer2, layer3, layer4;

    // Split hits into layers based on z position
    for (const auto& point : hitCollection) {
        if (point[0] < 12) layer1.push_back(point);
        else if (point[0] < 20) layer2.push_back(point);
        else if (point[0] < 28) layer3.push_back(point);
        else layer4.push_back(point);
    }

    // Perform weighted averages and convert to 3D arrays (z, x, y)
    auto firstSensorMergedHits = weightedAverage(layer1, layer2);
    auto secondSensorMergedHits = weightedAverage(layer3, layer4);

    return {firstSensorMergedHits, secondSensorMergedHits};
} //combineMultiGlobalHits

std::vector<std::array<double, 3>> ReducedSeedFinder::weightedAverage(const std::vector<std::array<double, 4>>& layer1, const std::vector<std::array<double, 4>>& layer2) {
    
    std::vector<std::array<double, 3>> mergedHits;

    for (const auto& p1 : layer1) {
        for (const auto& p2 : layer2) {
            double totalWeight = p1[3] + p2[3];  // Sum of edep values
            double zAvg = (p1[0] * p1[3] + p2[0] * p2[3]) / totalWeight;
            double xAvg = (p1[1] * p1[3] + p2[1] * p2[3]) / totalWeight;
            double yAvg = (p1[2] * p1[3] + p2[2] * p2[3]) / totalWeight;

            // Only include (z, x, y) in mergedHits
            mergedHits.push_back({zAvg, xAvg, yAvg});
        }
    }
    return mergedHits;
} //weightedAverage

std::tuple<double, double, double, double> ReducedSeedFinder::fit3DLine(const std::array<double, 3> &firstRecoil, const std::array<double, 3> &secondRecoil, const std::array<double, 3> &ECal) {
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

double ReducedSeedFinder::calculateDistance(const std::array<double, 3> &point1, const std::array<double, 3> &point2) {
    return sqrt(pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2));
} //calculateDistance

double ReducedSeedFinder::globalChiSquare(const std::array<double, 3> &firstSensor, const std::array<double, 3> &secondSensor, const std::array<double, 3> &ecalHit, double ax, double ay, double bx, double by) {
    double chi2_x = 0, chi2_y = 0;
    chi2_x += pow((ax * firstSensor[0] + bx - firstSensor[1]) / recoil_uncertainty_[0], 2);
    chi2_y += pow((ay * firstSensor[0] + by - firstSensor[2]) / recoil_uncertainty_[1], 2);
    
    chi2_x += pow((ax * secondSensor[0] + bx - secondSensor[1]) / recoil_uncertainty_[0], 2);
    chi2_y += pow((ay * secondSensor[0] + by - secondSensor[2]) / recoil_uncertainty_[1], 2);
    
    chi2_x += pow((ax * ecalHit[0] + bx - ecalHit[1]) / ecal_uncertainty_, 2);
    chi2_y += pow((ay * ecalHit[0] + by - ecalHit[2]) / ecal_uncertainty_, 2);
    
    return chi2_x + chi2_y;
} //globalChiSquare

int ReducedSeedFinder::uniqueSensorsHit(const std::vector<std::array<double, 4>> &digiPoints) {
    std::unordered_set<int> unique_zs;
    for (const auto &point : digiPoints) {
        unique_zs.insert(static_cast<int>(point[0]));
    }
    return unique_zs.size();
} //uniqueSensorsHit

} //namespace reco
} //namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, ReducedSeedFinder);
