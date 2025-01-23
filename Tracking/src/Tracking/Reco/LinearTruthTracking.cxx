#include "Tracking/Reco/LinearTruthTracking.h"

#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Seeding/EstimateTrackParamsFromSeed.hpp"
#include "Eigen/Dense"
#include "Tracking/Sim/TrackingUtils.h"

namespace tracking {
namespace reco {


LinearTruthTracking::LinearTruthTracking(const std::string& name, framework::Process& process)
: TrackingGeometryUser(name, process) {}

LinearTruthTracking::~LinearTruthTracking() {}

void LinearTruthTracking::onProcessStart() {
    truthMatchingTool_ = std::make_shared<tracking::sim::TruthMatchingTool>();
}

void LinearTruthTracking::configure(framework::config::Parameters& parameters) {
    // Output seed name
    out_trk_collection_ = parameters.getParameter<std::string>("out_trk_collection", "LinearRecoilTruthTracks");
    
    // Input strip hits
    input_hits_collection_ = parameters.getParameter<std::string>("input_hits_collection", "DigiRecoilSimHits");
    input_recHits_collection_ = parameters.getParameter<std::string>("input_recHits_collection", "EcalRecHits");
}

void LinearTruthTracking::produce(framework::Event& event) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<ldmx::StraightTrack> straight_truth_tracks;
    nevents_++;
    
    const std::vector<ldmx::Measurement> recoilHits = event.getCollection<ldmx::Measurement>(input_hits_collection_);
    const std::vector<ldmx::EcalHit> ecalRecHit = event.getCollection<ldmx::EcalHit>(input_recHits_collection_);
    
    std::vector<std::array<double, 3>> firstLayerEcalRecHits;
    
    for (const auto& x_ecal : ecalRecHit) {
        if (x_ecal.getZPos() < 250) {
            firstLayerEcalRecHits.push_back({x_ecal.getZPos(), x_ecal.getXPos(), x_ecal.getYPos()});
        } //if first layer of Ecal
    } //for positions in ecalRecHit
    
    std::map<int, ldmx::SimParticle> particleMap;
    if (event.exists("SimParticles")) {
      particleMap = event.getMap<int, ldmx::SimParticle>("SimParticles");
      truthMatchingTool_->setup(particleMap, recoilHits);
    }

//     ! Check if we would fit empty seeds !
    if (recoilHits.size() < 2) { // || uniqueSensorsHit(recoilHits) < 2) {
        nmissing_++;
        ntruth_ += straight_truth_tracks.size();
        return;
    }
    
    std::vector<ldmx::Measurement> truthPoints;

    if (truthMatchingTool_->configured()) {
        truthPoints = findTruthHits(recoilHits);
    }
    
    if (truthPoints.size() > 0) {
        straight_truth_tracks.push_back(TruthTracker(truthPoints, firstLayerEcalRecHits));
    }
    else {
        nmissing_++;
        ntruth_ += straight_truth_tracks.size();
        return;
    }

    ntruth_ += straight_truth_tracks.size();
    event.add(out_trk_collection_, straight_truth_tracks);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
    
    firstLayerEcalRecHits.clear();
    straight_truth_tracks.clear();
    
} //produce

ldmx::StraightTrack LinearTruthTracking::TruthTracker(const std::vector<ldmx::Measurement>& points, std::vector<std::array<double, 3>>& ecalPoints) {
    ldmx::StraightTrack trk = ldmx::StraightTrack();

    auto [ax, bx, ay, by] = fit3DLine(points);

    trk.setSlopeX(ax);
    trk.setInterceptX(bx);
    trk.setSlopeY(ay);
    trk.setInterceptY(by);
    trk.setTargetLocation(0.0, bx, by);
    trk.setNhits(points.size());
    trk.setNdf(points.size() - 2);
    
    if (truthMatchingTool_->configured()) {
        auto truthInfo = truthMatchingTool_->TruthMatch(points);
        trk.setTrackID(truthInfo.trackID);
        trk.setPdgID(truthInfo.pdgID);
        trk.setTruthProb(truthInfo.truthProb);
    }

    trk.setAllSensorPoints(points);

    trk.setChi2(globalChiSquare(points, ax, ay, bx, by));
    
    if (ecalPoints.size() > 0) {
        double ecal_firstLayer_z = ecalPoints[0][0];  // Z position from the first point of ecalPoints
        
        std::array<double, 3> extrapolatedPoint = {ecal_firstLayer_z, ax * ecal_firstLayer_z + bx, ay * ecal_firstLayer_z + by};
        
        // Initialize closestRecHit and minDistance
        const std::array<double, 3>* closestRecHit = nullptr;
        double minDistance = std::numeric_limits<double>::max();
        
        // Loop through ecalPoints to find the closest recHit
        for (const auto& ecalRecHit : ecalPoints) {
            double tempDistance = calculateDistance(extrapolatedPoint, ecalRecHit);
            
            if (tempDistance < minDistance) {
                minDistance = tempDistance;
                closestRecHit = &ecalRecHit; // Update to the closest recHit
            }
        }
        
        if (closestRecHit != nullptr) {
            trk.setFirstLayerEcalRecHit(*closestRecHit);  // Dereference the pointer to pass by value
            trk.setDistancetoRecHit(minDistance);
            trk.setEcalLayer1Location(extrapolatedPoint);
        }
    } //make sure there are ecalPoints to work with

    return trk;
}

void LinearTruthTracking::onProcessEnd() {
    ldmx_log(info) << "AVG Time / Event: " << std::fixed << std::setprecision(1)
        << processing_time_ / nevents_ << " ms";
    ldmx_log(info) << "Number of Events without enough seed points: " << nmissing_;
    ldmx_log(info) << "Total Truth Tracks / Event: " << ntruth_ << "/" << nevents_;
} //onProcessEnd

std::vector<ldmx::Measurement> LinearTruthTracking::findTruthHits(const std::vector<ldmx::Measurement>& hitCollection) {
    std::vector<ldmx::Measurement> layer1, layer2, layer3, layer4;
    
    // Split hits into layers based on z position
    for (const auto& point : hitCollection) {
        if (point.getGlobalPosition()[0] < 12) layer1.push_back(point);
        else if (point.getGlobalPosition()[0] < 20) layer2.push_back(point);
        else if (point.getGlobalPosition()[0] < 28) layer3.push_back(point);
        else layer4.push_back(point);
    }
    
    for (const auto& hit1 : layer1) {
        for (const auto& hit2 : layer2) {
            for (const auto& hit3 : layer3) {
                for (const auto& hit4 : layer4) {
                    std::vector<ldmx::Measurement> combination = {hit1, hit2, hit3, hit4};
                    auto truthInfo = truthMatchingTool_->TruthMatch(combination);
                    if (truthInfo.trackID == 1.0) {
                        return combination;
                    } //if trackID
                } //for layer4
            } //for layer3
        } //for layer2
    } //for layer1

    return {};
    
} //findTruthHits


double LinearTruthTracking::calculateDistance(const std::array<double, 3> &point1, const std::array<double, 3> &point2) {
    return sqrt(pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2));
} //calculateDistance

