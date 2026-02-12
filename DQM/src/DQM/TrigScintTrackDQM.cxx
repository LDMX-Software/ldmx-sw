#include "DQM/TrigScintTrackDQM.h"

namespace dqm {

TrigScintTrackDQM::TrigScintTrackDQM(const std::string &name,
                                     framework::Process &process)
    : framework::Analyzer(name, process) {}

void TrigScintTrackDQM::onProcessStart() {
  getHistoDirectory();

  histograms_.create("centroid", "Track channel centroid", 500, 0, 100);
  histograms_.create(
      "n_tracks", "TrigScint track multiplicity in the pad/event", 25, 0, 25);
  histograms_.create("n_clusters", "n_{clusters} forming the track", 4, 0, 4);
  histograms_.create("residual", "Track residual [channels]", 100, 0., 2.);
  histograms_.create("beamEfrac",
                     "Track edep fraction associated with beam electron", 101,
                     0., 1.01);
  histograms_.create("x", "Track x position [mm]", 1000, -100, 100);
  histograms_.create("y", "Track y position [mm]", 1000, -100, 100);
  histograms_.create("z", "Track z position [mm]", 1000, -900, 100);

  // TODO: implement getting a list of the constructed histograms, to iterate
  // through and set overflow boolean.
}

void TrigScintTrackDQM::configure(framework::config::Parameters &ps) {
  track_collection_name_ = ps.get<std::string>("track_collection");
  pass_name_ = ps.get<std::string>("pass_name").c_str();

  ldmx_log(debug) << "Collection name = " << track_collection_name_
                  << " pass name =" << pass_name_;
}

void TrigScintTrackDQM::analyze(const framework::Event &event) {
  // Get the collection of TrigScintTrack digitized tracks if the exists
  const std::vector<ldmx::TrigScintTrack> trig_scint_tracks =
      event.getCollection<ldmx::TrigScintTrack>(track_collection_name_,
                                                pass_name_);

  // Loop through all TrigScint tracks in the event
  for (const ldmx::TrigScintTrack &track : trig_scint_tracks) {
    histograms_.fill("centroid", track.getCentroid());
    histograms_.fill("residual", track.getResidual());
    histograms_.fill("n_clusters", track.getNclusters());
    histograms_.fill("beamEfrac", track.getBeamEfrac());

    histograms_.fill("x", track.getCentroidX());
    histograms_.fill("y", track.getCentroidY());
    histograms_.fill("z", track.getCentroidZ());
  }

  histograms_.fill("n_tracks", trig_scint_tracks.size());
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::TrigScintTrackDQM)
