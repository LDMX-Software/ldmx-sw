#include "Recon/PFTrackProducer.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

void PFTrackProducer::configure(framework::config::Parameters& ps) {
  input_track_coll_name_ = ps.get<std::string>("input_track_coll_name");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  output_track_coll_name_ = ps.get<std::string>("output_track_coll_name");
  do_electron_tracking_ = ps.get<bool>("do_electron_tracking");
  min_electron_momentum_z_ = ps.get<double>("min_electron_momentum_z");
  max_electron_track_id_ = ps.get<int>("max_electron_track_id");
  decode_overlay_track_ids_ = ps.get<bool>("decode_overlay_track_ids");
}

double getP(const ldmx::SimTrackerHit& tk) {
  std::vector<double> pxyz = tk.getMomentum();
  return sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
}

// Undo the bitwise track ID encoding applied by OverlayProducer:
// version tag in bits 27-31, event index in bits 24-26, original
// track ID in bits 0-23. Only strips the top bits; a no-op for
// plain (unencoded) track IDs below 2^24.
int decodeTrackID(int track_id) { return track_id & 0x00FFFFFF; }

void PFTrackProducer::produce(framework::Event& event) {
  if (!event.exists(input_track_coll_name_, input_pass_name_)) {
    ldmx_log(fatal) << "Couldn't find input collection "
                    << input_track_coll_name_ << "_" << input_pass_name_;
    return;
  }
  const auto ecal_sp_hits = event.getCollection<ldmx::SimTrackerHit>(
      input_track_coll_name_, input_pass_name_);

  std::vector<ldmx::SimTrackerHit> pf_tracks;
  if (truth_tracking_) {
    for (const auto& sp_hit : ecal_sp_hits) {
      if (sp_hit.getPdgID() == 22 || sp_hit.getPdgID() == 2112) continue;
      if (fabs(240 - sp_hit.getPosition()[2]) > 0.1) continue;

      // recover the original track ID if the input was overlay-encoded
      int track_id;
      if (decode_overlay_track_ids_) {
        track_id = decodeTrackID(sp_hit.getTrackID());
      } else {
        track_id = sp_hit.getTrackID();
      }
      
      if (do_electron_tracking_) {  // only select electron SP hits_
        if (sp_hit.getPdgID() != 11) continue;
        if (track_id < 2 &&
            sp_hit.getMomentum()[2] > min_electron_momentum_z_) {
          // this is almost guaranteed to be a pileup beam electron! keep it
          ldmx::SimTrackerHit kept{sp_hit};
          kept.setTrackID(track_id);
          pf_tracks.push_back(kept);
          ldmx_log(debug) << "Added beam electron SP hit: trackID=" << track_id
                          << ", pz = " << sp_hit.getMomentum()[2];
        } else if (track_id <= max_electron_track_id_ &&
                   sp_hit.getMomentum()[2] > 5) {
          // require more than minimum forward momentum to catch recoil electron
          // candidates
          ldmx::SimTrackerHit kept{sp_hit};
          kept.setTrackID(track_id);
          pf_tracks.push_back(kept);
          ldmx_log(debug) << "Adding SP hit: trackID=" << track_id
                          << ", pdgID= " << sp_hit.getPdgID()
                          << ", pz = " << sp_hit.getMomentum()[2];
          continue;
        }
      }  // if electron tracking
      else {
        if (track_id != 1 || fabs(240 - sp_hit.getPosition()[2]) > 0.1 ||
            sp_hit.getMomentum()[2] < 0)
          continue;
        ldmx::SimTrackerHit kept{sp_hit};
        kept.setTrackID(track_id);
        pf_tracks.push_back(kept);
        ldmx_log(debug) << "Adding SP hit: trackID=" << track_id
                        << ", pdgID= " << sp_hit.getPdgID()
                        << ", pz = " << sp_hit.getMomentum()[2];
        break;
      }
    }  // over SP hits_
  }  // do truth tracking
  std::sort(pf_tracks.begin(), pf_tracks.end(),
            [](ldmx::SimTrackerHit a, ldmx::SimTrackerHit b) {
              return getP(a) > getP(b);
            });
  event.add(output_track_coll_name_, pf_tracks);
}
}  // namespace recon

DECLARE_PRODUCER(recon::PFTrackProducer);
