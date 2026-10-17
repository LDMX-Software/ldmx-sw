#include "Recon/ParticleFlow.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "Ecal/Event/EcalCluster.h"
#include "Framework/Exception/Exception.h"
#include "Recon/Event/TrackDeDxMassEstimate.h"

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
  input_mass_coll_name_ = ps.get<std::string>("input_mass_coll_name");
  input_mass_passname_ = ps.get<std::string>("input_mass_passname");
  link_max_dist_ = ps.get<double>("link_max_dist");
  electron_ep_min_ = ps.get<double>("electron_ep_min");
  link_min_rms_ = ps.get<double>("link_min_rms");
  excess_ep_ = ps.get<double>("excess_ep");
  proton_min_mass2_ = ps.get<double>("proton_min_mass2");
  kaon_min_mass2_ = ps.get<double>("kaon_min_mass2");
  proton_max_p_ = ps.get<double>("proton_max_p");
  kaon_max_p_ = ps.get<double>("kaon_max_p");
  recovery_radius_ = ps.get<double>("recovery_radius");
  hcal_merge_angle_ = ps.get<double>("hcal_merge_angle");
  hcal_track_angle_ = ps.get<double>("hcal_track_angle");
  hadron_ecal_radius_ = ps.get<double>("hadron_ecal_radius");
  hadron_absorb_ep_ = ps.get<double>("hadron_absorb_ep");
  min_neutral_e_ = ps.get<double>("min_neutral_e");
  ecal_front_z_ = ps.get<double>("ecal_front_z");
  electron_max_depth_ = ps.get<double>("electron_max_depth");
  electron_depth_slope_ = ps.get<double>("electron_depth_slope");
  recovery_stop_ep_ = ps.get<double>("recovery_stop_ep");
  ecal_link_radius_ = ps.get<double>("ecal_link_radius");
  ecal_link_ratio_ = ps.get<double>("ecal_link_ratio");
  ecal_link_min_layers_ = ps.get<int>("ecal_link_min_layers");
  photon_keep_min_e_ = ps.get<double>("photon_keep_min_e");
  photon_keep_min_dist_ = ps.get<double>("photon_keep_min_dist");
  photon_keep_max_zstart_ = ps.get<double>("photon_keep_max_zstart");
  dedx_min_hits_ = ps.get<int>("dedx_min_hits");

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
  cand.setTruthTrackId(best->getTrackID());
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
  cand.setEcalDepth(ecalDepth(em));
  cand.clearEcalHits();
  cand.addEcalHits(em.getHitIDs(), em.getHitE());
  cand.setPID(cand.getPID() | 2);  // OR with 010
}
// produce candidate HCal info
void ParticleFlow::fillCandHadCalo(ldmx::PFCandidate& cand,
                                   const ldmx::CaloCluster& had) {
  float energy = had.getEnergy();
  float corr = correction(hcal_corr_energies_, hcal_corr_factors_, energy);
  cand.setHcalEnergy(energy * corr);
  cand.setHcalRawEnergy(energy);
  cand.clearHcalHits();
  cand.addHcalHits(had.getHitIDs(), had.getHitE());
  cand.setHcalClusterXYZ(had.getCentroidX(), had.getCentroidY(),
                         had.getCentroidZ());
  cand.setHcalClusterEXYZ(had.getRMSX(), had.getRMSY(), had.getRMSZ());
  cand.setHcalClusterDXDZ(had.getDXDZ());
  cand.setHcalClusterDYDZ(had.getDYDZ());
  cand.setHcalClusterEDXDZ(had.getEDXDZ());
  cand.setHcalClusterEDYDZ(had.getEDYDZ());
  cand.setPID(cand.getPID() | 4);  // OR with 100
}

