#include "Recon/PFTrackProducer.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Track.h"

namespace recon {

void PFTrackProducer::configure(framework::config::Parameters& ps) {
  truth_tracking_ = ps.get<bool>("truth_tracking");
  input_track_coll_name_ = ps.get<std::string>("input_track_coll_name");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  output_track_coll_name_ = ps.get<std::string>("output_track_coll_name");
  do_electron_tracking_ = ps.get<bool>("do_electron_tracking");
  min_electron_momentum_z_ = ps.get<double>("min_electron_momentum_z");
  max_electron_track_id_ = ps.get<int>("max_electron_track_id");
  reco_track_coll_name_ = ps.get<std::string>("reco_track_coll_name");
  reco_track_pass_name_ = ps.get<std::string>("reco_track_pass_name");
  min_n_hits_ = ps.get<int>("min_n_hits");
  max_chi2_ndf_ = ps.get<double>("max_chi2_ndf");
  min_momentum_ = ps.get<double>("min_momentum");
  max_shared_frac_ = ps.get<double>("max_shared_frac");
}

namespace {

double getP(const std::vector<double>& pxyz) {
  return sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
}

double getP(const ldmx::Track& tk) {
  auto pxyz = tk.getMomentum(ldmx::AtECAL);
  return pxyz.empty() ? 0. : getP(pxyz);
}

// charge of common long lived particles, 0 otherwise
int chargeFromPdg(int pdg) {
  int sign = pdg > 0 ? 1 : -1;
  switch (std::abs(pdg)) {
    case 11:
    case 13:
    case 15:
      return -sign;
    case 211:
    case 321:
    case 2212:
      return sign;
    default:
      return 0;
  }
}

// wrap a scoring plane hit into a track with a single AtECAL state
ldmx::Track trackFromSPHit(const ldmx::SimTrackerHit& sp_hit) {
  ldmx::Track::TrackState ts;
  auto xyz = sp_hit.getPosition();
  ts.pos_ = {xyz[0], xyz[1], xyz[2]};
  ts.mom_ = sp_hit.getMomentum();
  ts.ts_type_ = ldmx::AtECAL;
  ldmx::Track tk;
  tk.addTrackState(ts);
  tk.setTrackID(sp_hit.getTrackID());
  tk.setPdgID(sp_hit.getPdgID());
  tk.setCharge(chargeFromPdg(sp_hit.getPdgID()));
  tk.setTruthProb(1.);
  return tk;
}

}  // namespace

void PFTrackProducer::produce(framework::Event& event) {
  if (truth_tracking_)
    produceTruth(event);
  else
    produceReco(event);
}

void PFTrackProducer::produceTruth(framework::Event& event) {
  if (!event.exists(input_track_coll_name_, input_pass_name_)) {
    ldmx_log(fatal) << "Couldn't find input collection "
                    << input_track_coll_name_ << "_" << input_pass_name_;
    return;
  }
  const auto ecal_sp_hits = event.getCollection<ldmx::SimTrackerHit>(
      input_track_coll_name_, input_pass_name_);

  std::vector<ldmx::Track> pf_tracks;
  for (const auto& sp_hit : ecal_sp_hits) {
    if (sp_hit.getPdgID() == 22 || sp_hit.getPdgID() == 2112) continue;
    if (fabs(240 - sp_hit.getPosition()[2]) > 0.1) continue;
    if (do_electron_tracking_) {  // only select electron SP hits_
      if (sp_hit.getPdgID() != 11) continue;
      if (sp_hit.getTrackID() < 2 &&
          sp_hit.getMomentum()[2] > min_electron_momentum_z_) {
        // this is almost guaranteed to be a pileup beam electron! keep it
        pf_tracks.push_back(trackFromSPHit(sp_hit));
        ldmx_log(debug) << "Added beam electron SP hit: trackID="
                        << sp_hit.getTrackID()
                        << ", pz = " << sp_hit.getMomentum()[2];
      } else if (sp_hit.getTrackID() <= max_electron_track_id_ &&
                 sp_hit.getMomentum()[2] > 5) {
        // require more than minimum forward momentum to catch recoil electron
        // candidates
        pf_tracks.push_back(trackFromSPHit(sp_hit));
        ldmx_log(debug) << "Adding SP hit: trackID=" << sp_hit.getTrackID()
                        << ", pdgID= " << sp_hit.getPdgID()
                        << ", pz = " << sp_hit.getMomentum()[2];
        continue;
      }
    }  // if electron tracking
    else {
      if (sp_hit.getTrackID() != 1 || sp_hit.getMomentum()[2] < 0) continue;
      pf_tracks.push_back(trackFromSPHit(sp_hit));
      ldmx_log(debug) << "Adding SP hit: trackID=" << sp_hit.getTrackID()
                      << ", pdgID= " << sp_hit.getPdgID()
                      << ", pz = " << sp_hit.getMomentum()[2];
      break;
    }
  }  // over SP hits_
  std::sort(pf_tracks.begin(), pf_tracks.end(),
            [](const ldmx::Track& a, const ldmx::Track& b) {
              return getP(a) > getP(b);
            });
  event.add(output_track_coll_name_, pf_tracks);
}

void PFTrackProducer::produceReco(framework::Event& event) {
  if (!event.exists(reco_track_coll_name_, reco_track_pass_name_)) {
    ldmx_log(fatal) << "Couldn't find input collection "
                    << reco_track_coll_name_ << "_" << reco_track_pass_name_;
    return;
  }
  const auto tracks = event.getCollection<ldmx::Track>(reco_track_coll_name_,
                                                       reco_track_pass_name_);

  // best tracks first: more hits, then lower chi2/ndf
  std::vector<const ldmx::Track*> ranked;
  for (const auto& tk : tracks) ranked.push_back(&tk);
  auto chi2ndf = [](const ldmx::Track* t) {
    return t->getNdf() > 0 ? t->getChi2() / t->getNdf() : 9e9;
  };
  std::stable_sort(ranked.begin(), ranked.end(), [&](auto a, auto b) {
    if (a->getNhits() != b->getNhits()) return a->getNhits() > b->getNhits();
    return chi2ndf(a) < chi2ndf(b);
  });

  std::vector<ldmx::Track> pf_tracks;
  std::vector<std::set<unsigned int>> used_hits;
  for (const auto* tk_ptr : ranked) {
    const auto& tk = *tk_ptr;
    // need the Ecal face state for linking
    auto mom = tk.getMomentum(ldmx::AtECAL);
    if (mom.empty()) {
      ldmx_log(debug) << "Dropping track without AtECAL state";
      continue;
    }
    if (mom[2] <= 0) continue;
    if (tk.getNhits() < min_n_hits_) continue;
    if (max_chi2_ndf_ >= 0 &&
        (tk.getNdf() <= 0 || tk.getChi2() / tk.getNdf() > max_chi2_ndf_))
      continue;
    if (getP(mom) < min_momentum_) continue;
    // drop duplicates sharing most hits with a better track
    auto idxs = tk.getMeasurementsIdxs();
    std::set<unsigned int> hits(idxs.begin(), idxs.end());
    bool duplicate = false;
    for (const auto& prev : used_hits) {
      int n_shared = 0;
      for (auto h : hits) n_shared += prev.count(h);
      if (!hits.empty() && n_shared > max_shared_frac_ * hits.size()) {
        duplicate = true;
        break;
      }
    }
    if (duplicate) continue;
    used_hits.push_back(hits);
    pf_tracks.push_back(tk);
    ldmx_log(debug) << "Adding track: nhits=" << tk.getNhits()
                    << ", p = " << getP(mom) << ", trackID=" << tk.getTrackID();
  }
  std::sort(pf_tracks.begin(), pf_tracks.end(),
            [](const ldmx::Track& a, const ldmx::Track& b) {
              return getP(a) > getP(b);
            });
  event.add(output_track_coll_name_, pf_tracks);
}
}  // namespace recon

DECLARE_PRODUCER(recon::PFTrackProducer);
