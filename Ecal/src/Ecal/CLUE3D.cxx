#include "Ecal/CLUE3D.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>

#include "DetDescr/EcalID.h"

namespace ecal {

namespace {

double dist(double x1, double y1, double x2, double y2) {
  return std::hypot(x1 - x2, y1 - y2);
}

// x,y of a log weighted centroid in a window around the max cell
void logCentroid(const std::vector<double>& x, const std::vector<double>& y,
                 const std::vector<double>& e, double w0, double window,
                 double& cx, double& cy) {
  double etot = std::accumulate(e.begin(), e.end(), 0.);
  auto imax = std::distance(e.begin(), std::max_element(e.begin(), e.end()));
  double sw{0}, sx{0}, sy{0}, lx{0}, ly{0};
  for (std::size_t i = 0; i < e.size(); i++) {
    lx += e[i] * x[i];
    ly += e[i] * y[i];
    if (e[i] <= 0 || dist(x[i], y[i], x[imax], y[imax]) > window) continue;
    double w = std::max(0., w0 + std::log(e[i] / etot));
    sw += w;
    sx += w * x[i];
    sy += w * y[i];
  }
  if (sw > 0) {
    cx = sx / sw;
    cy = sy / sw;
  } else {
    // linear fallback
    cx = lx / etot;
    cy = ly / etot;
  }
}

}  // namespace

double CLUE3D::lambda(int layer) const {
  if (layer >= 0 && layer < static_cast<int>(p_.profile_lambda.size()))
    return p_.profile_lambda[layer];
  // median radius / 1.68 from v15 gun showers
  return std::max(2., 1.9 * std::exp(0.12 * layer));
}

void CLUE3D::cluster2D(const std::vector<int>& idx) {
  const auto& hits = *hits_;
  const int n = idx.size();
  std::vector<double> rho(n), delta(n, std::numeric_limits<double>::max());
  std::vector<int> nh(n, -1);
  auto x = [&](int i) { return hits[idx[i]].getXPos(); };
  auto y = [&](int i) { return hits[idx[i]].getYPos(); };
  auto e = [&](int i) { return hits[idx[i]].getEnergy(); };

  // self plus weighted neighbors within dc
  for (int i = 0; i < n; i++) {
    rho[i] = e(i);
    for (int j = 0; j < n; j++) {
      if (j != i && dist(x(i), y(i), x(j), y(j)) < p_.dc2d)
        rho[i] += p_.kernel2d * e(j);
    }
  }
  // detid breaks density ties
  auto higher = [&](int a, int b) {
    if (rho[a] != rho[b]) return rho[a] > rho[b];
    return hits[idx[a]].getID() > hits[idx[b]].getID();
  };
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (j == i || !higher(j, i)) continue;
      double d = dist(x(i), y(i), x(j), y(j));
      if (d < p_.deltao2d && d < delta[i]) {
        delta[i] = d;
        nh[i] = j;
      }
    }
  }
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), higher);

  // orphans with no higher neighbor make their own cluster
  std::vector<int> lc_of(n, -1);
  const int first = lcs_.size();
  for (int i : order) {
    bool seed = nh[i] < 0 || (rho[i] >= p_.rhoc2d && delta[i] > p_.deltas2d);
    if (seed) {
      lc_of[i] = lcs_.size();
      lcs_.emplace_back();
    } else {
      lc_of[i] = lc_of[nh[i]];
    }
    lcs_[lc_of[i]].hits.push_back(idx[i]);
  }

  for (std::size_t c = first; c < lcs_.size(); c++) {
    auto& lc = lcs_[c];
    std::vector<double> hx, hy, he;
    for (int h : lc.hits) {
      hx.push_back(hits[h].getXPos());
      hy.push_back(hits[h].getYPos());
      he.push_back(hits[h].getEnergy());
      lc.e += hits[h].getEnergy();
    }
    logCentroid(hx, hy, he, p_.w0, p_.pos_window, lc.x, lc.y);
    lc.z = hits[lc.hits[0]].getZPos();
    lc.layer = layer_of_[lc.hits[0]];
  }
}