// RMS-normalized distance of the track to the cluster centroid
double ParticleFlow::trackClusterDist(const std::vector<double>& xyz,
                                      const std::vector<double>& pxyz,
                                      const ldmx::CaloCluster& cl) const {
  double z = cl.getCentroidZ();
  double x = xyz[0] + pxyz[0] / pxyz[2] * (z - xyz[2]);
  double y = xyz[1] + pxyz[1] / pxyz[2] * (z - xyz[2]);
  // RMS floor covers scattering of soft tracks
  return std::hypot(
      (x - cl.getCentroidX()) / std::max(link_min_rms_, cl.getRMSX()),
      (y - cl.getCentroidY()) / std::max(link_min_rms_, cl.getRMSY()));
}

// Ecal to Ecal linking: merge satellite clusters into their parent shower.
// A satellite is short (few layers) or a small fraction of a nearby long
// cluster. Clusters a track points to are never absorbed, and soft clusters
// that are isolated and start early stay separate photons.
std::vector<ldmx::CaloCluster> ParticleFlow::linkEcalClusters(
    const std::vector<ldmx::CaloCluster>& clusters,
    const std::vector<ldmx::Track>& tracks) const {
  int n = clusters.size();
  std::vector<int> nlay(n), parent(n, -1);
  std::vector<double> zstart(n);
  std::vector<bool> tk_linked(n, false);
  for (int i = 0; i < n; i++) {
    std::set<int> layers;
    double zmin = 9e9;
    for (float z : clusters[i].getHitZ()) {
      layers.insert(std::lround(z));
      zmin = std::min<double>(zmin, z);
    }
    nlay[i] = layers.size();
    zstart[i] = layers.empty() ? clusters[i].getCentroidZ() : zmin;
    for (const auto& tk : tracks) {
      auto ts = tk.getTrackState(ldmx::AtECAL);
      if (!ts || ts->mom_.size() < 3 || ts->mom_[2] <= 0) continue;
      if (trackClusterDist(ts->pos_, ts->mom_, clusters[i]) < link_max_dist_) {
        tk_linked[i] = true;
        break;
      }
    }
  }
  for (int s = 0; s < n; s++) {
    if (tk_linked[s]) continue;
    double es = clusters[s].getEnergy();
    int best = -1;
    double best_d = ecal_link_radius_;
    double nearest = 9e9;  // to any more energetic cluster
    for (int p = 0; p < n; p++) {
      if (p == s || clusters[p].getEnergy() <= es) continue;
      double d =
          std::hypot(clusters[p].getCentroidX() - clusters[s].getCentroidX(),
                     clusters[p].getCentroidY() - clusters[s].getCentroidY());
      nearest = std::min(nearest, d);
      bool satellite = nlay[s] < ecal_link_min_layers_ ||
                       es < ecal_link_ratio_ * clusters[p].getEnergy();
      if (nlay[p] >= ecal_link_min_layers_ && satellite && d < best_d) {
        best_d = d;
        best = p;
      }
    }
    // isolated, early showers look like photons from the target
    bool photon_like = es >= photon_keep_min_e_ &&
                       nearest > photon_keep_min_dist_ &&
                       zstart[s] < photon_keep_max_zstart_;
    if (best >= 0 && !photon_like) parent[s] = best;
  }

  // follow chains to the final parent and merge
  auto root = [&parent](int i) {
    while (parent[i] >= 0) i = parent[i];
    return i;
  };
  std::map<int, std::vector<int>> groups;
  for (int i = 0; i < n; i++) groups[root(i)].push_back(i);
  std::vector<ldmx::CaloCluster> merged;
  for (const auto& [r, members] : groups) {
    merged.push_back(mergeClusters(clusters, r, members));
  }
  std::sort(merged.begin(), merged.end(), [](const auto& a, const auto& b) {
    return a.getEnergy() > b.getEnergy();
  });
  return merged;
}

