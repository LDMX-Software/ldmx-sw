#include "Recon/ParticleFlow.h"

#include <vector>

namespace recon {

void ParticleFlow::configure(framework::config::Parameters& ps) {
  // I/O
  input_ecal_coll_name_ = ps.get<std::string>("inputEcalCollName");
  input_hcal_coll_name_ = ps.get<std::string>("inputHcalCollName");
  input_track_coll_name_ = ps.get<std::string>("inputTrackCollName");
  output_coll_name_ = ps.get<std::string>("outputCollName");
  input_ecal_passname_ = ps.get<std::string>("input_ecal_passname");
  input_hcal_passname_ = ps.get<std::string>("input_hcal_passname");
  input_tracks_passname_ = ps.get<std::string>("input_tracks_passname");

  // Algorithm configuration
  single_particle_ = ps.get<bool>("singleParticle");
  use_existing_ecal_clusters_ = ps.get<bool>("use_existing_ecal_clusters");

  // Calibration factors, from jason, temperary
  std::vector<float> em1{250.0,  750.0,  1250.0, 1750.0, 2250.0, 2750.0,
                         3250.0, 3750.0, 4250.0, 4750.0, 5250.0, 5750.};
  std::vector<float> em2{1.175, 1.02, 0.99, 0.985, 0.975, 0.975,
                         0.96,  0.94, 0.87, 0.8,   0.73,  0.665};
  std::vector<float> h1{25.0,  75.0,  125.0, 175.0, 225.0,
                        275.0, 325.0, 375.0, 425.0};
  std::vector<float> h2{8.44,  7.38,  7.76, 8.535, 9.47,
                        10.45, 10.47, 9.71, 8.87};
  e_corr_ = new TGraph(em1.size(), em1.data(), em2.data());
  h_corr_ = new TGraph(h1.size(), h1.data(), h2.data());
}

// produce candidate track info
void ParticleFlow::fillCandTrack(ldmx::PFCandidate& cand,
                                 const ldmx::SimTrackerHit& tk) {
  // TODO: smear
  std::vector<float> xyz = tk.getPosition();
  std::vector<double> pxyz = tk.getMomentum();
  float ecal_z = 248;
  float ecal_x =
      xyz[0] + pxyz[0] / pxyz[2] * (ecal_z - xyz[2]);  // project onto ecal face
  float ecal_y = xyz[1] + pxyz[1] / pxyz[2] * (ecal_z - xyz[2]);
  cand.setEcalPositionXYZ(ecal_x, ecal_y, ecal_z);
  cand.setTrackPxPyPz(pxyz[0], pxyz[1], pxyz[2]);
  // also use this object to set truth info
  cand.setTruthEcalXYZ(ecal_x, ecal_y, ecal_z);
  cand.setTruthPxPyPz(pxyz[0], pxyz[1], pxyz[2]);
  float m2 = pow(tk.getEnergy(), 2) - pow(pxyz[0], 2) - pow(pxyz[1], 2) -
             pow(pxyz[2], 2);
  if (m2 < 0) m2 = 0;
  cand.setTruthMass(sqrt(m2));
  cand.setTruthEnergy(tk.getEnergy());
  cand.setTruthPdgId(tk.getPdgID());
  cand.setPID(cand.getPID() | 1);  // OR with 001
}
// produce candidate ECal info
void ParticleFlow::fillCandEMCalo(ldmx::PFCandidate& cand,
                                  const ldmx::CaloCluster& em) {
  float corr = 1.;
  float energy = em.getEnergy();
  // update energy: use min or max factor if outside calibration range
  if (energy < e_corr_->GetX()[0]) {
    corr = e_corr_->GetY()[0];
  } else if (energy > e_corr_->GetX()[e_corr_->GetN() - 1]) {
    corr = e_corr_->GetY()[e_corr_->GetN() - 1];
  } else {  // else look up calibration factor
    corr = e_corr_->Eval(energy);
  }
  cand.setEcalEnergy(energy * corr);
  cand.setEcalRawEnergy(energy);
  cand.setEcalClusterXYZ(em.getCentroidX(), em.getCentroidY(),
                         em.getCentroidZ());
  cand.setEcalClusterEXYZ(em.getRMSX(), em.getRMSY(), em.getRMSZ());
  cand.setEcalClusterDXDZ(em.getDXDZ());
  cand.setEcalClusterDYDZ(em.getDYDZ());
  cand.setEcalClusterEDXDZ(em.getEDXDZ());
  cand.setEcalClusterEDYDZ(em.getEDYDZ());
  cand.setPID(cand.getPID() | 2);  // OR with 010
}
// produce candidate HCal info
void ParticleFlow::fillCandHadCalo(ldmx::PFCandidate& cand,
                                   const ldmx::CaloCluster& had) {
  float corr = 1.;
  float energy = had.getEnergy();
  if (energy < h_corr_->GetX()[0]) {
    corr = h_corr_->GetY()[0];
  } else if (energy > h_corr_->GetX()[h_corr_->GetN() - 1]) {
    corr = h_corr_->GetY()[h_corr_->GetN() - 1];
  } else {
    corr = h_corr_->Eval(energy);
  }
  cand.setHcalEnergy(energy * corr);
  cand.setHcalRawEnergy(energy);
  cand.setHcalClusterXYZ(had.getCentroidX(), had.getCentroidY(),
                         had.getCentroidZ());
  cand.setHcalClusterEXYZ(had.getRMSX(), had.getRMSY(), had.getRMSZ());
  cand.setHcalClusterDXDZ(had.getDXDZ());
  cand.setHcalClusterDYDZ(had.getDYDZ());
  cand.setHcalClusterEDXDZ(had.getEDXDZ());
  cand.setHcalClusterEDYDZ(had.getEDYDZ());
  cand.setPID(cand.getPID() | 4);  // OR with 100
}

// produce candidate calorimeter info (any type)
void ParticleFlow::fillCandCalo(ldmx::PFCandidate& cand,
                                const ldmx::CaloCluster& cl, TGraph gResponse,
                                int PIDnb) {
  float corr = 1.;
  float energy = cl.getEnergy();
  // update energy: use min or max factor if outside calibration range
  if (energy < gResponse.GetX()[0]) {
    corr = gResponse.GetY()[0];
  } else if (energy > gResponse.GetX()[gResponse.GetN() - 1]) {
    corr = gResponse.GetY()[gResponse.GetN() - 1];
  } else {  // else look up calibration factor
    corr = gResponse.Eval(energy);
  }
  cand.setEcalEnergy(energy * corr);
  cand.setEcalRawEnergy(energy);
  cand.setEcalClusterXYZ(cl.getCentroidX(), cl.getCentroidY(),
                         cl.getCentroidZ());
  cand.setEcalClusterEXYZ(cl.getRMSX(), cl.getRMSY(), cl.getRMSZ());
  cand.setEcalClusterDXDZ(cl.getDXDZ());
  cand.setEcalClusterDYDZ(cl.getDYDZ());
  cand.setEcalClusterEDXDZ(cl.getEDXDZ());
  cand.setEcalClusterEDYDZ(cl.getEDYDZ());
  cand.setPID(cand.getPID() | PIDnb);  // set calo PID number bit
}

// produce track, ecal, and hcal linking
void ParticleFlow::produce(framework::Event& event) {
  if (!event.exists(input_track_coll_name_, input_tracks_passname_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << input_track_coll_name_ << "_" << input_tracks_passname_;
    return;
  }
  if (!event.exists(input_ecal_coll_name_, input_ecal_passname_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << input_ecal_coll_name_ << "_" << input_ecal_passname_;
    return;
  }
  if (!event.exists(input_hcal_coll_name_, input_hcal_passname_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << input_hcal_coll_name_ << "_" << input_hcal_passname_;
    return;
  }
  // get the track and clustering info
  const auto hcal_clusters = event.getCollection<ldmx::CaloCluster>(
      input_hcal_coll_name_, input_hcal_passname_);
  const auto tracks = event.getCollection<ldmx::SimTrackerHit>(
      input_track_coll_name_, input_tracks_passname_);
  // here allow for using existing clusters of different type (EcalCluster)
  const auto ecal_clusters =
      use_existing_ecal_clusters_
          ? getEcalClusters(event, input_ecal_coll_name_, input_ecal_passname_)
          : event.getCollection<ldmx::CaloCluster>(input_ecal_coll_name_,
                                                   input_ecal_passname_);

  std::vector<ldmx::PFCandidate> pf_cands;
  // multi-particle case
  if (!single_particle_) {
    /*
      1. Build links maps at the Tk/Ecal and Hcal/Hcal interfaces
      2. Categorize tracks as: Ecal-matched, (Side) Hcal-matched, unmatched
      3. Categorize Ecal clusters as: Hcal-matched, unmatched
      4a. (Upstream?) Categorize tracks with dedx?
      4b. (Upstream?) Categorize ecal clusters as: EM/Had-like
      4c. (Upstream?) Categorize hcal clusters as: EM/Had-like
      5. Build candidates by category, moving from Tk-Ecal-Hcal
    */

    //
    // track-calo linking
    //
    std::map<int, std::vector<int> > tk_calo_map;
    std::map<int, std::vector<int> > calo_tk_map;
    std::map<std::pair<int, int>, float> tk_em_dist;
    // std::vector<int> unmatchedTracks;
    for (int i = 0; i < tracks.size(); i++) {
      const auto& tk = tracks[i];
      const std::vector<float> xyz = tk.getPosition();
      const std::vector<double> pxyz = tk.getMomentum();
      const float p = sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
      // float bestMatchVal = 9e9;
      for (int j = 0; j < ecal_clusters.size(); j++) {
        const auto& ecal = ecal_clusters[j];
        // Matching logic
        const float ecal_clus_z = ecal.getCentroidZ();
        const float tk_x_at_clus =
            xyz[0] +
            pxyz[0] / pxyz[2] * (ecal_clus_z - xyz[2]);  // extrapolation
        const float tk_y_at_clus =
            xyz[1] + pxyz[1] / pxyz[2] * (ecal_clus_z - xyz[2]);
        float dist = hypot((tk_x_at_clus - ecal.getCentroidX()) /
                               std::max(1.0, ecal.getRMSX()),
                           (tk_y_at_clus - ecal.getCentroidY()) /
                               std::max(1.0, ecal.getRMSY()));
        tk_em_dist[{i, j}] = dist;
        bool is_match =
            (dist < 2) && (ecal.getEnergy() > 0.3 * p &&
                           ecal.getEnergy() < 2 * p);  // matching criteria *
        // if (isMatch && dist < bestMatchVal) bestMatchVal = dist;
        if (is_match) {
          if (tk_calo_map.count(i))
            tk_calo_map[i].push_back(j);
          else
            tk_calo_map[i] = {j};
          if (calo_tk_map.count(j))
            calo_tk_map[j].push_back(i);
          else
            calo_tk_map[j] = {i};
        }
      }
    }

    // em-hadcalo linking
    std::map<int, std::vector<int> > em_had_calo_map;
    std::map<std::pair<int, int>, float> em_had_dist;
    for (int i = 0; i < ecal_clusters.size(); i++) {
      const auto& ecal = ecal_clusters[i];
      for (int j = 0; j < hcal_clusters.size(); j++) {
        const auto& hcal = hcal_clusters[j];
        // TODO: matching logic
        const float x_at_h_clus =
            ecal.getCentroidX() +
            ecal.getDXDZ() * (hcal.getCentroidZ() -
                              ecal.getCentroidZ());  // extrapolated position
        const float y_at_h_clus =
            ecal.getCentroidY() +
            ecal.getDYDZ() * (hcal.getCentroidZ() - ecal.getCentroidZ());
        float dist = sqrt(
            pow(x_at_h_clus - hcal.getCentroidX(), 2) /
                std::max(1.0, pow(hcal.getRMSX(), 2) + pow(ecal.getRMSX(), 2)) +
            pow(y_at_h_clus - hcal.getCentroidY(), 2) /
                std::max(1.0, pow(hcal.getRMSY(), 2) + pow(ecal.getRMSY(), 2)));
        em_had_dist[{i, j}] = dist;
        bool is_match = (dist < 5);  // matching criteria, was 2
        if (is_match) {
          if (em_had_calo_map.count(i))
            em_had_calo_map[i].push_back(j);
          else
            em_had_calo_map[i] = {j};
        }
      }
    }

    // NOT YET IMPLEMENTED...
    // tk-hadcalo linking (Side HCal)
    std::map<int, std::vector<int> > tk_had_calo_map;
    // for(int i=0; i<tracks.size(); i++){
    //   const auto& tk = tracks[i];
    //   for(int j=0; j<hcalClusters.size(); j++){
    // 	const auto& hcal = hcalClusters[j];
    // 	// TODO: add the matching logic here...
    // 	bool isMatch = true;
    // 	if(isMatch){
    // 	  if (tkHadCaloMap.count(i)) tkHadCaloMap[i].push_back(j);
    // 	  else tkHadCaloMap[i] = {j};
    // 	}
    //   }
    // }

    //
    // track / ecal cluster arbitration
    //
    std::vector<bool> tk_is_em_linked(tracks.size(), false);
    std::vector<bool> em_is_tk_linked(ecal_clusters.size(), false);
    std::map<int, int> tk_em_pairs{};
    for (int i = 0; i < tracks.size(); i++) {
      if (tk_calo_map.count(i)) {
        // pick first (highest-energy) unused matching cluster
        for (int em_idx : tk_calo_map[i]) {
          if (!em_is_tk_linked[em_idx]) {
            em_is_tk_linked[em_idx] = true;
            tk_is_em_linked[i] = true;
            tk_em_pairs[i] = em_idx;
            break;
          }
        }
      }
    }

    // track / hcal cluster arbitration
    std::vector<bool> em_is_had_linked(ecal_clusters.size(), false);
    std::vector<bool> had_is_em_linked(hcal_clusters.size(), false);
    std::map<int, int> em_had_pairs{};
    for (int i = 0; i < ecal_clusters.size(); i++) {
      if (em_had_calo_map.count(i)) {
        // pick first (highest-energy) unused matching cluster
        for (int had_idx : em_had_calo_map[i]) {
          if (!had_is_em_linked[had_idx]) {
            had_is_em_linked[had_idx] = true;
            em_is_had_linked[i] = true;
            em_had_pairs[i] = had_idx;
            break;
          }
        }
      }
    }

    // can consider combining satellite clusters here...
    // define some "primary cluster" ID criterion
    //   and can add fails to the primaries

    //
    // Begin building pf candidates from tracks
    //

    // std::vector<ldmx::PFCandidate> chargedMatch;
    // std::vector<ldmx::PFCandidate> chargedUnmatch;
    for (int i = 0; i < tracks.size(); i++) {
      ldmx::PFCandidate cand;
      fillCandTrack(cand, tracks[i]);  // append track info to candidate

      cand.setTrackIndex(i);
      if (!tk_is_em_linked[i]) {
        // chargedUnmatch.push_back(cand);
      } else {  // if track is linked with ECal cluster
        fillCandEMCalo(cand, ecal_clusters[tk_em_pairs[i]]);
        if (em_is_had_linked[tk_em_pairs[i]]) {  // if ECal is linked with HCal
                                                 // cluster
          fillCandHadCalo(cand, hcal_clusters[em_had_pairs[tk_em_pairs[i]]]);
        }
        // chargedMatch.push_back(cand);
      }
      pf_cands.push_back(cand);
    }

    // std::vector<ldmx::PFCandidate> emMatch;
    // std::vector<ldmx::PFCandidate> emUnmatch;
    for (int i = 0; i < ecal_clusters.size(); i++) {
      // already linked with ECal in the previous step
      if (em_is_tk_linked[i]) continue;

      ldmx::PFCandidate cand;
      fillCandEMCalo(cand, ecal_clusters[i]);
      cand.setEcalIndex(i);
      if (em_is_had_linked[tk_em_pairs[i]]) {
        fillCandHadCalo(cand, hcal_clusters[em_had_pairs[i]]);
        // emMatch.push_back(cand);
      } else {
        // emUnmatch.push_back(cand);
      }
      pf_cands.push_back(cand);
    }
    std::vector<ldmx::PFCandidate> had_only;
    for (int i = 0; i < hcal_clusters.size(); i++) {
      if (had_is_em_linked[i]) continue;
      ldmx::PFCandidate cand;
      fillCandHadCalo(cand, hcal_clusters[i]);
      cand.setHcalIndex(i);
      // hadOnly.push_back(cand);
      pf_cands.push_back(cand);
    }

    // // track / ecal cluster arbitration
    // std::vector<ldmx::PFCandidate> caloMatchedTks;
    // std::vector<ldmx::PFCandidate> unmatchedTks;
    // std::vector<bool> emUsed (ecalClusters.size(), false);
    // for(int i=0; i<tracks.size(); i++){
    //   ldmx::PFCandidate cand;
    //   fillCandTrack(cand, tracks[i]);
    //   if( tkCaloMap.count(i)==0 ){
    // 	unmatchedTks.push_back(cand);
    //   } else {
    // 	// look for the first (highest-energy) unused matching cluster
    // 	bool linked=false;
    // 	for(int em_idx : tkCaloMap[i]){
    // 	  if(!emUsed[em_idx]){
    // 	    fillCandEMCalo(cand, tkCaloMap[i][0]);
    // 	    caloMatchedTks.push_back(cand);
    // 	    emUsed[ em_idx ] = true;
    // 	    linked = true;
    // 	    break;
    // 	  }
    // 	}
    // 	if (!linked) unmatchedTks.push_back(cand);
    //   }
    // }

    // // ecal / hcal cluster arbitration
    // std::vector<bool> hadUsed (hcalClusters.size(), false);
    // for(int i=0; i<ecalClusters.size(); i++){
    //   if( emHadCaloMap.count(i)==0 ){
    // 	unmatchedTks.push_back(cand);
    //   } else {
    // 	// look for the first (highest-energy) unused matching cluster
    // 	bool linked=false;
    // 	for(int em_idx : tkCaloMap[i]){
    // 	  if(!emUsed[em_idx]){
    // 	    fillCandEMCalo(cand, tkCaloMap[i][0]);
    // 	    caloMatchedTks.push_back(cand);
    // 	    emUsed[ em_idx ] = true;
    // 	    linked = true;
    // 	    break;
    // 	  }
    // 	}
    // 	if (!linked) unmatchedTks.push_back(cand);
    //   }
    // }

    // std::vector<ldmx::PFCandidate> unmatchedEMs;
    // for(int i=0; i<ecalClusters.size(); i++){
    //   if(emUsed[i]) continue;
    //   ldmx::PFCandidate cand;
    //   fillCandEMCalo(cand, ecalClusters[i]);
    // }

    // 	}
    // 	if( tkCaloMap[i].size()==1 ){
    // 	if(!emUsed[ tkCaloMap[i][0] ]){
    // 	  fillCandEMCalo(cand, tkCaloMap[i][0]);
    // 	  caloMatchedTks.push_back(cand);
    // 	  emUsed[ tkCaloMap[i][0] ] = true;
    // 	}
    //   }
    //   } else if( tkCaloMap[i].size()==1 ){
    // 	if(!emUsed[ tkCaloMap[i][0] ]){
    // 	  fillCandEMCalo(cand, tkCaloMap[i][0]);
    // 	  caloMatchedTks.push_back(cand);
    // 	  emUsed[ tkCaloMap[i][0] ] = true;
    // 	}
    //   }
    // }

  } else {
    // Single-particle builder
    ldmx::PFCandidate pf;
    int pid = 0;  // initialize pid to add
    if (tracks.size()) {
      fillCandTrack(pf, tracks[0]);
      pid += 1;
    }
    if (ecal_clusters.size()) {
      fillCandEMCalo(pf, ecal_clusters[0]);
      pid += 2;
    }
    if (hcal_clusters.size()) {
      fillCandHadCalo(pf, hcal_clusters[0]);
      pid += 4;
    }
    pf.setPID(pid);
    pf.setEnergy(pf.getEcalEnergy() + pf.getHcalEnergy());
    pf_cands.push_back(pf);
  }

  event.add(output_coll_name_, pf_cands);
}
// stupid function to type cast from ecal to calo cluster
const std::vector<ldmx::CaloCluster> ParticleFlow::getEcalClusters(
    framework::Event& event, std::string inputClusterCollName,
    std::string inputClusterPassName) {
  const auto tmp_clusters = event.getCollection<ldmx::EcalCluster>(
      inputClusterCollName, inputClusterPassName);
  std::string new_name = inputClusterCollName + "Cast";
  std::vector<ldmx::CaloCluster> new_clusters;
  for (auto cl : tmp_clusters) {
    new_clusters.emplace_back(cl);
  }
  event.add(new_name, new_clusters);
  const auto calo_clusters =
      event.getCollection<ldmx::CaloCluster>(new_name, "");
  return calo_clusters;
}

void ParticleFlow::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";
  delete e_corr_;
  delete h_corr_;

  return;
}

}  // namespace recon

DECLARE_PRODUCER(recon::ParticleFlow);