void CLUE3D::cluster3D() {
  const int m = lcs_.size();
  std::map<int, std::vector<int>> by_layer;
  for (int i = 0; i < m; i++) by_layer[lcs_[i].layer].push_back(i);

  auto loop_siblings = [&](int i, auto&& f) {
    int l = lcs_[i].layer;
    for (int dl = -p_.sibling_layers; dl <= p_.sibling_layers; dl++) {
      if (dl == 0) continue;
      auto it = by_layer.find(l + dl);
      if (it == by_layer.end()) continue;
      for (int j : it->second) f(j);
    }
  };

  for (int i = 0; i < m; i++) {
    auto& a = lcs_[i];
    a.rho = a.e;
    loop_siblings(i, [&](int j) {
      if (dist(a.x, a.y, lcs_[j].x, lcs_[j].y) < p_.dens_xy3d)
        a.rho += p_.kernel3d * lcs_[j].e;
    });
  }
  auto higher = [&](int a, int b) {
    if (lcs_[a].rho != lcs_[b].rho) return lcs_[a].rho > lcs_[b].rho;
    if (lcs_[a].e != lcs_[b].e) return lcs_[a].e > lcs_[b].e;
    return a > b;
  };
  const double max_d = p_.outlier_mult3d * p_.crit_xy3d;
  for (int i = 0; i < m; i++) {
    auto& a = lcs_[i];
    a.delta = std::numeric_limits<double>::max();
    a.nh = -1;
    loop_siblings(i, [&](int j) {
      if (!higher(j, i)) return;
      double d = dist(a.x, a.y, lcs_[j].x, lcs_[j].y);
      if (d < max_d && d < a.delta) {
        a.delta = d;
        a.nh = j;
      }
    });
  }
  std::vector<int> order(m);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), higher);
  int ncl = 0;
  for (int i : order) {
    auto& a = lcs_[i];
    bool seed = a.delta > p_.crit_xy3d && a.rho >= p_.crit_rho3d &&
                a.e / a.rho > p_.self_frac3d;
    if (seed)
      a.cid = ncl++;
    else
      a.cid = a.nh >= 0 ? lcs_[a.nh].cid : -1;
  }
}

void CLUE3D::fitAxis(Cluster& c, const std::vector<int>& idx,
                     const std::vector<double>& frac) const {
  const auto& hits = *hits_;
  std::map<int, std::vector<std::size_t>> by_layer;
  double etot{0}, sz{0};
  for (std::size_t k = 0; k < idx.size(); k++) {
    double e = frac[k] * hits[idx[k]].getEnergy();
    if (e <= 0) continue;
    by_layer[layer_of_[idx[k]]].push_back(k);
    etot += e;
    sz += e * hits[idx[k]].getZPos();
  }
  c.e = etot;
  if (etot <= 0) return;
  c.z = sz / etot;

  // axis from core hits only, if any
  bool any_core = false;
  for (const auto& [layer, ks] : by_layer)
    for (auto k : ks) any_core = any_core || core_[idx[k]];

  // weighted line fit of per layer centroids, weight E_L / lambda^2
  double s{0}, sz1{0}, szz{0}, sx{0}, sxz{0}, sy{0}, syz{0};
  for (const auto& [layer, ks] : by_layer) {
    std::vector<double> hx, hy, he;
    for (auto k : ks) {
      if (any_core && !core_[idx[k]]) continue;
      hx.push_back(hits[idx[k]].getXPos());
      hy.push_back(hits[idx[k]].getYPos());
      he.push_back(frac[k] * hits[idx[k]].getEnergy());
    }
    if (he.empty()) continue;
    double lx, ly;
    logCentroid(hx, hy, he, p_.w0, p_.pos_window, lx, ly);
    double el = std::accumulate(he.begin(), he.end(), 0.);
    double w = el / (lambda(layer) * lambda(layer));
    double z = hits[idx[ks[0]]].getZPos();
    s += w;
    sz1 += w * z;
    szz += w * z * z;
    sx += w * lx;
    sxz += w * lx * z;
    sy += w * ly;
    syz += w * ly * z;
  }
  double den = s * szz - sz1 * sz1;
  if (by_layer.size() > 1 && den > 1e-9 * s * szz) {
    c.dxdz = (s * sxz - sz1 * sx) / den;
    c.dydz = (s * syz - sz1 * sy) / den;
  } else {
    c.dxdz = c.dydz = 0;
  }
  double zbar = sz1 / s;
  c.x = sx / s + c.dxdz * (c.z - zbar);
  c.y = sy / s + c.dydz * (c.z - zbar);
}