// sum of member clusters: energy, energy-weighted centroid, all hits;
// shape info is kept from the root cluster
ldmx::CaloCluster ParticleFlow::mergeClusters(
    const std::vector<ldmx::CaloCluster>& clusters, int root,
    const std::vector<int>& members) {
  if (members.size() == 1) return clusters[root];
  ldmx::CaloCluster cl = clusters[root];
  double e = 0, x = 0, y = 0, z = 0;
  std::vector<unsigned int> ids;
  std::vector<float> hx, hy, hz, he;
  for (int m : members) {
    const auto& c = clusters[m];
    e += c.getEnergy();
    x += c.getEnergy() * c.getCentroidX();
    y += c.getEnergy() * c.getCentroidY();
    z += c.getEnergy() * c.getCentroidZ();
    ids.insert(ids.end(), c.getHitIDs().begin(), c.getHitIDs().end());
    hx.insert(hx.end(), c.getHitX().begin(), c.getHitX().end());
    hy.insert(hy.end(), c.getHitY().begin(), c.getHitY().end());
    hz.insert(hz.end(), c.getHitZ().begin(), c.getHitZ().end());
    he.insert(he.end(), c.getHitE().begin(), c.getHitE().end());
  }
  cl.setEnergy(e);
  if (e > 0) cl.setCentroidXYZ(x / e, y / e, z / e);
  cl.setNHits(ids.size());
  cl.setIDs(ids);
  cl.setHitValsX(hx);
  cl.setHitValsY(hy);
  cl.setHitValsZ(hz);
  cl.setHitValsE(he);
  return cl;
}

// Hcal to Hcal linking: hadron showers fragment into many clusters, merge
// those within a cone seen from the target around the most energetic one
std::vector<ldmx::CaloCluster> ParticleFlow::linkHcalClusters(
    const std::vector<ldmx::CaloCluster>& clusters) const {
  int n = clusters.size();
  std::vector<int> order(n);
  for (int i = 0; i < n; i++) order[i] = i;
  std::sort(order.begin(), order.end(), [&clusters](int a, int b) {
    return clusters[a].getEnergy() > clusters[b].getEnergy();
  });
  auto dir = [&clusters](int i) {
    const auto& c = clusters[i];
    double r = std::hypot(c.getCentroidX(), c.getCentroidY(), c.getCentroidZ());
    return std::array<double, 3>{c.getCentroidX() / r, c.getCentroidY() / r,
                                 c.getCentroidZ() / r};
  };
  double cos_max = std::cos(hcal_merge_angle_ * M_PI / 180.);
  std::vector<bool> used(n, false);
  std::vector<ldmx::CaloCluster> merged;
  for (int s : order) {
    if (used[s]) continue;
    auto ds = dir(s);
    std::vector<int> members;
    for (int o : order) {
      if (used[o]) continue;
      auto d = dir(o);
      if (ds[0] * d[0] + ds[1] * d[1] + ds[2] * d[2] >= cos_max) {
        used[o] = true;
        members.push_back(o);
      }
    }
    merged.push_back(mergeClusters(clusters, s, members));
  }
  return merged;
}

// free Hcal cluster closest in angle to the track direction, seen from the
// track at the Ecal face; -1 if none within hcal_track_angle
int ParticleFlow::linkTrackHcal(const ldmx::Track& tk,
                                const std::vector<ldmx::CaloCluster>& clusters,
                                const std::vector<bool>& used) const {
  auto ts = tk.getTrackState(ldmx::AtECAL);
  if (hcal_track_angle_ <= 0 || !ts || ts->mom_.size() < 3) return -1;
  const auto& pos = ts->pos_;
  const auto& mom = ts->mom_;
  double pm = std::hypot(mom[0], mom[1], mom[2]);
  int best = -1;
  double best_cos = std::cos(hcal_track_angle_ * M_PI / 180.);
  for (int j = 0; j < static_cast<int>(clusters.size()); j++) {
    if (used[j]) continue;
    double dx = clusters[j].getCentroidX() - pos[0];
    double dy = clusters[j].getCentroidY() - pos[1];
    double dz = clusters[j].getCentroidZ() - pos[2];
    double c = (dx * mom[0] + dy * mom[1] + dz * mom[2]) /
               (pm * std::hypot(dx, dy, dz));
    if (c > best_cos) {
      best_cos = c;
      best = j;
    }
  }
  return best;
}

