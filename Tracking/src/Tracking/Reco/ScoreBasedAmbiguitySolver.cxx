#include "Tracking/Reco/ScoreBasedAmbiguitySolver.h"

#include <algorithm>

#include "Acts/EventData/TrackHelpers.hpp"
#include "Acts/EventData/SourceLink.hpp"

namespace tracking {
namespace reco {

ScoreBasedAmbiguitySolver::ScoreBasedAmbiguitySolver(const std::string& name, framework::Process& process)
    : TrackingGeometryUser(name, process) {}

ScoreBasedAmbiguitySolver::~ScoreBasedAmbiguitySolver() {}


template <typename source_link_hash_t, typename source_link_equality_t, typename geometry_t>
std::vector<std::vector<ScoreBasedAmbiguitySolver::MeasurementInfo>> ScoreBasedAmbiguitySolver::computeInitialState(
      std::vector<ldmx::Track> tracks,  std::vector<ldmx::Measurement> meas_coll,
      source_link_hash_t sourceLinkHash,
      source_link_equality_t sourceLinkEquality, geometry_t& tg, 
      std::vector<std::vector<ScoreBasedAmbiguitySolver::TrackFeatures>>& trackFeaturesVectors) const{

  auto MeasurementIndexMap =
      std::unordered_map<Acts::SourceLink, std::size_t, source_link_hash_t,
                         source_link_equality_t>(0, sourceLinkHash,
                                                 sourceLinkEquality);

  std::vector<std::vector<ScoreBasedAmbiguitySolver::MeasurementInfo>> measurementsPerTrack;
  boost::container::flat_map<std::size_t, boost::container::flat_set<std::size_t>> tracksPerMeasurement;  
  measurementsPerTrack.reserve(tracks.size());
  std::vector<std::vector<std::size_t>> measurementsPerTrack_idxs;
  //Acts::ACTS_VERBOSE("Starting to compute initial state");

  for (const auto& track : tracks) {
    int numberOfDetectors = detectorConfigs_.size();
    int numberOfTrackStates = track.getNhits();
    std::vector<MeasurementInfo> measurements;
    std::vector<std::size_t> measurements_idxs;
    measurements.reserve(numberOfTrackStates);
    std::vector<ScoreBasedAmbiguitySolver::TrackFeatures> trackFeaturesVector(numberOfDetectors);

    for (auto imeas : track.getMeasurementsIdxs()) {
        auto meas = meas_coll.at(imeas);
        const Acts::Surface* hit_surface = tg.getSurface(meas.getLayerID());
      
   
        auto iVolume = hit_surface->geometryId().volume(); 
        auto volume_it = volumeMap_.find(iVolume);
        if (volume_it == volumeMap_.end()) {
      //  ACTS_ERROR("Volume " << iVolume << "not found in the volume map");
        continue;
      }
      auto detectorId = volume_it->second;
      auto outliers = track.getOutlierIdxs();
      auto holes = track.getHoleIdxs();
      auto sharedHoles = track.getSharedIdxs();

      if (std::find(holes.begin(), holes.end(), imeas) != holes.end()) {
            trackFeaturesVector[detectorId].nHoles++;
      } else if (std::find(outliers.begin(), outliers.end(), imeas) != outliers.end()) {
        ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
        Acts::SourceLink sourceLink = Acts::SourceLink(idx_sl);
        trackFeaturesVector[detectorId].nOutliers++;

         auto emplace = MeasurementIndexMap.try_emplace(
            sourceLink, MeasurementIndexMap.size());

        bool isOutlier = true;

         measurements.push_back({emplace.first->second, detectorId, isOutlier});
      } else {

        if (std::find(sharedHoles.begin(), sharedHoles.end(), imeas) != sharedHoles.end()) {
          trackFeaturesVector[detectorId].nSharedHits++;
        }
        ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
        Acts::SourceLink sourceLink = Acts::SourceLink(idx_sl);

         trackFeaturesVector[detectorId].nHits++;

         auto emplace = MeasurementIndexMap.try_emplace(
            sourceLink, MeasurementIndexMap.size());

        bool isOutlier = false;

        measurements.push_back({emplace.first->second, detectorId, isOutlier});
      }
    }
       measurementsPerTrack.push_back(std::move(measurements));
       trackFeaturesVectors.push_back(std::move(trackFeaturesVector));
    }

  return measurementsPerTrack;
}

std::vector<double> ScoreBasedAmbiguitySolver::simpleScore(
      const std::vector<ldmx::Track> tracks,
      const std::vector<std::vector<TrackFeatures>>& trackFeaturesVectors) const {

  std::vector<double> trackScore;
  trackScore.reserve(tracks.size());

  int iTrack = 0;

  
  ldmx_log(debug) << "Number of detectors: " << detectorConfigs_.size();


  ldmx_log(debug) << "Starting to score tracks";

  // Loop over all the tracks in the container
  for (const auto& track : tracks) {
    // get the trackFeatures map for the track
    const auto& trackFeaturesVector = trackFeaturesVectors[iTrack];
    double score = 1;
    auto track_mom = track.getMomentum();
    auto pT = std::sqrt(track_mom[1] * track_mom[1] + track_mom[2] * track_mom[2]);
    auto eta = std::asinh(track_mom[0] / pT);
    auto phi = std::atan2(track_mom[2], track_mom[1]);
    // cuts on pT
    if (pT < pTMin_ || pT > pTMax_) {
      score = 0;
      iTrack++;
      trackScore.push_back(score);
      ldmx_log(debug) << "Track: " << iTrack
                         <<   " has score = 0, due to pT cuts --- pT = " << pT;
      continue;
    }

    // cuts on phi
    if (phi > phiMax_ || phi < phiMin_) {
      score = 0;
      iTrack++;
      trackScore.push_back(score);
     ldmx_log(debug) << "Track: " << iTrack
                           << " has score = 0, due to phi cuts --- phi =  "
                           << phi;
      continue;
    }

    // cuts on eta
    if (eta > etaMax_ || eta < etaMin_) {
      score = 0;
      iTrack++;
      trackScore.push_back(score);
      ldmx_log(debug) << "Track: " << iTrack
                           << " has score = 0, due to eta cuts --- eta =  "
                          << eta;
      continue;
    }

    /*
    // cuts on optional cuts
    for (const auto& cutFunction : optionalCuts.cuts) {
      if (cutFunction(track)) {
        score = 0;
        //Acts::ACTS_DEBUG("Track: " << iTrack
        //                     << " has score = 0, due to optional cuts.");
        break;
      }
    }
    */

    if (score == 0) {
      iTrack++;
      trackScore.push_back(score);
      ldmx_log(debug) << "Track: " << iTrack << " score : " << score;
      continue;
    }
    // Reject tracks which didn't pass the detector cuts.
    for (std::size_t detectorId = 0; detectorId < detectorConfigs_.size();
         detectorId++) {
      const auto& detector = detectorConfigs_.at(detectorId);

      const auto& trackFeatures = trackFeaturesVector[detectorId];

      ldmx_log(debug) << "---> Found summary information";
      ldmx_log(debug) << "---> Detector ID: " << detectorId;
      ldmx_log(debug) << "---> Number of hits: " << trackFeatures.nHits;
      ldmx_log(debug) << "---> Number of holes: " << trackFeatures.nHoles;
      ldmx_log(debug) <<"---> Number of outliers: " << trackFeatures.nOutliers;
      ldmx_log(debug) <<"---> Number of shared hits: " << trackFeatures.nSharedHits;


      if ((trackFeatures.nHits < detector.minHits) ||
          (trackFeatures.nHits > detector.maxHits) ||
          (trackFeatures.nHoles > detector.maxHoles) ||
          (trackFeatures.nOutliers > detector.maxOutliers)) {
        score = 0;
        ldmx_log(debug) << "Track: " << iTrack
                            << " has score = 0, due to detector cuts";
        break;
      }
    }

    if (score == 0) {
      iTrack++;
      trackScore.push_back(score);
      ldmx_log(debug) << "Track: " << iTrack << " score : " << score;
      continue;
    }

    // real scoring starts here
    // if the ambiguity scoring function is used, the score is processed with a
    // different algorithm than the simple score.

    ldmx_log(debug) << "Using Simple Scoring function";


    score = 1;
    // Adding the score for each detector.
    // detector score is determined by the number of hits/hole/outliers *
    // hit/hole/outlier scoreWeights in a detector.
    for (std::size_t detectorId = 0; detectorId < detectorConfigs_.size();
         detectorId++) {
      const auto& detector = detectorConfigs_.at(detectorId);
      const auto& trackFeatures = trackFeaturesVector[detectorId];

      ldmx_log(debug) << score;
      score += trackFeatures.nHits * detector.hitsScoreWeight;
      ldmx_log(debug) << score;
      score += trackFeatures.nHoles * detector.holesScoreWeight;
      ldmx_log(debug) << score;
      score += trackFeatures.nOutliers * detector.outliersScoreWeight;
      ldmx_log(debug) << score;
      score += trackFeatures.nSharedHits * detector.otherScoreWeight;
      ldmx_log(debug) << score << " " << trackFeatures.nSharedHits << " " << trackFeatures.nSharedHits * detector.otherScoreWeight;

      ldmx_log(debug) << "---> Detector Configuration";
      ldmx_log(debug) << "---> Detector ID: " << detectorId;
      ldmx_log(debug) << "---> hitsScoreWeight: " << detector.hitsScoreWeight;
      ldmx_log(debug) << "---> holesScoreWeight: " << detector.holesScoreWeight;
      ldmx_log(debug) <<"---> outliersScoreWeight: " << detector.outliersScoreWeight;
      ldmx_log(debug) <<"---> otherScoreWeight: " <<  detector.otherScoreWeight;
    }

    ldmx_log(debug) << "After hit weights score is " << score;

    /*
    // Adding scores based on optional weights
    for (const auto& weightFunction : optionalCuts.weights) {
      weightFunction(track, score);
    }
    */

    // Adding the score based on the chi2/ndf
    if (track.getChi2() > 0 && track.getNdf() > 0) {
      double p = 1. / std::log10(10. * track.getChi2() /track.getNdf());
      if (p > 0) {
        score += p;
      } else {
        score -= 50;
      }
    }

    iTrack++;

    // Add the score to the vector
    trackScore.push_back(score);
    ldmx_log(debug) << "Track: " << iTrack << " score: " << score;

  }  // end of loop over tracks

  return trackScore;

}

std::vector<int> ScoreBasedAmbiguitySolver::solveAmbiguity(
     std::vector<ldmx::Track> tracks,
    const std::vector<std::vector<ScoreBasedAmbiguitySolver::MeasurementInfo>>& measurementsPerTrack,
    const std::vector<std::vector<ScoreBasedAmbiguitySolver::TrackFeatures>>& trackFeaturesVectors) const {
  //Acts::ACTS_INFO("Number of tracks before Ambiguty Resolution: " << tracks.size());
  // vector of trackFeaturesVectors. where each trackFeaturesVector contains the
  // number of hits/hole/outliers for each detector in a track.

  std::vector<double> trackScore;
  trackScore.reserve(tracks.size());
  //if (m_cfg.useAmbiguityFunction) {
    //trackScore = ambiguityScore(tracks, trackFeaturesVectors, optionalCuts);
  //} else {
  trackScore = simpleScore(tracks, trackFeaturesVectors);
  //}

  std::vector<bool> cleanTracks = ScoreBasedAmbiguitySolver::getCleanedOutTracks(
      trackScore, trackFeaturesVectors, measurementsPerTrack);

  std::vector<int> goodTracks;
  int cleanTrackIndex = 0;
  std::size_t iTrack = 0;
  for (const auto& track : tracks) {
    if (cleanTracks[iTrack]) {
      cleanTrackIndex++;
      if (trackScore[iTrack] >= minScore_) {
        goodTracks.push_back(iTrack);
      }
    }
    iTrack++;
  }
  //Acts::ACTS_VERBOSE("Number of clean tracks: " << cleanTrackIndex);
  //Acts::ACTS_VERBOSE("Min score: " << m_cfg.minScore);
  //Acts::ACTS_INFO("Number of Good tracks: " << goodTracks.size());
  return goodTracks;
}

std::vector<bool> ScoreBasedAmbiguitySolver::getCleanedOutTracks(
    const std::vector<double>& trackScore,
    const std::vector<std::vector<TrackFeatures>>& trackFeaturesVectors,
    const std::vector<std::vector<MeasurementInfo>>& measurementsPerTrack)
    const {
  std::vector<bool> cleanTracks(measurementsPerTrack.size(), false);

  //Acts::ACTS_VERBOSE("Cleaning tracks");

  if (trackScore.size() != measurementsPerTrack.size()) {
    throw std::invalid_argument(
        "Track score and measurementsPerTrack size mismatch");
  }

  std::size_t numberOfTracks = measurementsPerTrack.size();
  //Acts::ACTS_DEBUG("Number of tracks: " << numberOfTracks);

  boost::container::flat_map<std::size_t,
                             boost::container::flat_set<std::size_t>>
      tracksPerMeasurement;

  // Removes bad tracks and counts computes the vector of tracks per
  // measurement.
  for (std::size_t iTrack = 0; iTrack < numberOfTracks; ++iTrack) {
    if (trackScore[iTrack] <= 0) {
      continue;
    }
    for (auto measurementObjects : measurementsPerTrack[iTrack]) {
      auto iMeasurement = measurementObjects.iMeasurement;
      tracksPerMeasurement[iMeasurement].insert(iTrack);
    }
  }

  enum TrackStateTypes {
    // A measurement not yet used in any other track
    UnsharedHit = 1,
    // A measurement shared with another track
    SharedHit = 2,
    // A hit that needs to be removed from the track
    RejectedHit = 3,
    // an outlier, to be copied in case
    Outlier = 4,
    // other trackstate types to be copied in case
    OtherTrackStateType = 5
  };

  std::vector<std::vector<std::size_t>> newMeasurements;
  // Loop over all tracks in the track container
  for (std::size_t iTrack = 0; iTrack < numberOfTracks; ++iTrack) {
    double track_score = trackScore[iTrack];
    //Acts::ACTS_DEBUG("Track score: " << track_score);

    if (track_score <= 0) {
      //Acts::ACTS_DEBUG("Track " << iTrack << " could not be accepted - low score");
      continue;
    }

    const auto& trackFeaturesVector = trackFeaturesVectors.at(iTrack);

    bool trkCouldBeAccepted = true;

    // For tracks with shared hits, we need to check and remove bad hits

    std::vector<int> trackStateTypes(measurementsPerTrack[iTrack].size(),
                                     OtherTrackStateType);
    int index = 0;

    // Loop over all measurements of the track and for each hit a
    // trackStateTypes is assigned.
    for (const auto& measurementObjects : measurementsPerTrack[iTrack]) {
      auto iMeasurement = measurementObjects.iMeasurement;
      auto isoutliner = measurementObjects.isOutlier;
      auto detectorId = measurementObjects.detectorId;

      auto detector = detectorConfigs_.at(detectorId);
      if (isoutliner) {
        //Acts::ACTS_VERBOSE("Measurement is outlier on a fitter track, copy it over");
        trackStateTypes[index] = Outlier;
        index++;
        continue;
      }
      if (tracksPerMeasurement[iMeasurement].size() == 1) {
        //Acts::ACTS_VERBOSE("Measurement is not shared, copy it over");

        trackStateTypes[index] = UnsharedHit;

        index++;
        continue;
      }
      if (tracksPerMeasurement[iMeasurement].size() > 1) {
        //Acts::ACTS_VERBOSE("Measurement is shared, copy it over");

        if (detector.sharedHitsFlag == true) {
          //Acts::ACTS_VERBOSE("Measurement is shared, Reject it");
          trackStateTypes[index] = RejectedHit;
          index++;
          continue;
        }

        trackStateTypes[index] = SharedHit;

        index++;
        continue;
      }
    }

    std::vector<std::size_t> newMeasurementsPerTrack;
    std::size_t measurement = 0;
    std::size_t nshared = 0;

    // Loop over all measurements of the track and process them according to the
    // trackStateTypes and other conditions.
    // Good measurements are copied to the newMeasurementsPerTrack vector.
    for (std::size_t i = 0; i < trackStateTypes.size(); i++) {
      auto& measurementObjects = measurementsPerTrack[iTrack][i];
      measurement = measurementObjects.iMeasurement;

      if (trackStateTypes[i] == RejectedHit) {
        //Acts::ACTS_DEBUG("Dropping rejected hit");
      } else if (trackStateTypes[i] != SharedHit) {
        //Acts::ACTS_DEBUG("Good TSOS, copy hit");
        newMeasurementsPerTrack.push_back(measurement);

        // a counter called nshared is used to keep track of the number of
        // shared hits accepted.
      } else if (nshared >= maxShared_) {
        //Acts::ACTS_DEBUG("Too many shared hit, drop it");
      }
      // If the track is shared, the hit is only accepted if the track has score
      // higher than the minimum score for shared tracks.
      else {
        //Acts::ACTS_DEBUG("Try to recover shared hit ");
        if (tracksPerMeasurement[measurement].size() <
                maxSharedTracksPerMeasurement_ &&
            track_score > minScoreSharedTracks_) {
          //Acts::ACTS_DEBUG("Accepted hit shared with "
          //           << tracksPerMeasurement[measurement].size() << " tracks");
          newMeasurementsPerTrack.push_back(measurement);
          nshared++;
        } else {
          //Acts::ACTS_DEBUG("Rejected hit shared with "
          //           << tracksPerMeasurement[measurement].size() << " tracks");
        }
      }
    }

    // Check if the track has enough hits to be accepted.
    if (newMeasurementsPerTrack.size() < 3) {
      trkCouldBeAccepted = false;
      //Acts::ACTS_DEBUG(std::endl
       //          << "Track " << iTrack
       //          << " could not be accepted - not enough hits");
      //Acts::ACTS_DEBUG("Number of hits: " << measurementsPerTrack[iTrack].size());
      //Acts::ACTS_DEBUG("Number of good hits: " << newMeasurementsPerTrack.size());
      continue;
    }

    // Check if the track has too many shared hits to be accepted.
    for (std::size_t detectorId = 0; detectorId < detectorConfigs_.size();
         detectorId++) {
      auto detector = detectorConfigs_.at(detectorId);
      if (trackFeaturesVector[detectorId].nSharedHits >
          detector.maxSharedHits) {
        trkCouldBeAccepted = false;
        break;
      }
    }

    if (trkCouldBeAccepted) {
      cleanTracks[iTrack] = true;
      newMeasurements.push_back(newMeasurementsPerTrack);
      //Acts::ACTS_VERBOSE("Track " << iTrack << " is accepted");
      continue;
    }
  }

  return cleanTracks;
}

// Processor Functions

void ScoreBasedAmbiguitySolver::onNewRun(const ldmx::RunHeader& rh) {}

void ScoreBasedAmbiguitySolver::configure(framework::config::Parameters& parameters) {
    out_trk_collection_ =
      parameters.getParameter<std::string>("out_trk_collection", "TaggerTracksClean");

    trackCollection_= parameters.getParameter<std::string>("trackCollection", "TaggerTracks");

    measCollection_= parameters.getParameter<std::string>("measCollection", "DigiTaggerSimHits");

    verbose_= parameters.getParameter<bool>("verbose", false);
    
    std::vector<std::vector<int> > volumeMapVec = parameters.getParameter<std::vector<std::vector<int> >>("volumeMap");

    for (auto v : volumeMapVec) {volumeMap_[v[0]] = v[1];}

    minScore_ =  parameters.getParameter<double>("minScore", 0);
    minScoreSharedTracks_ =  parameters.getParameter<double>("minScoreSharedTracks", 0);
    maxSharedTracksPerMeasurement_ =  parameters.getParameter<int>("maxSharedTracksPerMeasurement", 10);
    maxShared_ =  parameters.getParameter<int>("maxShared", 5);
    pTMin_ =  parameters.getParameter<double>("pTMin", 0 * Acts::UnitConstants::GeV);
    pTMax_ =  parameters.getParameter<double>("pTMax", 1e5 * Acts::UnitConstants::GeV);

    phiMin_ =  parameters.getParameter<double>("phiMin", -M_PI * Acts::UnitConstants::rad);
    phiMax_ =  parameters.getParameter<double>("phiMax", M_PI * Acts::UnitConstants::rad);

    etaMin_ =  parameters.getParameter<double>("etaMin", -5);
    etaMax_ =  parameters.getParameter<double>("etaMax", 5);

    useAmbiguityFunction_ =  parameters.getParameter<bool>("useAmbiguityFunction", false);

    std::vector<double> hitsScoreWeight_det = parameters.getParameter<std::vector<double>>("hitsScoreWeight");
    std::vector<double> holesScoreWeight_det = parameters.getParameter<std::vector<double>>("holesScoreWeight");
    std::vector<double> outliersScoreWeight_det = parameters.getParameter<std::vector<double>>("outliersScoreWeight");
    std::vector<double> otherScoreWeight_det = parameters.getParameter<std::vector<double>>("otherScoreWeight");

    std::vector<int> minHits_det = parameters.getParameter<std::vector<int>>("minHits");
    std::vector<int> maxHits_det = parameters.getParameter<std::vector<int>>("maxHits");
    std::vector<int> maxHoles_det = parameters.getParameter<std::vector<int>>("maxHoles");
    std::vector<int> maxOutliers_det = parameters.getParameter<std::vector<int>>("maxOutliers");
    std::vector<int> maxSharedHits_det = parameters.getParameter<std::vector<int>>("maxSharedHits");

    std::vector<int> sharedHitsFlag_det = parameters.getParameter<std::vector<int>>("sharedHitsFlag");
    
    std::vector<int> detectorId_det = parameters.getParameter<std::vector<int>>("detectorId");

    std::vector<std::vector<double> > factorHits_det = parameters.getParameter<std::vector<std::vector<double> >>("factorHits");
    std::vector<std::vector<double> > factorHoles_det = parameters.getParameter<std::vector<std::vector<double> >>("factorHoles");

    for (auto iDet = 0; iDet < detectorId_det.size();iDet++) {
        ScoreBasedAmbiguitySolver::DetectorConfig det;
        det.hitsScoreWeight = hitsScoreWeight_det[iDet];
        det.holesScoreWeight = holesScoreWeight_det[iDet];
        det.outliersScoreWeight = outliersScoreWeight_det[iDet];
        det.otherScoreWeight = otherScoreWeight_det[iDet];

        det.minHits = minHits_det[iDet];
        det.maxHits = maxHits_det[iDet];
        det.maxHoles = maxHoles_det[iDet];
        det.maxOutliers = maxOutliers_det[iDet];
        det.maxSharedHits = maxSharedHits_det[iDet];

        det.sharedHitsFlag = (bool)sharedHitsFlag_det[iDet];

       
        det.detectorId = detectorId_det[iDet];

        det.factorHits = factorHits_det[iDet];
        det.factorHoles = factorHoles_det[iDet];

        detectorConfigs_.push_back(det);
    }
}

void ScoreBasedAmbiguitySolver::produce(framework::Event& event) {

  std::vector<ldmx::Track> out_tracks;


  auto tg{geometry()};

  if (!event.exists(trackCollection_)) return;
  auto tracks{event.getCollection<ldmx::Track>(trackCollection_)};

  if (!event.exists(measCollection_)) return;
  auto measurements{event.getCollection<ldmx::Measurement>(measCollection_)};

  std::vector<std::vector<ScoreBasedAmbiguitySolver::TrackFeatures>>
      trackFeaturesVectors;

  std::vector<std::vector<ScoreBasedAmbiguitySolver::MeasurementInfo>>
      measurementsPerTracks;

  measurementsPerTracks = computeInitialState(tracks, measurements, 
  tracking::sim::utils::sourceLinkHash, tracking::sim::utils::sourceLinkEquality, 
  tg, trackFeaturesVectors);

  std::vector<int> goodTracks = solveAmbiguity(
      tracks, measurementsPerTracks, trackFeaturesVectors);


  for (auto iTrack : goodTracks) {
      auto clean_trk = tracks[iTrack];
      //if (clean_trk.getNhits() > nMeasurementsMin_ && abs(1. / clean_trk.getQoP()) > 0.05) {
        out_tracks.push_back(clean_trk);
     //}
    }

   ldmx_log(debug) <<  " " << "Resolved to " << goodTracks.size() << " tracks from "
                           << " " << tracks.size();

    event.add(out_trk_collection_, out_tracks);

}

void ScoreBasedAmbiguitySolver::onProcessStart(){};
void ScoreBasedAmbiguitySolver::onProcessEnd(){};

}
}

DECLARE_PRODUCER_NS(tracking::reco, ScoreBasedAmbiguitySolver)
