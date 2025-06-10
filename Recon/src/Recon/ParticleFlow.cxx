#include "Recon/ParticleFlow.h"

#include <vector>

namespace recon {

void ParticleFlow::configure(framework::config::Parameters& ps) {
  // I/O
  inputEcalCollName_ = ps.getParameter<std::string>("inputEcalCollName");
  inputHcalCollName_ = ps.getParameter<std::string>("inputHcalCollName");
  inputTrackCollName_ = ps.getParameter<std::string>("inputTrackCollName");
  outputCollName_ = ps.getParameter<std::string>("outputCollName");
  // Algorithm configuration
  singleParticle_ = ps.getParameter<bool>("singleParticle");
  tkHadCaloMatchDist_ = ps.getParameter<double>("tkHadCaloMatchDist");
  tkHadCaloMinEnergyRatio_ = ps.getParameter<double>("tkHadCaloMinEnergyRatio");
  tkHadCaloMaxEnergyRatio_ = ps.getParameter<double>("tkHadCaloMaxEnergyRatio");

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
}

// produce candidate track info
void ParticleFlow::fillCandTrack(ldmx::PFCandidate& cand,
                                 const ldmx::SimTrackerHit& tk) {
  // TODO: smear
  std::vector<float> xyz = tk.getPosition();
  std::vector<double> pxyz = tk.getMomentum();
  float ecalZ = 248;
  float ecalX =
      xyz[0] + pxyz[0] / pxyz[2] * (ecalZ - xyz[2]);  // project onto ecal face
  float ecalY = xyz[1] + pxyz[1] / pxyz[2] * (ecalZ - xyz[2]);
  cand.setEcalPositionXYZ(ecalX, ecalY, ecalZ);
  cand.setTrackPxPyPz(pxyz[0], pxyz[1], pxyz[2]);
  // also use this object to set truth info
  cand.setTruthEcalXYZ(ecalX, ecalY, ecalZ);
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
  cand.setPID(cand.getPID() | 2);  // OR with 010
}
// produce candidate HCal info
void ParticleFlow::fillCandHadCalo(ldmx::PFCandidate& cand,
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
void ParticleFlow::produce(framework::Event& event) {
  if (!event.exists(inputTrackCollName_)) return;
  if (!event.exists(inputEcalCollName_)) return;
  if (!event.exists(inputHcalCollName_)) return;
  // get the track and clustering info
  const auto ecalClusters =
      event.getCollection<ldmx::CaloCluster>(inputEcalCollName_);
  const auto hcalClusters =
      event.getCollection<ldmx::CaloCluster>(inputHcalCollName_);
  const auto tracks =
      event.getCollection<ldmx::SimTrackerHit>(inputTrackCollName_);

  std::vector<ldmx::PFCandidate> pfCands;
  // multi-particle case
  if (!singleParticle_) {
    /*
      1. Build links maps at the Tk/Ecal and Hcal/Hcal interfaces
      2. Categorize tracks as: Ecal-matched, (Side) Hcal-matched, unmatched
      3. Categorize Ecal clusters as: Hcal-matched, unmatched
      4a. (Upstream?) Categorize tracks with dedx?
      4b. (Upstream?) Categorize ecal clusters as: EM/Had-like
      4c. (Upstream?) Categorize hcal clusters as: EM/Had-like
      5. Build candidates by category, moving from Tk-Ecal-Hcal
    */
    std::cout << "PF Multiple Particles for Event: "<< event.getEventNumber() << std::endl;
    //
    // track-calo linking
    //
    std::map<int, std::vector<int> > tkCaloMap;
    std::map<int, std::vector<int> > caloTkMap;
    std::map<std::pair<int, int>, float> tkEMDist;
    // std::vector<int> unmatchedTracks;
    for (int i = 0; i < tracks.size(); i++) {
      const auto& tk = tracks[i];
      const std::vector<float> xyz = tk.getPosition();
      const std::vector<double> pxyz = tk.getMomentum();
      const float p = sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
      // float bestMatchVal = 9e9;
      for (int j = 0; j < ecalClusters.size(); j++) {
        const auto& ecal = ecalClusters[j];
        // Matching logic
        const float ecalClusZ = ecal.getCentroidZ();
        const float tkXAtClus =
            xyz[0] + pxyz[0] / pxyz[2] * (ecalClusZ - xyz[2]);  // extrapolation
        const float tkYAtClus =
            xyz[1] + pxyz[1] / pxyz[2] * (ecalClusZ - xyz[2]);
        float dist = hypot(
            (tkXAtClus - ecal.getCentroidX()) / std::max(1.0, ecal.getRMSX()),
            (tkYAtClus - ecal.getCentroidY()) / std::max(1.0, ecal.getRMSY()));
        tkEMDist[{i, j}] = dist;
        bool isMatch =
            (dist < 2) && (ecal.getEnergy() > 0.3 * p &&
                           ecal.getEnergy() < 2 * p);  // matching criteria *
        // if (isMatch && dist < bestMatchVal) bestMatchVal = dist;
        if (isMatch) {
          if (tkCaloMap.count(i))
            tkCaloMap[i].push_back(j);
          else
            tkCaloMap[i] = {j};
          if (caloTkMap.count(j))
            caloTkMap[j].push_back(i);
          else
            caloTkMap[j] = {i};
        }
      }
    }

    // em-hadcalo linking
    std::map<int, std::vector<int> > emHadCaloMap;
    std::map<std::pair<int, int>, float> EMHadDist;
    for (int i = 0; i < ecalClusters.size(); i++) {
      const auto& ecal = ecalClusters[i];
      for (int j = 0; j < hcalClusters.size(); j++) {
        const auto& hcal = hcalClusters[j];
        // TODO: matching logic
        const float xAtHClus =
            ecal.getCentroidX() +
            ecal.getDXDZ() * (hcal.getCentroidZ() -
                              ecal.getCentroidZ());  // extrapolated position
        const float yAtHClus =
            ecal.getCentroidY() +
            ecal.getDYDZ() * (hcal.getCentroidZ() - ecal.getCentroidZ());
        float dist = sqrt(
            pow(xAtHClus - hcal.getCentroidX(), 2) /
                std::max(1.0, pow(hcal.getRMSX(), 2) + pow(ecal.getRMSX(), 2)) +
            pow(yAtHClus - hcal.getCentroidY(), 2) /
                std::max(1.0, pow(hcal.getRMSY(), 2) + pow(ecal.getRMSY(), 2)));
        EMHadDist[{i, j}] = dist;
        bool isMatch = (dist < 5);  // matching criteria, was 2
        if (isMatch) {
          if (emHadCaloMap.count(i))
            emHadCaloMap[i].push_back(j);
          else
            emHadCaloMap[i] = {j};
        }
      }
    }

    // tk-hadcalo linking (Side HCal) BY TILLRUE
    std::map<int, std::vector<int> > tkHadCaloMap;
    std::map<int, std::vector<int> > HadCaloTkMap;
    std::map<std::pair<int, int>, float> tkHadCaloDist;
    for(int i=0; i<tracks.size(); i++){
      const auto& tk = tracks[i];
      const std::vector<float> xyz = tk.getPosition();
      const std::vector<double> pxyz = tk.getMomentum();
      const float p = sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));

      for(int j=0; j<hcalClusters.size(); j++){
       	const auto& hcal = hcalClusters[j];
        // Matching logic same as for track ecalCluster matching
        const float hcalClusZ = hcal.getCentroidZ();
        const float tkXAtClus =
            xyz[0] + pxyz[0] / pxyz[2] * (hcalClusZ - xyz[2]);  // extrapolation
        const float tkYAtClus =
            xyz[1] + pxyz[1] / pxyz[2] * (hcalClusZ - xyz[2]);
        float dist = hypot(
            (tkXAtClus - hcal.getCentroidX()) / std::max(1.0, hcal.getRMSX()),
            (tkYAtClus - hcal.getCentroidY()) / std::max(1.0, hcal.getRMSY()));
        tkHadCaloDist[{i, j}] = dist;
        bool isMatch =
            (dist < tkHadCaloMatchDist_) && (hcal.getEnergy() > tkHadCaloMinEnergyRatio_ * p &&
                           hcal.getEnergy() < tkHadCaloMaxEnergyRatio_ * p);  // matching criteria, need to be configured
         if (isMatch) {
          if (tkHadCaloMap.count(i))
            tkHadCaloMap[i].push_back(j);
          else
            tkHadCaloMap[i] = {j};
          if (HadCaloTkMap.count(j))
            HadCaloTkMap[j].push_back(i);
          else
            HadCaloTkMap[j] = {i};
        }
      }
    }

    //
    // track / ecal cluster arbitration
    //
    std::vector<bool> tkIsEMLinked(tracks.size(), false);
    std::vector<bool> EMIsTkLinked(ecalClusters.size(), false);
    std::map<int, int> tkEMPairs{};
    for (int i = 0; i < tracks.size(); i++) {
      if (tkCaloMap.count(i)) {
        // pick first (highest-energy) unused matching cluster
        for (int em_idx : tkCaloMap[i]) {
          if (!EMIsTkLinked[em_idx]) {
            EMIsTkLinked[em_idx] = true;
            tkIsEMLinked[i] = true;
            tkEMPairs[i] = em_idx;
            break;
          }
        }
      }
    }

    // ecal / hcal cluster arbitration
    std::vector<bool> EMIsHadLinked(ecalClusters.size(), false);
    std::vector<bool> HadIsEMLinked(hcalClusters.size(), false);
    std::map<int, int> EMHadPairs{};
    std::map<int, int> HadEMPairs{};
    
    for (int i = 0; i < ecalClusters.size(); i++) {
      if (emHadCaloMap.count(i)) {
        // pick first (highest-energy) unused matching cluster
        for (int had_idx : emHadCaloMap[i]) {
          if (!HadIsEMLinked[had_idx]) {
            HadIsEMLinked[had_idx] = true;
            EMIsHadLinked[i] = true;
            EMHadPairs[i] = had_idx;
            HadEMPairs[had_idx] = i;
            break;
          }
        }
      }
    }

    //
    // track / hcal cluster arbitration BY TILLRUE
    //
    std::vector<bool> tkIsHadLinked(tracks.size(), false);
    std::vector<bool> HadIsTkLinked(hcalClusters.size(), false);
    std::map<int, int> tkHadPairs{};
    for (int i = 0; i < tracks.size(); i++) {
      if (tkHadCaloMap.count(i)) {
        // pick first (highest-energy) unused matching cluster
        for (int had_idx : tkHadCaloMap[i]) {
          if (!HadIsTkLinked[had_idx]) {
            HadIsTkLinked[had_idx] = true;
            tkIsHadLinked[i] = true;
            tkHadPairs[i] = had_idx;
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

    // loop through tracks
    for (int i = 0; i < tracks.size(); i++) {
      ldmx::PFCandidate cand;
      fillCandTrack(cand, tracks[i]);  // append track info to candidate
      if (tkIsEMLinked[i]){ // if track is linked with ECal cluster
        fillCandEMCalo(cand, ecalClusters[tkEMPairs[i]]);
        if (EMIsHadLinked[tkEMPairs[i]]) {  // if ECal is linked with HCal cluster
          if (!tkIsHadLinked[i]){   // if track is NOT also linked with Hcal
          fillCandHadCalo(cand, hcalClusters[EMHadPairs[tkEMPairs[i]]]);}
        }
      }
      
      if(tkIsHadLinked[i]){ // if track is linked with Hcal
        fillCandHadCalo(cand, hcalClusters[tkHadPairs[i]]);
        if(HadIsEMLinked[tkHadPairs[i]]){ // if hcal is linked with ecal
          if(!tkIsEMLinked[i]){ // if ecal is NOT also linked with track
          fillCandEMCalo(cand,ecalClusters[HadEMPairs[tkHadPairs[i]]]);
          }
        } else if(!tkIsEMLinked[i]){
          cand.setDistTkHcalMatch(tkHadCaloDist[{i,tkHadPairs[i]}]);
          if(hcalClusters.size() > 1){
            cand.setHcalSecondEnergy(hcalClusters[tkHadPairs[i]+1].getEnergy());
            cand.setDistTkHcalSecondCluster(tkHadCaloDist[{i,tkHadPairs[i]+1}]);
          }
        }
      }
      pfCands.push_back(cand);
    }

    // std::vector<ldmx::PFCandidate> emMatch;
    // std::vector<ldmx::PFCandidate> emUnmatch;
    for (int i = 0; i < ecalClusters.size(); i++) {
      if (EMIsTkLinked[i]) continue; // already linked with ECal with Track in the previous step
      if (EMIsHadLinked[i] && HadIsTkLinked[EMHadPairs[i]]) continue; // already linked with track through Hcal

      ldmx::PFCandidate cand;
      fillCandEMCalo(cand, ecalClusters[i]);
      if (EMIsHadLinked[i]) { // is Ecal is linked with Hcal
        fillCandHadCalo(cand, hcalClusters[EMHadPairs[i]]);
        // emMatch.push_back(cand);
      } else {
        // emUnmatch.push_back(cand);
      }
      pfCands.push_back(cand);
    }

    // std::vector<ldmx::PFCandidate> hadOnly;
    for (int i = 0; i < hcalClusters.size(); i++) {
      if (HadIsEMLinked[i]) continue; // already linked with ecal
      if (HadIsTkLinked[i]) continue; // already linked with track
      ldmx::PFCandidate cand;
      fillCandHadCalo(cand, hcalClusters[i]);
      // hadOnly.push_back(cand);
      pfCands.push_back(cand);
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
    std::cout << "Matching Single Particle Signals" << std::endl;
    // Single-particle builder
    ldmx::PFCandidate pf;
    int pid = 0;  // initialize pid to add
    if (tracks.size()) {
      fillCandTrack(pf, tracks[0]);
      pid += 1;
    }
    if (ecalClusters.size()) {
      fillCandEMCalo(pf, ecalClusters[0]);
      pid += 2;
    }
    if (hcalClusters.size()) {
      fillCandHadCalo(pf, hcalClusters[0]);
      pid += 4;
    }
    pf.setPID(pid);
    pf.setEnergy(pf.getEcalEnergy() + pf.getHcalEnergy());
    pfCands.push_back(pf);
  }

  event.add(outputCollName_, pfCands);
}

void ParticleFlow::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";
  delete eCorr_;
  delete hCorr_;

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, ParticleFlow);