// energy subtraction: a charged hadron absorbs free Ecal clusters near its
// track and free Hcal clusters in its cone, closest first, while the
// calibrated calorimeter energy stays below hadron_absorb_ep * p; what is
// left stays neutral
void ParticleFlow::absorbHadronShower(
    ldmx::PFCandidate& cand, const ldmx::Track& tk,
    const std::vector<ldmx::CaloCluster>& ecal, std::vector<bool>& ecal_used,
    const std::vector<ldmx::CaloCluster>& hcal,
    std::vector<bool>& hcal_used) const {
  auto ts = tk.getTrackState(ldmx::AtECAL);
  if (hadron_absorb_ep_ <= 0 || !ts || ts->mom_.size() < 3 || ts->mom_[2] <= 0)
    return;
  const auto& pos = ts->pos_;
  const auto& mom = ts->mom_;
  double pm = std::hypot(mom[0], mom[1], mom[2]);
  double budget = hadron_absorb_ep_ * pm;
  double calo = cand.getEcalEnergy() + cand.getHcalEnergy();

  // (distance key, is_hcal, index), Ecal in mm and Hcal in angle scaled to mm
  std::vector<std::tuple<double, bool, int>> near;
  for (int j = 0; j < static_cast<int>(ecal.size()); j++) {
    if (ecal_used[j]) continue;
    double z = ecal[j].getCentroidZ();
    double d = std::hypot(
        pos[0] + mom[0] / mom[2] * (z - pos[2]) - ecal[j].getCentroidX(),
        pos[1] + mom[1] / mom[2] * (z - pos[2]) - ecal[j].getCentroidY());
    if (d < hadron_ecal_radius_) near.emplace_back(d, false, j);
  }
  double cos_max = std::cos(hcal_track_angle_ * M_PI / 180.);
  for (int j = 0; j < static_cast<int>(hcal.size()); j++) {
    if (hcal_used[j]) continue;
    double dx = hcal[j].getCentroidX() - pos[0];
    double dy = hcal[j].getCentroidY() - pos[1];
    double dz = hcal[j].getCentroidZ() - pos[2];
    double cs = (dx * mom[0] + dy * mom[1] + dz * mom[2]) /
                (pm * std::hypot(dx, dy, dz));
    if (cs > cos_max) {
      // rank Hcal after nearby Ecal pieces, by angle
      double ang = std::acos(std::min(1., cs)) * 180. / M_PI;
      near.emplace_back(hadron_ecal_radius_ + ang, true, j);
    }
  }
  std::sort(near.begin(), near.end());
  for (const auto& [d, is_hcal, j] : near) {
    const auto& cl = is_hcal ? hcal[j] : ecal[j];
    double raw = cl.getEnergy();
    double e =
        raw * (is_hcal
                   ? correction(hcal_corr_energies_, hcal_corr_factors_, raw)
                   : correction(ecal_corr_energies_, ecal_corr_factors_, raw));
    if (calo + e > budget) continue;
    calo += e;
    if (is_hcal) {
      cand.setHcalEnergy(cand.getHcalEnergy() + e);
      cand.setHcalRawEnergy(cand.getHcalRawEnergy() + raw);
      cand.addHcalHits(cl.getHitIDs(), cl.getHitE());
      cand.setPID(cand.getPID() | 4);
      hcal_used[j] = true;
    } else {
      cand.setEcalEnergy(cand.getEcalEnergy() + e);
      cand.setEcalRawEnergy(cand.getEcalRawEnergy() + raw);
      cand.addEcalHits(cl.getHitIDs(), cl.getHitE());
      ecal_used[j] = true;
    }
  }
}

