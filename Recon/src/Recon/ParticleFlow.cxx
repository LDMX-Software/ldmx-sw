#include "Recon/ParticleFlow.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "Ecal/Event/EcalCluster.h"
#include "Framework/Exception/Exception.h"

namespace recon {

void ParticleFlow::configure(framework::config::Parameters& ps) {
  // I/O
  input_ecal_coll_name_ = ps.get<std::string>("input_ecal_coll_name");
  input_hcal_coll_name_ = ps.get<std::string>("input_hcal_coll_name");
  input_track_coll_name_ = ps.get<std::string>("input_track_coll_name");
  output_coll_name_ = ps.get<std::string>("output_coll_name");
  input_ecal_passname_ = ps.get<std::string>("input_ecal_passname");
  input_hcal_passname_ = ps.get<std::string>("input_hcal_passname");
  input_tracks_passname_ = ps.get<std::string>("input_tracks_passname");
  truth_sp_coll_name_ = ps.get<std::string>("truth_sp_coll_name");
  truth_sp_passname_ = ps.get<std::string>("truth_sp_passname");

  // Algorithm configuration
  single_particle_ = ps.get<bool>("single_particle");
  use_existing_ecal_clusters_ = ps.get<bool>("use_existing_ecal_clusters");

  // energy correction tables, empty means no correction
  ecal_corr_energies_ = ps.get<std::vector<double>>("ecal_corr_energies", {});
  ecal_corr_factors_ = ps.get<std::vector<double>>("ecal_corr_factors", {});
  hcal_corr_energies_ = ps.get<std::vector<double>>("hcal_corr_energies", {});
  hcal_corr_factors_ = ps.get<std::vector<double>>("hcal_corr_factors", {});
  for (const auto& [x, y] :
       {std::pair{&ecal_corr_energies_, &ecal_corr_factors_},
        std::pair{&hcal_corr_energies_, &hcal_corr_factors_}}) {
    if (x->size() != y->size() || !std::is_sorted(x->begin(), x->end())) {
      EXCEPTION_RAISE("InvalidArg",
                      "Correction energies must be sorted and match factors");
    }
  }
}

// linear interpolation, clamped to the end factors
double ParticleFlow::correction(const std::vector<double>& energies,
                                const std::vector<double>& factors,
                                double energy) {
  if (energies.empty()) return 1.;
  if (energy <= energies.front()) return factors.front();
  if (energy >= energies.back()) return factors.back();
  auto hi = std::upper_bound(energies.begin(), energies.end(), energy);
  size_t i = hi - energies.begin();
  double t = (energy - energies[i - 1]) / (energies[i] - energies[i - 1]);
  return factors[i - 1] + t * (factors[i] - factors[i - 1]);
}

// produce candidate track info
void ParticleFlow::fillCandTrack(
    ldmx::PFCandidate& cand, const ldmx::Track& tk,
    const std::vector<ldmx::SimTrackerHit>& sp_hits) {
  auto ts = tk.getTrackState(ldmx::AtECAL);
  if (!ts || ts->pos_.size() < 3 || ts->mom_.size() < 3) return;
  const std::vector<double>& xyz = ts->pos_;
  const std::vector<double>& pxyz = ts->mom_;
  float ecal_z = 248;
  float ecal_x =
      xyz[0] + pxyz[0] / pxyz[2] * (ecal_z - xyz[2]);  // project onto ecal face
  float ecal_y = xyz[1] + pxyz[1] / pxyz[2] * (ecal_z - xyz[2]);
  cand.setEcalPositionXYZ(ecal_x, ecal_y, ecal_z);
  cand.setTrackPxPyPz(pxyz[0], pxyz[1], pxyz[2]);
  cand.setPID(cand.getPID() | 1);  // OR with 001

  // truth from the matched Ecal SP hit closest in momentum
  const ldmx::SimTrackerHit* best = nullptr;
  double best_dp = 9e9;
  for (const auto& sp : sp_hits) {
    if (sp.getTrackID() != tk.getTrackID()) continue;
    if (fabs(240 - sp.getPosition()[2]) > 0.1) continue;
    auto sp_p = sp.getMomentum();
    double dp =
        std::hypot(sp_p[0] - pxyz[0], sp_p[1] - pxyz[1], sp_p[2] - pxyz[2]);
    if (dp < best_dp) {
      best_dp = dp;
      best = &sp;
    }
  }
  if (!best) return;
  std::vector<float> t_xyz = best->getPosition();
  std::vector<double> t_pxyz = best->getMomentum();
  cand.setTruthEcalXYZ(t_xyz[0] + t_pxyz[0] / t_pxyz[2] * (ecal_z - t_xyz[2]),
                       t_xyz[1] + t_pxyz[1] / t_pxyz[2] * (ecal_z - t_xyz[2]),
                       ecal_z);
  cand.setTruthPxPyPz(t_pxyz[0], t_pxyz[1], t_pxyz[2]);
  float m2 = pow(best->getEnergy(), 2) - pow(t_pxyz[0], 2) - pow(t_pxyz[1], 2) -
             pow(t_pxyz[2], 2);
  if (m2 < 0) m2 = 0;
  cand.setTruthMass(sqrt(m2));
  cand.setTruthEnergy(best->getEnergy());
  cand.setTruthPdgId(best->getPdgID());
}
// produce candidate ECal info
void ParticleFlow::fillCandEMCalo(ldmx::PFCandidate& cand,
                                  const ldmx::CaloCluster& em) {
  float energy = em.getEnergy();
  float corr = correction(ecal_corr_energies_, ecal_corr_factors_, energy);
  cand.setEcalEnergy(energy * corr);
  cand.setEcalRawEnergy(energy);
  cand.setEcalClusterXYZ(em.getCentroidX(), em.getCentroidY(),
                         em.getCentroidZ());
  cand.setEcalClusterEXYZ(em.getRMSX(), em.getRMSY(), em.getRMSZ());
  cand.setEcalClusterDXDZ(em.getDXDZ());
  cand.setEcalClusterDYDZ(em.getDYDZ());
  cand.setEcalClusterEDXDZ(em.getEDXDZ());
  cand.setEcalClusterEDYDZ(em.getEDYDZ());
  cand.setPID(cand.getPID() | 2);  // OR with 010
}
// produce candidate HCal info
void ParticleFlow::fillCandHadCalo(ldmx::PFCandidate& cand,
                                   const ldmx::CaloCluster& had) {
  float energy = had.getEnergy();
  float corr = correction(hcal_corr_energies_, hcal_corr_factors_, energy);
  cand.setHcalEnergy(energy * corr);
  cand.setHcalRawEnergy(energy);
  cand.setHcalClusterXYZ(had.getCentroidX(), had.getCentroidY(),
                         had.getCentroidZ());
  cand.setHcalClusterEXYZ(had.getRMSX(), had.getRMSY(), had.getRMSZ());
  cand.setHcalClusterDXDZ(had.getDXDZ());
  cand.setHcalClusterDYDZ(had.getDYDZ());
  cand.setHcalClusterEDXDZ(had.getEDXDZ());
  cand.setHcalClusterEDYDZ(had.getEDYDZ());
  cand.setPID(cand.getPID() | 4);  // OR with 100
}

// produce track, ecal, and hcal linking
void ParticleFlow::produce(framework::Event& event) {
  if (!event.exists(input_track_coll_name_, input_tracks_passname_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << input_track_coll_name_ << "_" << input_tracks_passname_;
    return;
  }
  if (!event.exists(input_ecal_coll_name_, input_ecal_passname_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << input_ecal_coll_name_ << "_" << input_ecal_passname_;
    return;
  }
  if (!event.exists(input_hcal_coll_name_, input_hcal_passname_)) {
    ldmx_log(error) << "Unable to find (one) collection named "
                    << input_hcal_coll_name_ << "_" << input_hcal_passname_;
    return;
  }
  // get the track and clustering info
  const auto hcal_clusters = event.getCollection<ldmx::CaloCluster>(
      input_hcal_coll_name_, input_hcal_passname_);
  const auto tracks = event.getCollection<ldmx::Track>(input_track_coll_name_,
                                                       input_tracks_passname_);
  // truth info is optional
  std::vector<ldmx::SimTrackerHit> sp_hits;
  if (event.exists(truth_sp_coll_name_, truth_sp_passname_))
    sp_hits = event.getCollection<ldmx::SimTrackerHit>(truth_sp_coll_name_,
                                                       truth_sp_passname_);
  // here allow for using existing clusters of different type (EcalCluster)
  const auto ecal_clusters =
      use_existing_ecal_clusters_
          ? getEcalClusters(event, input_ecal_coll_name_, input_ecal_passname_)
          : event.getCollection<ldmx::CaloCluster>(input_ecal_coll_name_,
                                                   input_ecal_passname_);

  std::vector<ldmx::PFCandidate> pf_cands;
  // multi-particle case
  if (!single_particle_) {
    /*
      1. Build links maps at the Tk/Ecal and Hcal/Hcal interfaces
      2. Categorize tracks as: Ecal-matched, (Side) Hcal-matched, unmatched
      3. Categorize Ecal clusters as: Hcal-matched, unmatched
      4a. (Upstream?) Categorize tracks with dedx?
      4b. (Upstream?) Categorize ecal clusters as: EM/Had-like
      4c. (Upstream?) Categorize hcal clusters as: EM/Had-like
      5. Build candidates by category, moving from Tk-Ecal-Hcal
    */

    //
    // track-calo linking
    //
    std::vector<bool> tk_usable(tracks.size(), false);
    std::map<int, std::vector<int>> tk_calo_map;
    for (int i = 0; i < tracks.size(); i++) {
      const auto& tk = tracks[i];
      const std::vector<double> xyz = tk.getPosition(ldmx::AtECAL);
      const std::vector<double> pxyz = tk.getMomentum(ldmx::AtECAL);
      // need a forward Ecal face state to extrapolate
      if (pxyz.empty() || pxyz[2] <= 0) {
        ldmx_log(warn) << "Skipping track " << i << " without AtECAL state";
        continue;
      }
      tk_usable[i] = true;
      const float p = sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
      for (int j = 0; j < ecal_clusters.size(); j++) {
        const auto& ecal = ecal_clusters[j];
        // Matching logic
        const float ecal_clus_z = ecal.getCentroidZ();
        const float tk_x_at_clus =
            xyz[0] +
            pxyz[0] / pxyz[2] * (ecal_clus_z - xyz[2]);  // extrapolation
        const float tk_y_at_clus =
            xyz[1] + pxyz[1] / pxyz[2] * (ecal_clus_z - xyz[2]);
        float dist = hypot((tk_x_at_clus - ecal.getCentroidX()) /
                               std::max(1.0, ecal.getRMSX()),
                           (tk_y_at_clus - ecal.getCentroidY()) /
                               std::max(1.0, ecal.getRMSY()));
        bool is_match =
            (dist < 2) && (ecal.getEnergy() > 0.3 * p &&
                           ecal.getEnergy() < 2 * p);  // matching criteria *
        if (is_match) tk_calo_map[i].push_back(j);
      }
    }

    // em-hadcalo linking
    std::map<int, std::vector<int>> em_had_calo_map;
    for (int i = 0; i < ecal_clusters.size(); i++) {
      const auto& ecal = ecal_clusters[i];
      for (int j = 0; j < hcal_clusters.size(); j++) {
        const auto& hcal = hcal_clusters[j];
        const float x_at_h_clus =
            ecal.getCentroidX() +
            ecal.getDXDZ() * (hcal.getCentroidZ() -
                              ecal.getCentroidZ());  // extrapolated position
        const float y_at_h_clus =
            ecal.getCentroidY() +
            ecal.getDYDZ() * (hcal.getCentroidZ() - ecal.getCentroidZ());
        float dist = sqrt(
            pow(x_at_h_clus - hcal.getCentroidX(), 2) /
                std::max(1.0, pow(hcal.getRMSX(), 2) + pow(ecal.getRMSX(), 2)) +
            pow(y_at_h_clus - hcal.getCentroidY(), 2) /
                std::max(1.0, pow(hcal.getRMSY(), 2) + pow(ecal.getRMSY(), 2)));
        bool is_match = (dist < 5);  // matching criteria, was 2
        if (is_match) em_had_calo_map[i].push_back(j);
      }
    }

    // TODO: tk-hadcalo linking (Side HCal)

    //
    // track / ecal cluster arbitration
    //
    std::vector<bool> tk_is_em_linked(tracks.size(), false);
    std::vector<bool> em_is_tk_linked(ecal_clusters.size(), false);
    std::map<int, int> tk_em_pairs{};
    for (int i = 0; i < tracks.size(); i++) {
      if (tk_calo_map.count(i)) {
        // pick first (highest-energy) unused matching cluster
        for (int em_idx : tk_calo_map[i]) {
          if (!em_is_tk_linked[em_idx]) {
            em_is_tk_linked[em_idx] = true;
            tk_is_em_linked[i] = true;
            tk_em_pairs[i] = em_idx;
            break;
          }
        }
      }
    }

    // ecal / hcal cluster arbitration
    std::vector<bool> em_is_had_linked(ecal_clusters.size(), false);
    std::vector<bool> had_is_em_linked(hcal_clusters.size(), false);
    std::map<int, int> em_had_pairs{};
    for (int i = 0; i < ecal_clusters.size(); i++) {
      if (em_had_calo_map.count(i)) {
        // pick first (highest-energy) unused matching cluster
        for (int had_idx : em_had_calo_map[i]) {
          if (!had_is_em_linked[had_idx]) {
            had_is_em_linked[had_idx] = true;
            em_is_had_linked[i] = true;
            em_had_pairs[i] = had_idx;
            break;
          }
        }
      }
    }

    // can consider combining satellite clusters here...
    // define some "primary cluster" ID criterion
    //   and can add fails to the primaries

    //
    // Begin building pf candidates from tracks
    //
    for (int i = 0; i < tracks.size(); i++) {
      if (!tk_usable[i]) continue;
      ldmx::PFCandidate cand;
      fillCandTrack(cand, tracks[i],
                    sp_hits);  // append track info to candidate
      cand.setTrackIndex(i);
      if (tk_is_em_linked[i]) {  // if track is linked with ECal cluster
        int em_idx = tk_em_pairs[i];
        fillCandEMCalo(cand, ecal_clusters[em_idx]);
        cand.setEcalIndex(em_idx);
        if (em_is_had_linked[em_idx]) {  // if ECal is linked with HCal
          fillCandHadCalo(cand, hcal_clusters[em_had_pairs[em_idx]]);
          cand.setHcalIndex(em_had_pairs[em_idx]);
        }
      }
      // charged energy from the track
      auto p = cand.getTrackPxPyPz();
      cand.setEnergy(std::hypot(p[0], p[1], p[2]));
      pf_cands.push_back(cand);
    }

    // neutral candidates from Ecal clusters not linked to a track
    for (int i = 0; i < ecal_clusters.size(); i++) {
      if (em_is_tk_linked[i]) continue;
      ldmx::PFCandidate cand;
      fillCandEMCalo(cand, ecal_clusters[i]);
      cand.setEcalIndex(i);
      if (em_is_had_linked[i]) {
        fillCandHadCalo(cand, hcal_clusters[em_had_pairs[i]]);
        cand.setHcalIndex(em_had_pairs[i]);
      }
      cand.setEnergy(cand.getEcalEnergy() + cand.getHcalEnergy());
      pf_cands.push_back(cand);
    }

    // neutral candidates from unlinked Hcal clusters
    for (int i = 0; i < hcal_clusters.size(); i++) {
      if (had_is_em_linked[i]) continue;
      ldmx::PFCandidate cand;
      fillCandHadCalo(cand, hcal_clusters[i]);
      cand.setHcalIndex(i);
      cand.setEnergy(cand.getHcalEnergy());
      pf_cands.push_back(cand);
    }

  } else {
    // Single-particle builder
    ldmx::PFCandidate pf;
    int pid = 0;  // initialize pid to add
    if (tracks.size() && !tracks[0].getMomentum(ldmx::AtECAL).empty()) {
      fillCandTrack(pf, tracks[0], sp_hits);
      pf.setTrackIndex(0);
      pid += 1;
    }
    if (ecal_clusters.size()) {
      fillCandEMCalo(pf, ecal_clusters[0]);
      pf.setEcalIndex(0);
      pid += 2;
    }
    if (hcal_clusters.size()) {
      fillCandHadCalo(pf, hcal_clusters[0]);
      pf.setHcalIndex(0);
      pid += 4;
    }
    pf.setPID(pid);
    pf.setEnergy(pf.getEcalEnergy() + pf.getHcalEnergy());
    pf_cands.push_back(pf);
  }

  event.add(output_coll_name_, pf_cands);
}
// stupid function to type cast from ecal to calo cluster
const std::vector<ldmx::CaloCluster> ParticleFlow::getEcalClusters(
    framework::Event& event, std::string inputClusterCollName,
    std::string inputClusterPassName) {
  const auto tmp_clusters = event.getCollection<ldmx::EcalCluster>(
      inputClusterCollName, inputClusterPassName);
  std::string new_name = inputClusterCollName + "Cast";
  std::vector<ldmx::CaloCluster> new_clusters;
  for (auto cl : tmp_clusters) {
    new_clusters.emplace_back(cl);
  }
  event.add(new_name, new_clusters);
  const auto calo_clusters =
      event.getCollection<ldmx::CaloCluster>(new_name, event.getPassName());
  return calo_clusters;
}

void ParticleFlow::onProcessEnd() { ldmx_log(debug) << "Process ends!"; }

}  // namespace recon

DECLARE_PRODUCER(recon::ParticleFlow);
