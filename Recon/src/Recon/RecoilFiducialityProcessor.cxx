/**
 * @file RecoilFiducialityProcessor.cxx
 * @brief Processor used to select events where the recoil electron is fiducial.
 * @author Elizabeth Berzin, Stanford University
 */

 #include "Recon/RecoilFiducialityProcessor.h"

 namespace recon {

void RecoilFiducialityProcessor::configure(framework::config::Parameters& parameters) {
    min_p_mag_ = parameters.getParameter<double>("min_p_mag");
    min_tracker_hits_ = parameters.getParameter<int>("min_tracker_hits");
    ecal_collection_ = parameters.getParameter<std::string>("ecal_collection");
    hcal_collection_ = parameters.getParameter<std::string>("hcal_collection");
    recoil_collection_ = parameters.getParameter<std::string>("recoil_collection");
    output_collection_ = parameters.getParameter<std::string>("output_collection");

}
 
 void RecoilFiducialityProcessor::produce(framework::Event &event) {
   // Get the collection of simulated particles from the event
   auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};
 
   // Search for the recoil electron
   auto [recoilTrackID, recoilElectron] = Analysis::getRecoil(particleMap);
 
   // Get the collection of simulated Ecal hits from the event.
   const std::vector<ldmx::SimCalorimeterHit> ecalSimHits =
       event.getCollection<ldmx::SimCalorimeterHit>(ecal_collection_);

   // Get the collection of simulated Ecal hits from the event.
   const std::vector<ldmx::SimCalorimeterHit> hcalSimHits =
       event.getCollection<ldmx::SimCalorimeterHit>(hcal_collection_);

   // Get the collection of simulated tracker hits from the event.
   const std::vector<ldmx::SimTrackerHit> recoilSimHits =
       event.getCollection<ldmx::SimTrackerHit>(recoil_collection_);
 
   // Loop through the Ecal hits and check if the recoil electron is
   // associated with any of them.  
   bool hasEcalHit = false;
   int ecalHitID = -1;
   for (const ldmx::SimCalorimeterHit &simHit : ecalSimHits) {
     for (int iContrib = 0; iContrib < simHit.getNumberOfContribs();
          ++iContrib) {
       ldmx::SimCalorimeterHit::Contrib contrib = simHit.getContrib(iContrib);
 
       if (contrib.trackID == recoilTrackID) {
          hasEcalHit = true;
          ecalHitID = simHit.getID();
       }
     }
   }

   // Loop through the Hcal hits and check if the recoil electron is
   // associated with any of them.  
   bool hasHcalHit = false;
   int hcalHitID = -1;
   for (const ldmx::SimCalorimeterHit &simHit : hcalSimHits) {
     for (int iContrib = 0; iContrib < simHit.getNumberOfContribs();
          ++iContrib) {
       ldmx::SimCalorimeterHit::Contrib contrib = simHit.getContrib(iContrib);
 
       if (contrib.trackID == recoilTrackID) {
          hasHcalHit = true;
          hcalHitID = simHit.getID();
       }
     }
   }

   
   // Loop through the recoil tracker hits and count how many
   // the recoil electron is associated with
   std::set<int> layers_hit;
   for (const ldmx::SimTrackerHit &simHit : recoilSimHits) {
        if (simHit.getTrackID() == recoilTrackID) {
            //int sensorID = tracking::sim::utils::getSensorID(sim_hit);
            if ((simHit.getTime() < 0.8) & (simHit.getMomentum()[2] > 0)) {layers_hit.insert(simHit.getLayerID());}
        }
   }
   bool hasMinTrackerHits = false;
   if (layers_hit.size() >= min_tracker_hits_) {hasMinTrackerHits = true;}

   // Checking if the recoil electron energy is > min energy
   bool hasMinEnergy = false;
   if (recoilElectron->getEnergy() >= min_p_mag_) {hasMinEnergy = true;}


   // Configure outputs
   bool isFiducialFlag = hasMinEnergy & hasMinTrackerHits & hasEcalHit;
   
   int maskTrackerE = hasMinEnergy << 0; 
   int maskTrackerhits = hasMinTrackerHits << 1; 
   int maskEcal = hasEcalHit << 2; 
   int maskHcal = hasHcalHit << 3; 
   int fiducialFlag = maskTrackerE | maskTrackerhits | maskEcal | maskHcal;

   ldmx::FiducialFlag flag;
   flag.setFiducialFlag(fiducialFlag, 6);
   flag.setAlgoVar(0, recoilElectron->getEnergy());
   flag.setAlgoVar(1, min_p_mag_);
   flag.setAlgoVar(2, layers_hit.size());
   flag.setAlgoVar(3, min_tracker_hits_);
   flag.setAlgoVar(4, ecalHitID);
   flag.setAlgoVar(5, hcalHitID);

   flag.setIsFiducial(isFiducialFlag);
   flag.setHasMinEnergy(hasMinEnergy);
   flag.setHasMinTrackerHits(hasMinTrackerHits);
   flag.setHasEcalHit(hasEcalHit);
   flag.setHasHcalHit(hasHcalHit);

   event.add(output_collection_, flag);
   
   // Tell the skimmer to keep or drop the event based on whether there
   // were recoil electron was fiducial.
   if (isFiducialFlag) {
     setStorageHint(framework::hint_shouldDrop);
   } else {
     setStorageHint(framework::hint_shouldKeep);
   }
 }
 }  // namespace recon
 
 DECLARE_PRODUCER_NS(recon, RecoilFiducialityProcessor);
 