// energy-weighted RMS distance of the cluster hits from the straight track
// line through the Ecal; EM showers are narrow, hadronic ones wide [mm]
double ParticleFlow::ecalWidth(const ldmx::CaloCluster& cl,
                               const ldmx::Track& tk) const {
  auto ts = tk.getTrackState(ldmx::AtECAL);
  if (!ts || ts->mom_.size() < 3 || ts->mom_[2] <= 0) return 0.;
  const auto& pos = ts->pos_;
  const auto& mom = ts->mom_;
  const auto& hx = cl.getHitX();
  const auto& hy = cl.getHitY();
  const auto& hz = cl.getHitZ();
  const auto& he = cl.getHitE();
  double e = 0, er2 = 0;
  for (size_t k = 0; k < he.size() && k < hz.size(); k++) {
    double x = pos[0] + mom[0] / mom[2] * (hz[k] - pos[2]);
    double y = pos[1] + mom[1] / mom[2] * (hz[k] - pos[2]);
    e += he[k];
    er2 += he[k] * (std::pow(hx[k] - x, 2) + std::pow(hy[k] - y, 2));
  }
  return e > 0 ? std::sqrt(er2 / e) : 0.;
}

// energy-weighted mean hit depth behind the Ecal front [mm]
double ParticleFlow::ecalDepth(const ldmx::CaloCluster& cl) const {
  const auto& hz = cl.getHitZ();
  const auto& he = cl.getHitE();
  double e = 0, ez = 0;
  for (size_t k = 0; k < hz.size() && k < he.size(); k++) {
    e += he[k];
    ez += he[k] * hz[k];
  }
  return (e > 0 ? ez / e : cl.getCentroidZ()) - ecal_front_z_;
}

// a linked track with low E/p absorbs nearby unlinked clusters, closest
// first, to recover electron showers split by clustering
void ParticleFlow::recoverEcal(ldmx::PFCandidate& cand, const ldmx::Track& tk,
                               const std::vector<ldmx::CaloCluster>& clusters,
                               std::vector<bool>& used) const {
  auto ts = tk.getTrackState(ldmx::AtECAL);
  if (recovery_radius_ <= 0 || !ts || ts->mom_.size() < 3 || ts->mom_[2] <= 0)
    return;
  const auto& xyz = ts->pos_;
  const auto& pxyz = ts->mom_;
  double trk_p = std::hypot(pxyz[0], pxyz[1], pxyz[2]);
  if (cand.getEcalEnergy() >= electron_ep_min_ * trk_p) return;
  std::vector<std::pair<double, int>> near;
  for (int j = 0; j < static_cast<int>(clusters.size()); j++) {
    if (used[j]) continue;
    double z = clusters[j].getCentroidZ();
    double d = std::hypot(
        xyz[0] + pxyz[0] / pxyz[2] * (z - xyz[2]) - clusters[j].getCentroidX(),
        xyz[1] + pxyz[1] / pxyz[2] * (z - xyz[2]) - clusters[j].getCentroidY());
    if (d < recovery_radius_) near.emplace_back(d, j);
  }
  std::sort(near.begin(), near.end());
  for (const auto& [d, j] : near) {
    if (cand.getEcalEnergy() >= recovery_stop_ep_ * trk_p) break;
    double e = clusters[j].getEnergy();
    double corr = correction(ecal_corr_energies_, ecal_corr_factors_, e);
    cand.setEcalEnergy(cand.getEcalEnergy() + e * corr);
    cand.setEcalRawEnergy(cand.getEcalRawEnergy() + e);
    cand.addEcalHits(clusters[j].getHitIDs(), clusters[j].getHitE());
    used[j] = true;
  }
}

