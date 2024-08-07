#include "TrigScint/PositionAccuracy.h"
#include "TrigScint/Event/PositionDifference.h"
#include "TrigScint/Event/TrigScintTrack.h"
#include "Recon/Event/BeamElectronTruth.h"

namespace trigscint {
    
void PositionAccuracy::configure(framework::config::Parameters &parameters) {
    input_collections_ = parameters.getParameter<std::vector<std::string>>("input_collections");
    inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
    output_collections_ = parameters.getParameter<std::vector<std::string>>("output_collections");
    vertBarStartIdx_ = parameters.getParameter<int>("vertical_bar_start_index");
    padNumber_ = parameters.getParameter<int>("padNumber");
    padPosition_ = parameters.getParameter<std::vector<double>>("padPosition");
    verbose_ = parameters.getParameter<bool>("verbose");
}

void PositionAccuracy::produce(framework::Event &event) {
    // Check if cluster and track input collection exists.
    // If not, do not process event.
    if (!event.exists(input_collections_[0], inputPassName_) ||
        !event.exists(input_collections_[1], inputPassName_)) {
        ldmx_log(fatal) << "Attempting to use non-existing input colelction. "
                        << "Either: " << input_collections_[0] << "_" << inputPassName_
                        << " or " << input_collections_[1] << "_" << inputPassName_
                        << " does not exist. Exiting!";
        return;
    }
    // Check if beam electron position input collection exists.
    // If not, do not process event.
    if (!event.exists(input_collections_[2], inputPassName_)) {
        ldmx_log(fatal) << "Attempting to use non-existing input colelction "
                        << input_collections_[2] << "_" << inputPassName_
                        << " to determine position difference! Exiting.";
        return;
    }

    // Define vectors with position differences for output later
    std::vector<ldmx::PositionDifference> positionDifferenceClusters;
    std::vector<ldmx::PositionDifference> positionDifferenceTracks;

    // Load input clusters
    auto clusters {event.getCollection<ldmx::TrigScintCluster>(
        input_collections_[0], inputPassName_)};
    // Load input tracks
    auto tracks {event.getCollection<ldmx::TrigScintTrack>(
        input_collections_[1], inputPassName_)};
    // Load input beam electrons
    auto beamElectronInfo {event.getCollection<ldmx::BeamElectronTruth>(
        input_collections_[2], inputPassName_)};

    // Pair all horizontal and vertical cluster combination on same z-value
    std::vector<ldmx::TrigScintCluster> clusterPairs = matchXYClusters(clusters);

    // Determine cluster and beam electron position difference
    for (int i = 0; i < clusterPairs.size(); i++) {
        for (int j = 0; j < beamElectronInfo.size(); j++) {
            // Define new position difference
            ldmx::PositionDifference clusterDiff;
            // Set x, y, z uncertainties sx, sy, z
            clusterDiff.setSigmaX(clusterPairs.at(i).getSigmaX());
            clusterDiff.setSigmaY(clusterPairs.at(i).getSigmaY());
            clusterDiff.setSigmaZ(clusterPairs.at(i).getSigmaZ());
            // Set dx, dy, dz differences
            clusterDiff.setDx(clusterPairs.at(i).getX() -
                                (beamElectronInfo.at(j).getX() -
                                padPosition_.at(0)));
            clusterDiff.setDy(clusterPairs.at(i).getY() -
                                (beamElectronInfo.at(j).getY() -
                                padPosition_.at(1)));
            clusterDiff.setDz(clusterPairs.at(i).getZ() -
                                (beamElectronInfo.at(j).getZ() -
                                padPosition_.at(2)));
            // Append clusterDiff to positionDifferenceClusters
            positionDifferenceClusters.push_back(clusterDiff);
        }
    }

    // Determine track and beam electron position difference
    for (int i = 0; i < tracks.size(); i++) {
        for (int j = 0; j < beamElectronInfo.size(); j++) {
            // Define new position difference
            ldmx::PositionDifference trackDiff;
            // Set x, y, z uncertainties sx, sy, sz
            trackDiff.setSigmaX(tracks.at(i).getSigmaX());
            trackDiff.setSigmaY(tracks.at(i).getSigmaY());
            trackDiff.setSigmaZ(tracks.at(i).getSigmaZ());
            // Set dx, dy, dz differences
            trackDiff.setDx(tracks.at(i).getX() -
                                (beamElectronInfo.at(j).getX() -
                                padPosition_.at(0)));
            trackDiff.setDy(tracks.at(i).getY() -
                                (beamElectronInfo.at(j).getY() -
                                padPosition_.at(1)));
            trackDiff.setDz(tracks.at(i).getZ().at(padNumber_-1) -
                                (beamElectronInfo.at(j).getZ() -
                                padPosition_.at(2)));
            // Append clusterDiff to positionDifferenceClusters
            positionDifferenceTracks.push_back(trackDiff);
        }
    }

    // Add event outputs
    event.add(output_collections_[0], positionDifferenceClusters);
    event.add(output_collections_[1], positionDifferenceTracks);
}

std::vector<ldmx::TrigScintCluster> PositionAccuracy::matchXYClusters(std::vector<ldmx::TrigScintCluster> clusters) {
    /** Define matchedClusters: paired horizontal and vertical
     * clusters with both x,y and sx, sy on same z-position in a TS pad
     * Note: These matched clusters will only contain x,y and sx, sy information!
     */
    std::vector<ldmx::TrigScintCluster> matchedClusters;

    // Define vertical (x) and horizontal (y) clusters
    std::vector<ldmx::TrigScintCluster> xClusters;
    std::vector<ldmx::TrigScintCluster> yClusters;

    // Sort out vertical (x) and horizontal (y) clusters
    for (int i = 0; i < clusters.size(); i++) {
        if (clusters.at(i).getCentroid() < vertBarStartIdx_) {
            yClusters.push_back(clusters.at(i));
        } else {
            xClusters.push_back(clusters.at(i));
        }
    }

    // Loop through all horizontal and vertical clusters if both exists
    if (yClusters.size() != 0 && xClusters.size() != 0) {
        for (int i = 0; i < yClusters.size(); i++) {
            for (int j = 0; j < xClusters.size(); j++) {
                // Create new cluster with position
                ldmx::TrigScintCluster newCluster;
                newCluster.setPositionXYZ(xClusters.at(j).getX(),
                                        yClusters.at(i).getY(),
                                        yClusters.at(i).getZ());
                newCluster.setSigmaXYZ(
                    xClusters.at(j).getSigmaX(), yClusters.at(i).getSigmaY(),
                    std::max(xClusters.at(j).getSigmaZ(), yClusters.at(i).getSigmaZ()));
                // Append new cluster to matchedClusters
                matchedClusters.push_back(newCluster);
            }
        }
    }

    // Return all found matched clusters
    return matchedClusters;
}

};  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, PositionAccuracy);