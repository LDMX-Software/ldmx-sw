#include "Tracking/Reco/DigitizationProcessor.h"

#include <chrono>

#include "Tracking/Event/Measurement.h"
#include "Tracking/Sim/TrackingUtils.h"

using namespace framework;

namespace tracking::reco {

DigitizationProcessor::DigitizationProcessor(const std::string& name,
                                             framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void DigitizationProcessor::onProcessStart() {
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);

  std::cout << "Loading the tracking geometry" << std::endl;

  // Module Bounds => Take them from the tracking geometry TODO
  auto moduleBounds = std::make_shared<const Acts::RectangleBounds>(
      20.17 * Acts::UnitConstants::mm, 50 * Acts::UnitConstants::mm);

  // I assume 5 APVs
  int nbinsx = 128 * 5;

  // Strips
  int nbinsy = 1;

  // Thickness = 0.320 mm
  double thickness = 0.320 * Acts::UnitConstants::mm;

  // Lorentz angle
  double lAngle = 0.01;

  // Energy threshold
  double eThresh = 0.;

  // Analogue readout
  bool isAnalog = true;

  // Cartesian segmentation
  auto cSegmentation = std::make_shared<const Acts::CartesianSegmentation>(
      moduleBounds, nbinsx, nbinsy);

  // Negative side readout => TODO Make sure this is correct!
  //  - Ask Paul what does this mean: depending on how local w is oriented
  // TODO: load proper lorentz angle

  Acts::DigitizationModule ndModule(cSegmentation, thickness * 0.5, -1, lAngle,
                                    eThresh, isAnalog);

  std::cout << getName() << " Initialization done" << std::endl;

  // Seed the generator
  generator_.seed(1);
}

void DigitizationProcessor::configure(
    framework::config::Parameters& parameters) {
  detector_ = parameters.getParameter<std::string>("detector");
  hit_collection_ =
      parameters.getParameter<std::string>("hit_collection", "TaggerSimHits");
  out_collection_ = parameters.getParameter<std::string>("out_collection",
                                                         "OutputMeasuements");
  min_e_dep_ = parameters.getParameter<double>("min_e_dep", 0.05);
  track_id_ = parameters.getParameter<int>("track_id", -1);
  do_smearing_ = parameters.getParameter<bool>("do_smearing", true);
  sigma_u_ = parameters.getParameter<double>("sigma_u", 0.01);
  sigma_v_ = parameters.getParameter<double>("sigma_v", 0.);
  merge_hits_ = parameters.getParameter<bool>("merge_hits", false);
}

void DigitizationProcessor::produce(framework::Event& event) {

  ldmx_log(debug) << " Getting the tracking geometry:" << geometry().getTG();

  // Mode 0: Load simulated hits and produce smeared 1d measurements
  // Mode 1: Load simulated hits and produce digitized 1d measurements

  const std::vector<ldmx::SimTrackerHit> sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(hit_collection_);

  std::vector<ldmx::SimTrackerHit> merged_hits;

  std::vector<ldmx::Measurement> measurements;
  if (merge_hits_) {
    mergeSimHits(sim_hits, merged_hits);
    measurements = digitizeHits(merged_hits);
  }

  else {
    measurements = digitizeHits(sim_hits);
  }

  event.add(out_collection_, measurements);
}

// This method merges hits that have the same track_id on the same layer.
// The energy of the merged hit is the sum of the energy of the single sub-hits
// The position/momentum of the merged hit is the energy-weighted average
// sihits = vector of hits to merge
// mergedHits = total merged collection

bool DigitizationProcessor::mergeHits(
    const std::vector<ldmx::SimTrackerHit>& sihits,
    std::vector<ldmx::SimTrackerHit>& mergedHits) {
  if (sihits.size() < 1) return false;

  if (sihits.size() == 1) {
    mergedHits.push_back(sihits[0]);
    return true;
  }

  ldmx::SimTrackerHit mergedHit;
  // Since all the hits will be on the same sensor, just use the ID of the first
  mergedHit.setLayerID(sihits[0].getLayerID());
  mergedHit.setModuleID(sihits[0].getModuleID());
  mergedHit.setID(sihits[0].getID());
  mergedHit.setTrackID(sihits[0].getTrackID());

  double X{0}, Y{0}, Z{0}, PX{0}, PY{0}, PZ{0};
  double T{0}, E{0}, EDEP{0}, path{0};
  int pdgID{0};

  pdgID = sihits[0].getPdgID();

  for (auto hit : sihits) {
    double edep_hit = hit.getEdep();
    EDEP += edep_hit;
    E += hit.getEnergy();
    T += edep_hit * hit.getTime();
    X += edep_hit * hit.getPosition()[0];
    Y += edep_hit * hit.getPosition()[1];
    Z += edep_hit * hit.getPosition()[2];
    PX += edep_hit * hit.getMomentum()[0];
    PY += edep_hit * hit.getMomentum()[1];
    PZ += edep_hit * hit.getMomentum()[2];
    path += edep_hit * hit.getPathLength();

    if (hit.getPdgID() != pdgID) {
      std::cout << "ERROR:: Found hits with compatible sensorID and track_id "
                   "but different PDGID"
                << std::endl;
      std::cout << "TRACKID ==" << hit.getTrackID() << " vs "
                << sihits[0].getTrackID() << std::endl;
      std::cout << "PDGID== " << hit.getPdgID() << " vs " << pdgID << std::endl;
      return false;
    }
  }

  mergedHit.setTime(T / EDEP);
  mergedHit.setPosition(X / EDEP, Y / EDEP, Z / EDEP);
  mergedHit.setMomentum(PX / EDEP, PY / EDEP, PZ / EDEP);
  mergedHit.setPathLength(path / EDEP);
  mergedHit.setEnergy(E);
  mergedHit.setEdep(EDEP);
  mergedHit.setPdgID(pdgID);

  mergedHits.push_back(mergedHit);

  return true;
}

// TODO avoid copies and use references
bool DigitizationProcessor::mergeSimHits(
    const std::vector<ldmx::SimTrackerHit>& sim_hits,
    std::vector<ldmx::SimTrackerHit>& merged_hits) {
  // The first key is the index of the sensitive element ID, second key is the
  // track_id
  std::map<int, std::map<int, std::vector<ldmx::SimTrackerHit>>> hitmap;

  for (auto hit : sim_hits) {
    unsigned int index = tracking::sim::utils::getSensorID(hit);
    unsigned int trackid = hit.getTrackID();
    hitmap[index][trackid].push_back(hit);

    ldmx_log(debug) << "hitmap being filled, size::[" << index << "]["
                    << trackid << "] size " << hitmap[index][trackid].size();
  }

  typedef std::map<int,
                   std::map<int, std::vector<ldmx::SimTrackerHit>>>::iterator
      hitmap_it1;
  typedef std::map<int, std::vector<ldmx::SimTrackerHit>>::iterator hitmap_it2;
  for (hitmap_it1 it = hitmap.begin(); it != hitmap.end(); it++) {
    for (hitmap_it2 it2 = it->second.begin(); it2 != it->second.end(); it2++) {
      mergeHits(it2->second, merged_hits);
    }
  }

  ldmx_log(debug) << "Sim_hits Size=" << sim_hits.size()
                  << "Merged_hits Size=" << merged_hits.size();

  // for (auto hit : sim_hits) hit.Print();
  // for (auto mhit : merged_hits) mhit.Print();

  return true;
}

std::vector<ldmx::Measurement> DigitizationProcessor::digitizeHits(
    const std::vector<ldmx::SimTrackerHit>& sim_hits) {
  ldmx_log(debug) << "Found:" << sim_hits.size() << " sim hits in the "
                  << hit_collection_;

  std::vector<ldmx::Measurement> measurements;

  // Loop over all SimTrackerHits and
  // * Use the position of the SimTrackerHit (global position) and the surface
  //   the hit was created on to extract the local coordinates.
  // * If specified, smear the local coordinates and update the global
  //   coordinates.
  // * Create a Measurement object.
  for (auto& sim_hit : sim_hits) {
    // Remove low energy deposit hits
    if (sim_hit.getEdep() > min_e_dep_) {
      if (track_id_ > 0 && sim_hit.getTrackID() != track_id_) continue;

      ldmx::Measurement measurement(sim_hit);
      
      // Get the layer ID.
      auto layer_id = tracking::sim::utils::getSensorID(sim_hit);
      measurement.setLayerID(layer_id);

      
      
      // Get the surface
      auto hit_surface{geometry().getSurface(layer_id)};

      if (hit_surface) {
        // Transform from global to local coordinates.
        // hit_surface->toStream(geometry_context(), std::cout);
        ldmx_log(debug) << "Local to global" << std::endl
                        << hit_surface->transform(geometry_context()).rotation() << std::endl
                        << hit_surface->transform(geometry_context()).translation();

        Acts::Vector3 dummy_momentum;
        Acts::Vector2 local_pos;
        double surface_thickness = 0.320 * Acts::UnitConstants::mm;
        Acts::Vector3 global_pos(measurement.getGlobalPosition()[0],
                                 measurement.getGlobalPosition()[1],
                                 measurement.getGlobalPosition()[2]);

        try {
          local_pos = hit_surface
                          ->globalToLocal(geometry_context(), global_pos, dummy_momentum,
                                          surface_thickness)
                          .value();
        } catch (const std::exception& e) {
          std::cout << "WARNING:: hit not on surface.. Skipping." << std::endl;
          continue;
        }

        // Smear the local position
        if (do_smearing_) {
          float smear_factor{(*normal_)(generator_)};

          local_pos[0] += smear_factor * sigma_u_;
          smear_factor = (*normal_)(generator_);
          local_pos[1] += smear_factor * sigma_v_;

          // update covariance
          measurement.setLocalCovariance(sigma_u_ * sigma_u_,
                                         sigma_v_ * sigma_v_);

          // transform to global
          auto global_pos{
              hit_surface->localToGlobal(geometry_context(), local_pos, dummy_momentum)};
          measurement.setGlobalPosition(measurement.getGlobalPosition()[0],
                                        global_pos(1), global_pos(2));

        }  // do smearing
        measurement.setLocalPosition(local_pos(0), local_pos(1));
        measurements.push_back(measurement);
      }  // hit_surface exists
      
    }    // energy cut
  }      // loop on sim-hits

  return measurements;

}  // digitizeHits
}  // namespace tracking::reco

DECLARE_PRODUCER_NS(tracking::reco, DigitizationProcessor)
