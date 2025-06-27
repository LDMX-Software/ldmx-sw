#include "Recon/ParticleFlowBrem.h"

#include <vector>
#include <fstream>


namespace recon {

std::pair<float, float> ParticleFlowBrem::getTrajectory(
    std::vector<double> momentum, std::vector<float> position, float z) {
  XYCoords positions;
 // for (int iLayer = 0; iLayer < nEcalLayers_; iLayer++) {
    //std::cout << iLayer << " " << geometry_->getZPosition(iLayer) << std::endl;
    float posX =
        position[0] + (momentum[0] / momentum[2]) *
                          (z - position[2]);
    float posY =
        position[1] + (momentum[1] / momentum[2]) *
                          (z - position[2]);
    //positions.push_back(std::make_pair(posX, posY));
    positions = std::make_pair(posX, posY);
 // }
  return positions;
}

void ParticleFlowBrem::configure(framework::config::Parameters& ps) {
  // I/O
  inputEcalCollName_ = ps.getParameter<std::string>("inputEcalCollName");
  inputHcalCollName_ = ps.getParameter<std::string>("inputHcalCollName");
  inputTaggerTrackCollName_ = ps.getParameter<std::string>("inputTaggerTrackCollName");
  inputRecoilTrackCollName_ = ps.getParameter<std::string>("inputRecoilTrackCollName");

  beamEnergyGeV_ = ps.getParameter<double>("beam_energy");


  outputCollName_ = ps.getParameter<std::string>("outputCollName");
  // Algorithm configuration
  singleParticle_ = ps.getParameter<bool>("singleParticle");
  rocFileName_ = ps.getParameter<std::string>("roc_file");

  // Calibration factors, from jason, temperary
  std::vector<float> em1{250.0,  750.0,  1250.0, 1750.0, 2250.0, 2750.0,
                         3250.0, 3750.0, 4250.0, 4750.0, 5250.0, 5750.};
  std::vector<float> em2{1.175, 1.02, 0.99, 0.985, 0.975, 0.975,
                         0.96,  0.94, 0.87, 0.8,   0.73,  0.665};
  std::vector<float> h1{25.0,  75.0,  125.0, 175.0, 225.0,
                        275.0, 325.0, 375.0, 425.0};
  std::vector<float> h2{8.44,  7.38,  7.76, 8.535, 9.47,
                        10.45, 10.47, 9.71, 8.87};
  eCorr_ = new TGraph(em1.size(), em1.data(), em2.data());
  hCorr_ = new TGraph(h1.size(), h1.data(), h2.data());

  // Read in arrays holding 68% containment radius per layer
  // for different bins in momentum/angle
  if (!std::ifstream(rocFileName_).good()) {
    EXCEPTION_RAISE(
        "EcalVetoProcessor",
        "The specified RoC file '" + rocFileName_ + "' does not exist!"); 
  } else {
    std::ifstream rocfile(rocFileName_);
    std::string line, value;

    // Extract the first line in the file
    std::getline(rocfile, line);
    std::vector<double> values;

    // Read data, line by line
    while (std::getline(rocfile, line)) {
      std::stringstream ss(line);
      values.clear();
      while (std::getline(ss, value, ',')) {
        double f_value = (value != "") ? std::stof(value) : -1.0;
        values.push_back(f_value);
      }
      roc_range_values_.push_back(values);
    }
  }


}

// produce candidate track info
void ParticleFlowBrem::fillCandTrack(ldmx::PFCandidateBrem& cand,
                                 const ldmx::Track& trk, int idx) {

  cand.setRecoilTrackPxPyPz(trk.getMomentum()[1], trk.getMomentum()[2], trk.getMomentum()[0]);
  cand.setTargetRecPositionXYZ(trk.getD0(), trk.getZ0(), 0);

  auto ts =  trk.getTrackState(ldmx::TrackStateType::AtECAL);
  if (ts.has_value()) {
    ldmx::Track::TrackState& ecalTrackState = ts.value();
    cand.setEcalPositionXYZ(ecalTrackState.params[0], ecalTrackState.params[1], ecalTrackState.refX);
    double track_state_phi = ecalTrackState.params[2];
    double track_state_theta = ecalTrackState.params[3];
    double track_state_p = 1/abs(ecalTrackState.params[4]);
    double px = track_state_p * sin(track_state_theta) * sin(track_state_phi);
    double py = track_state_p * cos(track_state_theta);
    double pz = track_state_p * sin(track_state_theta) * cos(track_state_phi);
    cand.setEcalMomentumXYZ(px, py, pz);
  }

  cand.setPID(trk.getTrackID());  // OR with 001
  cand.setTrackIdx(idx);
}
// produce candidate ECal info
void ParticleFlowBrem::fillCandEMCalo(ldmx::PFCandidateBrem& cand,
                                  const ldmx::EcalCluster& em, int idx) {
  float corr = 1.;
  float e = em.getEnergy();
  if (e < eCorr_->GetX()[0]) {
    corr = eCorr_->GetX()[0];
  } else if (e > eCorr_->GetX()[eCorr_->GetN() - 1]) {
    corr = eCorr_->GetX()[eCorr_->GetN() - 1];
  } else {
    corr = eCorr_->Eval(e);
  }
  cand.setEcalEnergy(e * corr);
  cand.setEcalRawEnergy(e);
  cand.setEcalClusterXYZ(em.getCentroidX(), em.getCentroidY(),
                         em.getCentroidZ());
  cand.setEcalClusterEXYZ(em.getRMSX(), em.getRMSY(), em.getRMSZ());
  cand.setEcalClusterDXDZ(em.getDXDZ());
  cand.setEcalClusterDYDZ(em.getDYDZ());
  cand.setEcalClusterEDXDZ(em.getEDXDZ());
  cand.setEcalClusterEDYDZ(em.getEDYDZ());
  cand.setECalClusterHits(em.getHitIDs());
  cand.setClusterIdx(idx);
  //cand.setPID(cand.getPID() | 2);  // OR with 010
}
// produce candidate ECal info
void ParticleFlowBrem::fillCandBremCalo(ldmx::PFCandidateBrem& cand,
                                  const ldmx::EcalCluster& em, int idx) {
  float corr = 1.;
  float e = em.getEnergy();
  if (e < eCorr_->GetX()[0]) {
    corr = eCorr_->GetX()[0];
  } else if (e > eCorr_->GetX()[eCorr_->GetN() - 1]) {
    corr = eCorr_->GetX()[eCorr_->GetN() - 1];
  } else {
    corr = eCorr_->Eval(e);
  }
  cand.addBremProduct(0);
  cand.setBremProductEnergy(0, e);
  cand.setBremProductECalPositionXYZ(0, em.getCentroidX(), em.getCentroidY(),
                         em.getCentroidZ());
  cand.setBremProductECalHits(0, em.getHitIDs());
  cand.setBremClusterIdx(0, idx);

  //cand.setPID(cand.getPID() | 2);  // OR with 010
}
// produce candidate HCal info
void ParticleFlowBrem::fillCandHadCalo(ldmx::PFCandidateBrem& cand,
                                   const ldmx::CaloCluster& had) {
  float corr = 1.;
  float e = had.getEnergy();
  if (e < hCorr_->GetX()[0]) {
    corr = hCorr_->GetX()[0];
  } else if (e > hCorr_->GetX()[hCorr_->GetN() - 1]) {
    corr = hCorr_->GetX()[hCorr_->GetN() - 1];
  } else {
    corr = hCorr_->Eval(e);
  }
  cand.setHcalEnergy(e * corr);
  cand.setHcalRawEnergy(e);
  cand.setHcalClusterXYZ(had.getCentroidX(), had.getCentroidY(),
                         had.getCentroidZ());
  cand.setHcalClusterEXYZ(had.getRMSX(), had.getRMSY(), had.getRMSZ());
  cand.setHcalClusterDXDZ(had.getDXDZ());
  cand.setHcalClusterDYDZ(had.getDYDZ());
  cand.setHcalClusterEDXDZ(had.getEDXDZ());
  cand.setHcalClusterEDYDZ(had.getEDYDZ());
  cand.setPID(cand.getPID() | 4);  // OR with 100
}

// produce track, ecal, and hcal linking
void ParticleFlowBrem::produce(framework::Event& event) {

   geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  const auto ecalClusters =
      event.getCollection<ldmx::EcalCluster>(inputEcalCollName_,"sim");
  const auto taggerTracks =
      event.getCollection<ldmx::Track>(inputTaggerTrackCollName_);
  const auto recoilTracks =
      event.getCollection<ldmx::Track>(inputRecoilTrackCollName_);

    std::map<int, ldmx::PFCandidateBrem> pfCands;

    std::map<int, std::vector<int>> recoilClusterMap;
    std::map<int, std::vector<int>> recoilBremClusterMap;
    std::map<int, std::vector<int>> recoilTaggerMap;

    std::vector<int> clusterLink(ecalClusters.size(), 0);

    for (int i = 0; i < recoilTracks.size(); i++) {
        std::vector<int> linkedClusters;
        std::vector<double> linkedDistances;
        std::vector<int> bremClusters;
        std::vector<double> bremDistances;
        ldmx::PFCandidateBrem pfCand;
        for (int j = 0; j < ecalClusters.size(); j++) {

            double clusterZ = ecalClusters[j].getCentroidZ();
            auto ts =  recoilTracks[i].getTrackState(ldmx::TrackStateType::AtECAL);
            if (!ts.has_value()) continue;
            ldmx::Track::TrackState& ecalTrackState = ts.value();
            std::vector<float> track_position = {ecalTrackState.params[0], ecalTrackState.params[1], ecalTrackState.refX};

            //std::cout << ecalTrackState.params[0] << " " << ecalTrackState.params[1] << " " << ecalTrackState.refX << std::endl;

            double track_state_phi = ecalTrackState.params[2];
            double track_state_theta = ecalTrackState.params[3];
            double track_state_p = 1/abs(ecalTrackState.params[4]);
            double px = track_state_p * sin(track_state_theta) * sin(track_state_phi);
            double py = track_state_p * cos(track_state_theta);
            double pz = track_state_p * sin(track_state_theta) * cos(track_state_phi);
            std::vector<double> track_momentum = {px, py, pz};
            // Extrapolating the track to the cluster centroid
            auto track_pos_clusterZ = getTrajectory(track_momentum, track_position, clusterZ);

            //Extrapolating the photon to the cluster centroid:
            auto pvec = recoilTracks[i].getMomentum();
            //auto posx = recoilTracks[i].getParams()[0];
            //auto posy = recoilTracks[i].getParams()[1];
            std::vector<float> posvec = {recoilTracks[i].getD0(), recoilTracks[i].getZ0(), 0.0};
            auto photon_trajectory =
                  getTrajectory({-pvec[1], -pvec[2], beamEnergyGeV_ - pvec[0]}, posvec, clusterZ);

            // Getting distance between cluster and track
            auto link_distance = sqrt(pow(ecalClusters[j].getCentroidX() - track_pos_clusterZ.first, 2) +  
                                    pow(ecalClusters[j].getCentroidY() - track_pos_clusterZ.second, 2));

            // Getting distance between photon and track
            auto brem_distance = sqrt(pow(ecalClusters[j].getCentroidX() - photon_trajectory.first, 2) +  
                                    pow(ecalClusters[j].getCentroidY() - photon_trajectory.second, 2));

            //std::cout << "DEBUG" << std::endl;
            //std::cout << pvec[0] << " " << pvec[1] << " " << pvec[2] << std::endl;
            //std::cout << track_pos_clusterZ.first << " " << track_pos_clusterZ.second << std::endl;
            //std::cout << ecalClusters[j].getCentroidX() << " " << ecalClusters[j].getCentroidY() << std::endl;
            //std::cout << link_distance << std::endl;

            // Getting angle wrt ecal face
            float recoilTheta = track_state_p > 0 ? acos(track_momentum[2] / track_state_p) * 180.0 / M_PI : -1.0;

            // Getting the right RoC region
            // Use the appropriate containment radii for the recoil electron
            std::vector<double> roc_values_bin0(roc_range_values_[0].begin() + 4,
                                      roc_range_values_[0].end());
            std::vector<double> ele_radii = roc_values_bin0;
            double theta_min, theta_max, p_min, p_max;
            bool inrange;

            for (int i = 0; i < roc_range_values_.size(); i++) {
              theta_min = roc_range_values_[i][0];
              theta_max = roc_range_values_[i][1];
              p_min = roc_range_values_[i][2];
              p_max = roc_range_values_[i][3]; 
              inrange = true;

              if (theta_min != -1.0) {
                inrange = inrange && (recoilTheta >= theta_min);
              }
              if (theta_max != -1.0) {
                inrange = inrange && (recoilTheta < theta_max);
              }
              if (p_min != -1.0) {
                inrange = inrange && (track_state_p >= p_min);
              }
              if (p_max != -1.0) {
                inrange = inrange && (track_state_p < p_max);
              }
              if (inrange) {
                std::vector<double> roc_values_bini(roc_range_values_[i].begin() + 4,
                                          roc_range_values_[i].end());
                ele_radii = roc_values_bini;
              }
            }

            // Use default RoC bin for photon
            std::vector<double> photon_radii = roc_values_bin0;
            // Next need to get the RoC that applies to the cluster centroid...
            // Need to get the right layer for that z-value. Hmmm...
            int layerZ = geometry_->getClosestID(track_pos_clusterZ.first, track_pos_clusterZ.second,clusterZ);
            int layerZ_brem = geometry_->getClosestID(photon_trajectory.first, photon_trajectory.second,clusterZ);

            if (link_distance < ele_radii[layerZ]) {
              linkedDistances.push_back(link_distance);
              linkedClusters.push_back(j);
            }

             if (brem_distance < photon_radii[layerZ]) {
              bremDistances.push_back(brem_distance);
              bremClusters.push_back(j);
            }
        }

        int min_cluster = -1;
        int min_cluster_brem = -1;
        double min_cluster_dist = 1000;
        for (int k = 0; k < linkedClusters.size(); k++) {
          if (linkedDistances[k] < min_cluster_dist) {
            min_cluster = linkedClusters[k];
            min_cluster_dist = linkedDistances[k];
          }
        }
        min_cluster_dist = 1000;
        for (int k = 0; k < bremClusters.size(); k++) {
          if (bremDistances[k] < min_cluster_dist) {
            min_cluster_brem = bremClusters[k];
            min_cluster_dist = bremDistances[k];
          }
        }
        fillCandTrack(pfCand, recoilTracks[i], i);
        //std::cout << min_cluster << std::endl;
        //std::cout << min_cluster_brem << std::endl;

        if (linkedClusters.size() > 0) {fillCandEMCalo(pfCand, ecalClusters[min_cluster], min_cluster);}
        if (bremClusters.size() > 0) {fillCandBremCalo(pfCand, ecalClusters[min_cluster_brem], min_cluster_brem);}

        pfCands[i] = pfCand;


    }


  std::vector<ldmx::PFCandidateBrem> values;
    for (const auto& pair : pfCands) {
        values.push_back(pair.second);
    }

  event.add(outputCollName_, values);
}

void ParticleFlowBrem::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";
  delete eCorr_;
  delete hCorr_;

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, ParticleFlowBrem);
