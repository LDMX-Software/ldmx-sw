#include "Recon/ParticleFlow.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Ecal/Event/EcalCluster.h"
#include "Hcal/Event/HcalCluster.h"
#include "Recon/Event/PFCandidate.h"


namespace recon {

void ParticleFlow::configure(framework::config::Parameters& ps) {
  inputEcalCollName_ = ps.getParameter<std::string>("inputEcalCollName");
  inputHcalCollName_ = ps.getParameter<std::string>("inputHcalCollName");
  inputTrackCollName_ = ps.getParameter<std::string>("inputTrackCollName");
  outputCollName_ = ps.getParameter<std::string>("outputCollName"); 
}

void ParticleFlow::produce(framework::Event& event) {

  if (!event.exists(inputTrackCollName_)) return;
  if (!event.exists(inputEcalCollName_)) return;
  if (!event.exists(inputHcalCollName_)) return;
  const auto ecalClusters = event.getCollection<ldmx::EcalCluster>(inputEcalCollName_);
  const auto hcalClusters = event.getCollection<ldmx::HcalCluster>(inputHcalCollName_);
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
    pf.setEcalEnergy( em.getEnergy() );
    pf.setEcalClusterXYZ(em.getCentroidX(),
			 em.getCentroidY(),
			 em.getCentroidZ());
			 
    pid += 2;
  }
  if (hcalClusters.size()){
    const auto& had = hcalClusters[0];
    pf.setHcalEnergy( had.getEnergy() );
    pf.setHcalClusterXYZ(had.getCentroidX(),
			 had.getCentroidY(),
			 had.getCentroidZ());
			 
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
