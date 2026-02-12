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
}

double getP(const ldmx::SimTrackerHit& tk) {
  std::vector<double> pxyz = tk.getMomentum();
  return sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
}

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
      if (do_electron_tracking_) {  // only select electron SP hits_
        if (sp_hit.getPdgID() != 11) continue;
        if (sp_hit.getTrackID() < 2 &&
            sp_hit.getMomentum()[2] > min_electron_momentum_z_) {
          // this is almost guaranteed to be a pileup beam electron! keep it
          pf_tracks.push_back(sp_hit);
          ldmx_log(debug) << "Added beam electron SP hit: trackID="
                          << sp_hit.getTrackID()
                          << ", pz = " << sp_hit.getMomentum()[2];
        } else if (sp_hit.getTrackID() <= max_electron_track_id_ &&
                   sp_hit.getMomentum()[2] > 5) {
          // require more than minimum forward momentum to catch recoil electron
          // candidates
          pf_tracks.push_back(sp_hit);
          ldmx_log(debug) << "Adding SP hit: trackID=" << sp_hit.getTrackID()
                          << ", pdgID= " << sp_hit.getPdgID()
                          << ", pz = " << sp_hit.getMomentum()[2];
          continue;
        }
      }  // if electron tracking
      else {
        if (sp_hit.getTrackID() != 1 ||
            fabs(240 - sp_hit.getPosition()[2]) > 0.1 ||
            sp_hit.getMomentum()[2] < 0)
          continue;
        pf_tracks.push_back(sp_hit);
        ldmx_log(debug) << "Adding SP hit: trackID=" << sp_hit.getTrackID()
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
