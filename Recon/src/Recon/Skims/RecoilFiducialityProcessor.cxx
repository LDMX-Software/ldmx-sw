/**
 * @file RecoilFiducialityProcessor.cxx
 * @brief Processor used to select events where the recoil electron is fiducial.
 * @author Elizabeth Berzin, Stanford University
 */

#include "Recon/Skims/RecoilFiducialityProcessor.h"

namespace recon {

void RecoilFiducialityProcessor::configure(
    framework::config::Parameters &parameters) {
  min_p_mag_ = parameters.getParameter<double>("min_p_mag");
  min_tracker_hits_ = parameters.getParameter<int>("min_tracker_hits");
  ecal_collection_ = parameters.getParameter<std::string>("ecal_collection");
  hcal_collection_ = parameters.getParameter<std::string>("hcal_collection");
  recoil_collection_ =
      parameters.getParameter<std::string>("recoil_collection");
  output_collection_ =
      parameters.getParameter<std::string>("output_collection");
  inverse_skim_ = parameters.getParameter<bool>("inverse_skim");
}

void RecoilFiducialityProcessor::produce(framework::Event &event) {
  // Get the collection of simulated particles from the event
  auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};

  // Search for the recoil electron
  auto [recoil_track_id, recoil_electron] = Analysis::getRecoil(particleMap);

  // Get the collection of simulated Ecal hits from the event.
  const std::vector<ldmx::SimCalorimeterHit> ecal_sim_hits =
      event.getCollection<ldmx::SimCalorimeterHit>(ecal_collection_);

  // Get the collection of simulated Ecal hits from the event.
  const std::vector<ldmx::SimCalorimeterHit> hcal_sim_hits =
      event.getCollection<ldmx::SimCalorimeterHit>(hcal_collection_);

  // Get the collection of simulated tracker hits from the event.
  const std::vector<ldmx::SimTrackerHit> recoil_sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(recoil_collection_);

  // Loop through the Ecal hits and check if the recoil electron is
  // associated with any of them.
  bool has_ecal_hit = false;
  int ecal_hit_id = -1;
  for (const ldmx::SimCalorimeterHit &sim_hit : ecal_sim_hits) {
    for (int iContrib = 0; iContrib < sim_hit.getNumberOfContribs();
         ++iContrib) {
      ldmx::SimCalorimeterHit::Contrib contrib = sim_hit.getContrib(iContrib);

      if (contrib.trackID == recoil_track_id) {
        has_ecal_hit = true;
        ecal_hit_id = sim_hit.getID();
      }
    }
  }

  // Loop through the Hcal hits and check if the recoil electron is
  // associated with any of them.
  bool has_hcal_hit = false;
  int hcal_hit_id = -1;
  for (const ldmx::SimCalorimeterHit &sim_hit : hcal_sim_hits) {
    for (int iContrib = 0; iContrib < sim_hit.getNumberOfContribs();
         ++iContrib) {
      ldmx::SimCalorimeterHit::Contrib contrib = sim_hit.getContrib(iContrib);

      if (contrib.trackID == recoil_track_id) {
        has_hcal_hit = true;
        hcal_hit_id = sim_hit.getID();
      }
    }
  }

  // Loop through the recoil tracker hits and count how many
  // the recoil electron is associated with
  std::set<int> layers_hit;
  for (const ldmx::SimTrackerHit &sim_hit : recoil_sim_hits) {
    if (sim_hit.getTrackID() == recoil_track_id) {
      // int sensorID = tracking::sim::utils::getSensorID(sim_hit);
      if ((sim_hit.getTime() < 0.8) && (sim_hit.getMomentum()[2] > 0)) {
        layers_hit.insert(sim_hit.getLayerID());
      }
    }
  }
  bool has_min_tracker_hits = false;
  if (layers_hit.size() >= min_tracker_hits_) {
    has_min_tracker_hits = true;
  }

  // Checking if the recoil electron energy is > min energy
  bool has_min_energy = false;
  if (recoil_electron->getEnergy() >= min_p_mag_) {
    has_min_energy = true;
  }

  // Configure outputs
  bool is_fiducial = has_min_energy & has_min_tracker_hits & has_ecal_hit;

  int mask_tracker_E = has_min_energy << 0;
  int mask_tracker_hits = has_min_tracker_hits << 1;
  int mask_ecal = has_ecal_hit << 2;
  int mask_hcal = has_hcal_hit << 3;
  int fiducial_flag =
      mask_tracker_E | mask_tracker_hits | mask_ecal | mask_hcal;

  ldmx::FiducialFlag flag;
  flag.setFiducialFlag(fiducial_flag, 6);
  flag.setAlgoVar(0, recoil_electron->getEnergy());
  flag.setAlgoVar(1, min_p_mag_);
  flag.setAlgoVar(2, layers_hit.size());
  flag.setAlgoVar(3, min_tracker_hits_);
  flag.setAlgoVar(4, ecal_hit_id);
  flag.setAlgoVar(5, hcal_hit_id);

  flag.setIsFiducial(is_fiducial);
  flag.setHasMinEnergy(has_min_energy);
  flag.setHasMinTrackerHits(has_min_tracker_hits);
  flag.setHasEcalHit(has_ecal_hit);
  flag.setHasHcalHit(has_hcal_hit);

  event.add(output_collection_, flag);

  // Tell the skimmer to keep or drop the event based on whether there
  // were recoil electron was fiducial.

  if (!inverse_skim_) {
    if (is_fiducial) {
      setStorageHint(framework::hint_shouldKeep);
    } else {
      setStorageHint(framework::hint_shouldDrop);
    }
  } else {
    if (is_fiducial) {
      setStorageHint(framework::hint_shouldDrop);
    } else {
      setStorageHint(framework::hint_shouldKeep);
    }
  }
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, RecoilFiducialityProcessor);
