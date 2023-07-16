#include "Recon/ParticleFlow.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Recon/Event/CaloCluster.h"
#include "Ecal/Event/EcalCluster.h"
#include "Hcal/Event/HcalCluster.h"
#include "Recon/Event/PFCandidate.h"
#include <vector>

namespace recon {

void ParticleFlow::configure(framework::config::Parameters& ps) {
  inputEcalCollName_ = ps.getParameter<std::string>("inputEcalCollName");
  inputHcalCollName_ = ps.getParameter<std::string>("inputHcalCollName");
  inputTrackCollName_ = ps.getParameter<std::string>("inputTrackCollName");
  outputCollName_ = ps.getParameter<std::string>("outputCollName"); 
  //from jason
  std::vector<float> em1{250.0,750.0,1250.0,1750.0,2250.0,2750.0,3250.0,3750.0,4250.0,4750.0,5250.0,5750.};
  std::vector<float> em2{1.175,1.02,0.99,0.985,0.975,0.975,0.96,0.94,0.87,0.8,0.73,0.665};
  std::vector<float> h1{25.0,75.0,125.0,175.0,225.0,275.0,325.0,375.0,425.0};
  std::vector<float> h2{8.44,7.38,7.76,8.535,9.47,10.45,10.47,9.71,8.87};
  eCorr_ = new TGraph(em1.size(), em1.data(), em2.data());
  hCorr_ = new TGraph(h1.size(), h1.data(), h2.data());
}

void ParticleFlow::produce(framework::Event& event) {

  if (!event.exists(inputTrackCollName_)) return;
  if (!event.exists(inputEcalCollName_)) return;
  if (!event.exists(inputHcalCollName_)) return;
  const auto ecalClusters = event.getCollection<ldmx::EcalCluster>(inputEcalCollName_);
  const auto hcalClusters = event.getCollection<ldmx::CaloCluster>(inputHcalCollName_);
  // const auto hcalClusters = event.getCollection<ldmx::HcalCluster>(inputHcalCollName_);
  const auto tracks = event.getCollection<ldmx::SimTrackerHit>(inputTrackCollName_);

  //std::cout << ecalClusters.size() << " " << hcalClusters.size() << " " << tracks.size() << std::endl;

  /*
    1. Build links maps at the Tk/Ecal and Hcal/Hcal interfaces
    2. Categorize tracks as: Ecal-matched, (Side) Hcal-matched, unmatched
    3. Categorize Ecal clusters as: Hcal-matched, unmatched
    4a. (Upstream?) Categorize tracks with dedx?
    4b. (Upstream?) Categorize ecal clusters as: EM/Had-like
    4c. (Upstream?) Categorize hcal clusters as: EM/Had-like
    5. Build candidates by category, moving from Tk-Ecal-Hcal
   */

  // track-calo linking
  std::map<int, std::vector<int> > tkCaloMap;
  for(int i=0; i<tracks.size(); i++){
    const auto& tk = tracks[i];
    for(int j=0; j<ecalClusters.size(); j++){
      const auto& ecal = ecalClusters[j];
      // TODO: add the matching logic here...
      bool isMatch = true;
      if(isMatch){
	if (tkCaloMap.count(i)) tkCaloMap[i].push_back(j);
	else tkCaloMap[i] = {j};
      }
    }
  }
  
  // em-hadcalo linking
  std::map<int, std::vector<int> > emHadCaloMap;
  for(int i=0; i<ecalClusters.size(); i++){
    const auto& ecal = ecalClusters[i];
    for(int j=0; j<hcalClusters.size(); j++){
      const auto& hcal = hcalClusters[j];
      // TODO: add the matching logic here...
      bool isMatch = true;
      if(isMatch){
	if (emHadCaloMap.count(i)) emHadCaloMap[i].push_back(j);
	else emHadCaloMap[i] = {j};
      }
    }
  }

  // tk-hadcalo linking (Side HCal)
  std::map<int, std::vector<int> > tkHadCaloMap;
  for(int i=0; i<tracks.size(); i++){
    const auto& tk = tracks[i];
    for(int j=0; j<hcalClusters.size(); j++){
      const auto& hcal = hcalClusters[j];
      // TODO: add the matching logic here...
      bool isMatch = true;
      if(isMatch){
	if (tkHadCaloMap.count(i)) tkHadCaloMap[i].push_back(j);
	else tkHadCaloMap[i] = {j};
      }
    }
  }
  
  std::vector<ldmx::PFCandidate> pfCands;
  // Single-particle builder
  ldmx::PFCandidate pf;
  int pid=0;
  if (tracks.size()){
    const auto& tk = tracks[0];
    // TODO: smear
    std::vector<float> xyz = tk.getPosition();
    std::vector<double> pxyz = tk.getMomentum();
    float ecalZ = 248;
    float ecalX = xyz[0] + pxyz[0]/pxyz[2] * (ecalZ - xyz[2]);
    float ecalY = xyz[1] + pxyz[1]/pxyz[2] * (ecalZ - xyz[2]);    
    pf.setEcalPositionXYZ(ecalX, ecalY, ecalZ);
    pf.setTrackPxPyPz(pxyz[0], pxyz[1], pxyz[2]);
    pid += 1;
    // also use this object to set truth info
    pf.setTruthEcalXYZ(ecalX, ecalY, ecalZ);
    pf.setTruthPxPyPz(pxyz[0], pxyz[1], pxyz[2]);
    float m2 = pow(tk.getEnergy(),2) - pow(pxyz[0],2) - pow(pxyz[1],2) - pow(pxyz[2],2);
    if(m2<0) m2=0;
    pf.setTruthMass(sqrt(m2));
    pf.setTruthEnergy(tk.getEnergy());
    pf.setTruthPdgId(tk.getPdgID());
  }
  if (ecalClusters.size()){
    const auto& em = ecalClusters[0];
    float corr = 1.;
    float e = em.getEnergy();
    if (e<eCorr_->GetX()[0]){
      corr = eCorr_->GetX()[0];
    } else if (e>eCorr_->GetX()[eCorr_->GetN()-1]){
      corr = eCorr_->GetX()[eCorr_->GetN()-1];
    } else {
      corr = eCorr_->Eval(e);
    }
    pf.setEcalEnergy( e*corr );
    pf.setEcalRawEnergy( e );
    pf.setEcalClusterXYZ(em.getCentroidX(),
			 em.getCentroidY(),
			 em.getCentroidZ());
    pf.setEcalClusterEXYZ(em.getRMSX(),
			  em.getRMSY(),
			  em.getRMSZ());
    pf.setEcalClusterDXDZ(em.getDXDZ());
    pf.setEcalClusterDYDZ(em.getDYDZ());
    pf.setEcalClusterEDXDZ(em.getEDXDZ());
    pf.setEcalClusterEDYDZ(em.getEDYDZ());
			 
    pid += 2;
  }
  if (hcalClusters.size()){
    const auto& had = hcalClusters[0];
    float corr = 1.;
    float e = had.getEnergy();
    if (e<hCorr_->GetX()[0]){
      corr = hCorr_->GetX()[0];
    } else if (e>hCorr_->GetX()[hCorr_->GetN()-1]){
      corr = hCorr_->GetX()[hCorr_->GetN()-1];
    } else {
      corr = hCorr_->Eval(e);
    }
    pf.setHcalEnergy( e*corr );
    pf.setHcalRawEnergy( e );
    pf.setHcalClusterXYZ(had.getCentroidX(),
			 had.getCentroidY(),
			 had.getCentroidZ());
    pf.setHcalClusterEXYZ(had.getRMSX(),
			  had.getRMSY(),
			  had.getRMSZ());
    pf.setHcalClusterDXDZ(had.getDXDZ());
    pf.setHcalClusterDYDZ(had.getDYDZ());
    pf.setHcalClusterEDXDZ(had.getEDXDZ());
    pf.setHcalClusterEDYDZ(had.getEDYDZ());
    pid += 4;
  }
  pf.setPID(pid);
  pf.setEnergy( pf.getEcalEnergy() + pf.getHcalEnergy() );
  pfCands.push_back(pf);

  // // Super-simple builder
  // for(int i=0; i<tracks.size(); i++){
  //   const auto& tk = tracks[i];
  //   ldmx::PFCandidate pf;
    
  //   if (emHadCaloMap.count(i)){ // tk matches em
  //     int iem = emHadCaloMap[i];
  //     const auto& em = ecalClusters[iem];
      
  //     if (emHadCaloMap.count(iem)){ // tk matches em
  // 	int ihad = emHadCaloMap[iem];
  // 	const auto& had = hcalClusters[ihad];
	
  //     }
  //   }
  // }

  event.add(outputCollName_, pfCands);
}

void ParticleFlow::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void ParticleFlow::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void ParticleFlow::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void ParticleFlow::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, ParticleFlow);