std::tuple<double, double, double, double> LinearTruthTracking::fit3DLine(const std::vector<ldmx::Measurement>& points) {

    if (points.size() < 2) {
        throw std::invalid_argument("At least two points are required to fit a 3D line.");
    }

    std::vector<double> z_vals, x_vals, y_vals;

    for (const auto& point : points) {
        const auto& position = point.getGlobalPosition();
        z_vals.push_back(position[0]);
        x_vals.push_back(position[1]);
        y_vals.push_back(position[2]);
    } //get coordinates

    double sigma_x = recoil_truth_uncertainty_[0];
    double sigma_y = recoil_truth_uncertainty_[1];
    std::vector<double> weights(2 * points.size()); //weighting

    for (size_t i = 0; i < points.size(); ++i) {
        weights[2 * i] = 1.0 / (sigma_x * sigma_x);  // Weight for x
        weights[2 * i + 1] = 1.0 / (sigma_y * sigma_y);  // Weight for y
    }

    Eigen::MatrixXd A(2 * points.size(), 4);  // 2 equations for each point (x and y fitting)
    Eigen::VectorXd d(2 * points.size());    // Data vector (x and y values)
    Eigen::VectorXd w(2 * points.size());    // Weight vector

    // Fill the A matrix and d vector
    for (size_t i = 0; i < points.size(); ++i) {
        double z = z_vals[i], x = x_vals[i], y = y_vals[i];

        // Fill the A matrix (z, 1, 0, 0) for x and (0, 0, z, 1) for y
        A(2 * i, 0) = z;
        A(2 * i, 1) = 1;
        A(2 * i, 2) = 0;
        A(2 * i, 3) = 0;

        A(2 * i + 1, 0) = 0;
        A(2 * i + 1, 1) = 0;
        A(2 * i + 1, 2) = z;
        A(2 * i + 1, 3) = 1;

        // Fill the d vector with x and y values
        d(2 * i) = x;
        d(2 * i + 1) = y;

        // Fill the weights vector
        w(2 * i) = weights[2 * i];   // Weight for x
        w(2 * i + 1) = weights[2 * i + 1]; // Weight for y
    }

    // Solve the weighted least squares system
    Eigen::MatrixXd At_W_A = A.transpose() * w.asDiagonal() * A;
    Eigen::MatrixXd At_W_d = A.transpose() * w.asDiagonal() * d;
    Eigen::VectorXd m = At_W_A.ldlt().solve(At_W_d);

    // Return results (ax, bx, ay, by)
    return {m(0), m(1), m(2), m(3)};
}

double LinearTruthTracking::globalChiSquare(const std::vector<ldmx::Measurement>& points, double ax, double ay, double bx, double by) {

    double chi2_x = 0, chi2_y = 0;

    for (const auto& point : points) {
        const auto& position = point.getGlobalPosition();
        double z = position[0];
        double x = position[1];
        double y = position[2];

        double x_fit = ax * z + bx;
        double y_fit = ay * z + by;

        chi2_x += std::pow((x - x_fit) / recoil_truth_uncertainty_[0], 2);
        chi2_y += std::pow((y - y_fit) / recoil_truth_uncertainty_[1], 2);
    }

    return chi2_x + chi2_y;
}


int LinearTruthTracking::uniqueSensorsHit(const std::vector<ldmx::Measurement>& digiPoints) {
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

DECLARE_PRODUCER_NS(tracking::reco, LinearTruthTracking);