std::vector<CLUE3D::Cluster> CLUE3D::cluster(
    const std::vector<ldmx::EcalHit>& hits) {
  hits_ = &hits;
  lcs_.clear();
  layer_of_.assign(hits.size(), 0);
  std::map<int, std::vector<int>> by_layer;
  for (std::size_t i = 0; i < hits.size(); i++) {
    if (hits[i].getEnergy() <= p_.hit_ecut || hits[i].getEnergy() <= 0)
      continue;
    layer_of_[i] = ldmx::EcalID(hits[i].getID()).layer();
    by_layer[layer_of_[i]].push_back(i);
  }
  for (const auto& [layer, idx] : by_layer) cluster2D(idx);
  cluster3D();

  // hits of 3D linked layer clusters define the axes
  core_.assign(hits.size(), 0);
  for (const auto& lc : lcs_)
    if (lc.cid >= 0)
      for (int h : lc.hits) core_[h] = 1;

  // hit members of the 3D clusters
  int ncl = 0;
  for (const auto& lc : lcs_) ncl = std::max(ncl, lc.cid + 1);
  std::vector<std::vector<int>> members(ncl);
  for (const auto& lc : lcs_)
    if (lc.cid >= 0)
      members[lc.cid].insert(members[lc.cid].end(), lc.hits.begin(),
                             lc.hits.end());
  auto fit_all = [&](std::vector<Cluster>& cls,
                     const std::vector<std::vector<int>>& mem) {
    cls.assign(mem.size(), Cluster{});
    for (std::size_t k = 0; k < mem.size(); k++)
      fitAxis(cls[k], mem[k], std::vector<double>(mem[k].size(), 1.));
  };
  std::vector<Cluster> cls;
  fit_all(cls, members);

  // attach orphan layer clusters to the nearest axis, else keep alone
  for (const auto& lc : lcs_) {
    if (lc.cid >= 0) continue;
    int best = -1;
    double best_d = p_.attach_radius;
    for (std::size_t k = 0; k < cls.size(); k++) {
      double d = dist(lc.x, lc.y, cls[k].axisX(lc.z), cls[k].axisY(lc.z));
      if (d < best_d) {
        best_d = d;
        best = k;
      }
    }
    if (best >= 0) {
      members[best].insert(members[best].end(), lc.hits.begin(), lc.hits.end());
    } else {
      members.push_back(lc.hits);
      for (int h : lc.hits) core_[h] = 1;
    }
  }
  fit_all(cls, members);

  // merge satellites into a much larger nearby cluster
  {
    std::vector<int> order(cls.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(),
              [&](int a, int b) { return cls[a].e < cls[b].e; });
    std::vector<double> esum(cls.size());
    std::vector<int> nlay(cls.size());
    for (std::size_t k = 0; k < cls.size(); k++) {
      esum[k] = cls[k].e;
      std::vector<int> ls;
      for (int h : members[k]) ls.push_back(layer_of_[h]);
      std::sort(ls.begin(), ls.end());
      nlay[k] = std::unique(ls.begin(), ls.end()) - ls.begin();
    }
    for (int sat : order) {
      int best = -1;
      double best_d = p_.sc_radius;
      bool short_sat = nlay[sat] < p_.sc_min_layers;
      for (std::size_t k = 0; k < cls.size(); k++) {
        // only merge into clusters that are not short themselves
        if (static_cast<int>(k) == sat || members[k].empty() ||
            esum[k] <= esum[sat] || nlay[k] < p_.sc_min_layers ||
            (!short_sat && esum[sat] >= p_.sc_ratio * esum[k]))
          continue;
        double d = dist(cls[sat].x, cls[sat].y, cls[k].axisX(cls[sat].z),
                        cls[k].axisY(cls[sat].z));
        if (d < best_d) {
          best_d = d;
          best = k;
        }
      }
      if (best < 0) continue;
      members[best].insert(members[best].end(), members[sat].begin(),
                           members[sat].end());
      for (int h : members[sat]) core_[h] = 0;
      esum[best] += esum[sat];
      members[sat].clear();
    }
    std::vector<std::vector<int>> left;
    for (auto& m : members)
      if (!m.empty()) left.push_back(std::move(m));
    members = std::move(left);
    fit_all(cls, members);
  }

  // drop tiny clusters
  std::vector<std::vector<int>> kept;
  for (std::size_t k = 0; k < cls.size(); k++)
    if (cls[k].e >= p_.min_cluster_energy) kept.push_back(members[k]);
  members = std::move(kept);
  fit_all(cls, members);
  std::vector<std::vector<double>> fracs(members.size());
  for (std::size_t k = 0; k < members.size(); k++)
    fracs[k].assign(members[k].size(), 1.);

  // share hits between large clusters with an exponential profile
  std::vector<int> big;
  for (std::size_t k = 0; k < cls.size(); k++)
    if (cls[k].e >= p_.share_min_energy) big.push_back(k);
  if (p_.share && big.size() > 1) {
    std::vector<int> shared_hits;
    for (int k : big)
      shared_hits.insert(shared_hits.end(), members[k].begin(),
                         members[k].end());
    std::vector<std::vector<int>> mem(big.size());
    std::vector<std::vector<double>> fr(big.size());
    for (int it = 0; it < p_.share_iters; it++) {
      for (auto& v : mem) v.clear();
      for (auto& v : fr) v.clear();
      for (int h : shared_hits) {
        double hz = hits[h].getZPos();
        double lam = lambda(layer_of_[h]);
        std::vector<double> w(big.size());
        double sw{0};
        for (std::size_t b = 0; b < big.size(); b++) {
          const auto& c = cls[big[b]];
          double d = dist(hits[h].getXPos(), hits[h].getYPos(), c.axisX(hz),
                          c.axisY(hz));
          w[b] = c.e * std::exp(-d / lam);
          sw += w[b];
        }
        if (sw <= 0) {
          // far from all axes: keep with the closest
          std::size_t bb = 0;
          double dmin = std::numeric_limits<double>::max();
          for (std::size_t b = 0; b < big.size(); b++) {
            const auto& c = cls[big[b]];
            double d = dist(hits[h].getXPos(), hits[h].getYPos(), c.axisX(hz),
                            c.axisY(hz));
            if (d < dmin) {
              dmin = d;
              bb = b;
            }
          }
          w.assign(big.size(), 0.);
          w[bb] = sw = 1.;
        }
        for (std::size_t b = 0; b < big.size(); b++) {
          double f = w[b] / sw;
          if (f < 1e-4) continue;
          mem[b].push_back(h);
          fr[b].push_back(f);
        }
      }
      for (std::size_t b = 0; b < big.size(); b++) {
        fitAxis(cls[big[b]], mem[b], fr[b]);
      }
    }
    for (std::size_t b = 0; b < big.size(); b++) {
      members[big[b]] = mem[b];
      fracs[big[b]] = fr[b];
    }
    // merge clusters that converged onto the same axis
    for (std::size_t i = 0; i < big.size(); i++) {
      for (std::size_t j = i + 1; j < big.size(); j++) {
        auto& a = cls[big[i]];
        auto& b = cls[big[j]];
        if (members[big[i]].empty() || members[big[j]].empty()) continue;
        double zz = a.e < b.e ? a.z : b.z;
        if (dist(a.axisX(zz), a.axisY(zz), b.axisX(zz), b.axisY(zz)) >
            p_.coincide_dist)
          continue;
        std::map<int, double> f;
        for (std::size_t k = 0; k < members[big[i]].size(); k++)
          f[members[big[i]][k]] += fracs[big[i]][k];
        for (std::size_t k = 0; k < members[big[j]].size(); k++)
          f[members[big[j]][k]] += fracs[big[j]][k];
        members[big[i]].clear();
        fracs[big[i]].clear();
        for (auto& [h, v] : f) {
          members[big[i]].push_back(h);
          fracs[big[i]].push_back(v);
        }
        members[big[j]].clear();
        fracs[big[j]].clear();
        fitAxis(a, members[big[i]], fracs[big[i]]);
      }
    }
  }

  // final quantities
  std::vector<Cluster> out;
  for (std::size_t k = 0; k < members.size(); k++) {
    Cluster c;
    fitAxis(c, members[k], fracs[k]);
    if (c.e < p_.min_cluster_energy) continue;
    c.hits = members[k];
    c.frac = fracs[k];
    double sxx{0}, syy{0}, szz{0}, sx{0}, sy{0}, e0{0};
    for (std::size_t i = 0; i < c.hits.size(); i++) {
      const auto& h = hits[c.hits[i]];
      double e = c.frac[i] * h.getEnergy();
      sx += e * h.getXPos();
      sy += e * h.getYPos();
      sxx += e * h.getXPos() * h.getXPos();
      syy += e * h.getYPos() * h.getYPos();
      szz += e * (h.getZPos() - c.z) * (h.getZPos() - c.z);
      if (layer_of_[c.hits[i]] == 0) {
        c.x0 += e * h.getXPos();
        c.y0 += e * h.getYPos();
        c.z0 = h.getZPos();
        e0 += e;
      }
    }
    sx /= c.e;
    sy /= c.e;
    c.rms_x = std::sqrt(std::max(0., sxx / c.e - sx * sx));
    c.rms_y = std::sqrt(std::max(0., syy / c.e - sy * sy));
    c.rms_z = std::sqrt(szz / c.e);
    if (e0 > 0) {
      c.x0 /= e0;
      c.y0 /= e0;
    }
    out.push_back(std::move(c));
  }
  std::sort(out.begin(), out.end(),
            [](const Cluster& a, const Cluster& b) { return a.e > b.e; });
  return out;
}

}  // namespace ecal
