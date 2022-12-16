#include "Tracking/Reco/DigitizationProcessor.h"

#include <chrono>

#include "Tracking/Event/Measurement.h"
#include "Tracking/Sim/TrackingUtils.h"

using namespace framework;

namespace tracking {
namespace reco {

DigitizationProcessor::DigitizationProcessor(const std::string& name,
                                             framework::Process& process)
    : framework::Producer(name, process) {}

DigitizationProcessor::~DigitizationProcessor() {}

void DigitizationProcessor::onProcessStart() {
  gctx_ = Acts::GeometryContext();
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);

  std::cout << "Loading the tracking geometry" << std::endl;

  // Load the tracking geometry
  ldmx_tg = std::make_shared<tracking::reco::TrackersTrackingGeometry>(
      "/Users/pbutti/sw/ldmx-sw/Detectors/data/ldmx-det-v12/detector.gdml",
      &gctx_, debug_);

  TestTracker tt("/Users/pbutti/sw/ldmx-sw/digi/testPlanes.gdml", &gctx_, true);
  

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
  hit_collection_ =
      parameters.getParameter<std::string>("hit_collection", "TaggerSimHits");
  out_collection_ = parameters.getParameter<std::string>("out_collection",
                                                         "OutputMeasuements");
  minEdep_ = parameters.getParameter<double>("minEdep", 0.05);
  trackID_ = parameters.getParameter<int>("trackID", -1);
  do_smearing_ = parameters.getParameter<bool>("do_smearing", true);
  sigma_u_ = parameters.getParameter<double>("sigma_u", 0.01);
  sigma_v_ = parameters.getParameter<double>("sigma_v", 0.);
  debug_ = parameters.getParameter<bool>("debug", false);
  mergeHits_ = parameters.getParameter<bool>("mergeHits", false);

  std::cout << "hit_collection_::" << hit_collection_ << std::endl;
  std::cout << "out_collection_::" << out_collection_ << std::endl;
  std::cout << "minEdep_::" << minEdep_ << std::endl;
}

void DigitizationProcessor::produce(framework::Event& event) {
  // Get the tracking geometry

  if (debug_)
    std::cout << getName() << " getting the tracking geometry" << std::endl;

  const auto tGeometry = ldmx_tg->getTG();

  // Mode 0: Load simulated hits and produce smeared 1d measurements
  // Mode 1: Load simulated hits and produce digitized 1d measurements

  std::vector<ldmx::LdmxSpacePoint*> digitized_hits;

  if (debug_) std::cout << "getting the sim hits" << std::endl;

  const std::vector<ldmx::SimTrackerHit> sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(hit_collection_);

  if (debug_) {
    std::cout << getName() << " running merge" << std::endl;
  }

  std::vector<ldmx::SimTrackerHit> merged_hits;

  if (mergeHits_) {
    mergeSimHits(sim_hits, merged_hits);
    digitizeHits(merged_hits, digitized_hits);
  }

  else {
    digitizeHits(sim_hits, digitized_hits);
  }

  // Get the measurements and run clustering

  std::vector<ldmx::Measurement> output_measurements;

  for (auto dh : digitized_hits) {
    ldmx::Measurement m;
    m.setGlobalPosition(dh->global_pos_(0), dh->global_pos_(1),
                        dh->global_pos_(2));
    m.setLocalPosition(dh->local_pos_(0), dh->local_pos_(1));
    m.setTime(-999);
    m.setLayer(dh->layer());

    output_measurements.push_back(m);
  }
  if (debug_)
    std::cout << "Output measurements " << output_measurements.size()
              << std::endl;

  event.add(out_collection_, output_measurements);
}

void DigitizationProcessor::onProcessEnd() {}
// This method merges hits that have the same trackID on the same layer.
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
      std::cout << "ERROR:: Found hits with compatible sensorID and trackID "
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
  // trackID
  std::map<int, std::map<int, std::vector<ldmx::SimTrackerHit>>> hitmap;

  for (auto hit : sim_hits) {
    unsigned int index = tracking::sim::utils::getSensorID(hit);
    unsigned int trackid = hit.getTrackID();
    hitmap[index][trackid].push_back(hit);
    std::cout << "hitmap being filled, size::[" << index << "][" << trackid
              << "] size " << hitmap[index][trackid].size() << std::endl;
  }