// charged ID, types as in CMS PF: electron from E/p and shower depth, else
// charged hadron (pdg 211 by convention); dE/dx at low p only sets the
// hadron mass hypothesis (p, K or pi), used for the energy
void ParticleFlow::identifyCharged(ldmx::PFCandidate& cand) const {
  auto p = cand.getTrackPxPyPz();
  double trk_p = std::hypot(p[0], p[1], p[2]);
  double ep = trk_p > 0 ? cand.getEcalEnergy() / trk_p : 0;
  int q = cand.getCharge() >= 0 ? 1 : -1;
  // m2 resolution grows as p^2, so dE/dx only separates at low p
  bool has_dedx = cand.getDeDxNhits() >= dedx_min_hits_;
  double m2 = cand.getDeDxMassSquared();
  bool is_proton = has_dedx && trk_p < proton_max_p_ && m2 > proton_min_mass2_;
  bool is_kaon = has_dedx && trk_p < kaon_max_p_ && m2 > kaon_min_mass2_ &&
                 m2 <= proton_min_mass2_;
  if (is_proton) {
    cand.setPdgId(211 * q);
    cand.setMass(938.27);
  } else if (is_kaon) {
    cand.setPdgId(211 * q);
    cand.setMass(493.68);
  } else if ((cand.getPID() & 2) && ep > electron_ep_min_ &&
             cand.getEcalDepth() <
                 electron_max_depth_ +
                     electron_depth_slope_ *
                         std::log2(std::max(trk_p, 100.) / 1000.)) {
    // EM showers stay shallow, hadronic ones start late and go deep
    // no upper E/p, overlapping photons are split off as excess
    cand.setPdgId(-11 * q);
    cand.setMass(0.511);
  } else {
    cand.setPdgId(211 * q);
    cand.setMass(139.57);
  }
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
  auto hcal_clusters = event.getCollection<ldmx::CaloCluster>(
      input_hcal_coll_name_, input_hcal_passname_);
  if (hcal_merge_angle_ > 0) hcal_clusters = linkHcalClusters(hcal_clusters);
  const auto tracks = event.getCollection<ldmx::Track>(input_track_coll_name_,
                                                       input_tracks_passname_);
  // truth info is optional
  std::vector<ldmx::SimTrackerHit> sp_hits;
  if (event.exists(truth_sp_coll_name_, truth_sp_passname_))
    sp_hits = event.getCollection<ldmx::SimTrackerHit>(truth_sp_coll_name_,
                                                       truth_sp_passname_);
  // dE/dx estimates made on the input tracks, keyed by track index
  std::map<int, ldmx::TrackDeDxMassEstimate> mass_estimates;
  if (!input_mass_coll_name_.empty() &&
      event.exists(input_mass_coll_name_, input_mass_passname_)) {
    for (const auto& est : event.getCollection<ldmx::TrackDeDxMassEstimate>(
             input_mass_coll_name_, input_mass_passname_)) {
      mass_estimates[est.getTrackIndex()] = est;
    }
  }
  // here allow for using existing clusters of different type (EcalCluster)
  auto ecal_clusters =
      use_existing_ecal_clusters_
          ? getEcalClusters(event, input_ecal_coll_name_, input_ecal_passname_)
          : event.getCollection<ldmx::CaloCluster>(input_ecal_coll_name_,
                                                   input_ecal_passname_);
  if (ecal_link_radius_ > 0)
    ecal_clusters = linkEcalClusters(ecal_clusters, tracks);

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
    // linked Ecal clusters per track, closest first
    std::map<int, std::vector<std::pair<float, int>>> tk_calo_map;
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
      for (int j = 0; j < ecal_clusters.size(); j++) {
        float dist = trackClusterDist(xyz, pxyz, ecal_clusters[j]);
        // E/p is used for the ID, not the link
        if (dist < link_max_dist_) tk_calo_map[i].emplace_back(dist, j);
      }
      std::sort(tk_calo_map[i].begin(), tk_calo_map[i].end());
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

    //
    // track / ecal cluster arbitration
    //
    std::vector<bool> tk_is_em_linked(tracks.size(), false);
    std::vector<bool> em_is_tk_linked(ecal_clusters.size(), false);
    std::map<int, int> tk_em_pairs{};
    for (int i = 0; i < tracks.size(); i++) {
      if (tk_calo_map.count(i)) {
        // pick the closest unused matching cluster
        for (const auto& [dist, em_idx] : tk_calo_map[i]) {
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
        cand.setEcalWidth(ecalWidth(ecal_clusters[em_idx], tracks[i]));
        cand.setEcalIndex(em_idx);
        if (em_is_had_linked[em_idx]) {  // if ECal is linked with HCal
          fillCandHadCalo(cand, hcal_clusters[em_had_pairs[em_idx]]);
          cand.setHcalIndex(em_had_pairs[em_idx]);
        }
        recoverEcal(cand, tracks[i], ecal_clusters, em_is_tk_linked);
      }
      // charged hadron showers in the Hcal belong to the track, not to a
      // neutral; takes the closest free cluster in angle
      if (cand.getHcalIndex() < 0) {
        int h = linkTrackHcal(tracks[i], hcal_clusters, had_is_em_linked);
        if (h >= 0) {
          fillCandHadCalo(cand, hcal_clusters[h]);
          cand.setHcalIndex(h);
          had_is_em_linked[h] = true;
        }
      }
      if (auto it = mass_estimates.find(i); it != mass_estimates.end()) {
        cand.setDeDxMassSquared(it->second.getMassSquared());
        cand.setDeDxIh(it->second.getIh());
        cand.setDeDxNhits(it->second.getNhits());
      }
      cand.setCharge(static_cast<int>(tracks[i].getCharge()));
      identifyCharged(cand);
      if (std::abs(cand.getPdgId()) == 211) {
        absorbHadronShower(cand, tracks[i], ecal_clusters, em_is_tk_linked,
                           hcal_clusters, had_is_em_linked);
      }
      // charged energy from the track and the mass hypothesis
      auto p = cand.getTrackPxPyPz();
      double trk_p = std::hypot(p[0], p[1], p[2]);
      cand.setEnergy(std::hypot(trk_p, cand.getMass()));
      pf_cands.push_back(cand);

      // Ecal energy well above the track momentum is a neutral overlap
      double ecal_e = cand.getEcalEnergy();
      if (tk_is_em_linked[i] && trk_p > 0 && ecal_e > excess_ep_ * trk_p) {
        ldmx::PFCandidate excess;
        fillCandEMCalo(excess, ecal_clusters[tk_em_pairs[i]]);
        excess.setEcalIndex(tk_em_pairs[i]);
        excess.setPdgId(22);
        // hits stay with the charged candidate, one candidate per hit
        excess.clearEcalHits();
        excess.setEnergy(ecal_e - trk_p);
        pf_cands.push_back(excess);
      }
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
      // Hcal energy means a hadronic shower
      // pdg 130 for neutral hadrons, as in CMS PF
      cand.setPdgId(em_is_had_linked[i] ? 130 : 22);
      cand.setEnergy(cand.getEcalEnergy() + cand.getHcalEnergy());
      if (cand.getEnergy() >= min_neutral_e_) pf_cands.push_back(cand);
    }

    // neutral candidates from unlinked Hcal clusters
    for (int i = 0; i < hcal_clusters.size(); i++) {
      if (had_is_em_linked[i]) continue;
      ldmx::PFCandidate cand;
      fillCandHadCalo(cand, hcal_clusters[i]);
      cand.setHcalIndex(i);
      cand.setPdgId(130);
      cand.setEnergy(cand.getHcalEnergy());
      if (cand.getEnergy() >= min_neutral_e_) pf_cands.push_back(cand);
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
