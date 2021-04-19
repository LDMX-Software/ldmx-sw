/**
 * @file RecoilTrackerDQM.cxx
 * @brief Analyzer used for DQM of the Recoil tracker.
 * @author Omar Moreno, SLAC National Accelerator
 */

#include "DQM/RecoilTrackerDQM.h"

#include "TVector3.h"

#include "SimCore/Event/SimTrackerHit.h"

namespace dqm {

RecoilTrackerDQM::RecoilTrackerDQM(const std::string &name,
                                   framework::Process &process)
    : framework::Analyzer(name, process) {}

RecoilTrackerDQM::~RecoilTrackerDQM() {}

void RecoilTrackerDQM::onProcessStart() {}

void RecoilTrackerDQM::configure(framework::config::Parameters &parameters) {}

void RecoilTrackerDQM::analyze(const framework::Event &event) {
  // If the collection of findable tracks doesn't exist, stop processing
  // the event.
  if (!event.exists("FindableTracks")) return;

  auto recoilTrackID = 0;
  // Get the collection of simulated particles from the event
  /*const std::vector<FindableTrackResult> tracks
      = event.getCollection<FindableTrackResult>("FindableTracks");

  TrackMaps map = Analysis::getFindableTrackMaps(tracks);

  histograms_.fill("track_count",map.findable.size());
  histograms_.fill("loose_track_count",map.loose.size());
  histograms_.fill("axial_track_count",map.axial.size());

  // Get the collection of simulated particles from the event
  auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};

  // Search for the recoil electron
  auto [recoilTrackID, recoil] = Analysis::getRecoil(particleMap);

  auto it = map.findable.find(recoilTrackID);
  bool recoilIsFindable = ( it != map.findable.end() );

  // Fill the recoil vertex position histograms
  std::vector<double> recoilVertex = recoil->getVertex();
  histograms_.fill("recoil_vx",recoilVertex[0]);
  histograms_.fill("recoil_vy",recoilVertex[1]);
  histograms_.fill("recoil_vz",recoilVertex[2]);  */

  double p{-1}, pt{-1}, px{-9999}, py{-9999}, pz{-9999};
  const ldmx::SimTrackerHit *spHit{nullptr};
  if (event.exists("TargetScoringPlaneHits")) {
    // Get the collection of simulated particles from the event
    const std::vector<ldmx::SimTrackerHit> spHits =
        event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");

    for (const ldmx::SimTrackerHit &hit : spHits) {
      if ((hit.getTrackID() == recoilTrackID) /*hit caused by recoil*/ and
          (hit.getLayerID() == 2) /*hit on downstream side of target*/ and
          (hit.getMomentum()[2] > 0) /*hit momentum leaving target*/
      ) {
        spHit = &hit;
        break;
      }
    }

    if (spHit) {
      TVector3 recoilP(spHit->getMomentum().data());

      p = recoilP.Mag();
      pt = recoilP.Pt();
      px = recoilP.Px();
      py = recoilP.Py();
      pz = recoilP.Pz();
    }
  }

  histograms_.fill("tp", p);
  histograms_.fill("tpt", pt);
  histograms_.fill("tpx", px);
  histograms_.fill("tpy", py);
  histograms_.fill("tpz", pz);

  bool passesTrackVeto{false};
  // Check if the TrackerVeto result exists
  if (event.exists("TrackerVeto")) {
    // Get the collection of trackerVeto results
    /*const TrackerVetoResult trackerVeto =
    event.getObject<TrackerVetoResult>("TrackerVeto");

    // Check if the event passes the tracker veto
    if (trackerVeto.passesVeto()) {
        passesTrackVeto = true;
    }*/
  }

  if (passesTrackVeto) {
    histograms_.fill("tp_track_veto", p);
    histograms_.fill("tpt_track_veto", pt);
    histograms_.fill("tpx_track_veto", px);
    histograms_.fill("tpy_track_veto", py);
    histograms_.fill("tpz_track_veto", pz);
  }
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, RecoilTrackerDQM)