  typedef std::map<int,
                   std::map<int, std::vector<ldmx::SimTrackerHit>>>::iterator
      hitmap_it1;
  typedef std::map<int, std::vector<ldmx::SimTrackerHit>>::iterator hitmap_it2;
  for (hitmap_it1 it = hitmap.begin(); it != hitmap.end(); it++) {
    for (hitmap_it2 it2 = it->second.begin(); it2 != it->second.end(); it2++) {
      if (debug_) std::cout << "Merging hits in the hitmap" << std::endl;
      mergeHits(it2->second, merged_hits);
    }
  }

  if (debug_) {
    std::cout << "######### CHECKING MERGING RESULTS #########" << std::endl;
    std::cout << "Sim_hits Size=" << sim_hits.size() << std::endl;
    std::cout << "Merged_hits Size=" << merged_hits.size() << std::endl;

    for (auto hit : sim_hits) hit.Print();

    for (auto mhit : merged_hits) mhit.Print();
  }

  return true;
}

void DigitizationProcessor::digitizeHits(
    const std::vector<ldmx::SimTrackerHit>& sim_hits,
    std::vector<ldmx::LdmxSpacePoint*>& ldmxsps) {
  if (debug_)
    std::cout << "Found:" << sim_hits.size() << " sim hits in the "
              << hit_collection_ << std::endl;

  // Convert to ldmxsps

  for (auto& simHit : sim_hits) {
    // Remove low energy deposit hits
    if (simHit.getEdep() > minEdep_) {
      if (trackID_ > 0 && simHit.getTrackID() != trackID_) continue;

      ldmx::LdmxSpacePoint* ldmxsp =
          tracking::sim::utils::convertSimHitToLdmxSpacePoint(simHit);

      // Get the layer from the ldmxsp
      unsigned int layerid = ldmxsp->layer();

      // Get the surface
      const Acts::Surface* hit_surface = ldmx_tg->getSurface(layerid);

      if (hit_surface) {
        // Transform the ldmx space point from global to local and store the
        // information

        if (debug_) {
          std::cout << "Global hit position on layer::" << ldmxsp->layer()
                    << std::endl;
          std::cout << ldmxsp->global_pos_ << std::endl;
          hit_surface->toStream(gctx_, std::cout);
          std::cout << std::endl;
          std::cout << "TRANSFORM LOCAL TO GLOBAL" << std::endl;
          std::cout << hit_surface->transform(gctx_).rotation() << std::endl;
          std::cout << hit_surface->transform(gctx_).translation() << std::endl;
        }

        Acts::Vector3 dummy_momentum;
        Acts::Vector2 local_pos;
        double surface_thickness = 0.320 * Acts::UnitConstants::mm;

        try {
          local_pos = hit_surface
                          ->globalToLocal(gctx_, ldmxsp->global_pos_,
                                          dummy_momentum, surface_thickness)
                          .value();
        } catch (const std::exception& e) {
          std::cout << "WARNING:: hit not on surface.. Skipping." << std::endl;
          std::cout << ldmxsp->global_pos_ << std::endl;
          continue;
        }

        // Smear the local position

        if (debug_) std::cout << getName() << "  Doing smearing" << std::endl;

        if (do_smearing_) {
          float smear_factor{(*normal_)(generator_)};

          local_pos[0] += smear_factor * sigma_u_;
          smear_factor = (*normal_)(generator_);
          local_pos[1] += smear_factor * sigma_v_;

          if (debug_) std::cout << "update covariance" << std::endl;
          // update covariance
          ldmxsp->setLocalCovariance(sigma_u_ * sigma_u_, sigma_v_ * sigma_v_);

          // cache the acts x coordinate
          double original_x = ldmxsp->global_pos_(0);

          // transform to global
          ldmxsp->global_pos_ =
              hit_surface->localToGlobal(gctx_, local_pos, dummy_momentum);
          // update the acts x location
          ldmxsp->global_pos_(0) = original_x;

        }  // do smearing

        if (debug_) std::cout << "done smearing" << std::endl;

        ldmxsp->local_pos_ = local_pos;

        ldmxsps.push_back(ldmxsp);
      }  // hit_surface exists
    }    // energy cut

  }  // loop on sim-hits
}  // digitizeHits
}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, DigitizationProcessor)
