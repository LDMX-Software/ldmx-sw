#include "Tracking/Digitization/StripClusterer.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <map>
#include <set>

namespace tracking::digitization {

StripClusterer::StripClusterer(double seed_threshold, double neighbor_threshold,
                               double cluster_threshold, double noise_sigma_adc,
                               double mean_time_ns, double time_window_ns,
                               double neighbor_delta_t_ns, double max_chi2_ndf)
    : seed_threshold_(seed_threshold),
      neighbor_threshold_(neighbor_threshold),
      cluster_threshold_(cluster_threshold),
      noise_sigma_adc_(noise_sigma_adc),
      mean_time_ns_(mean_time_ns),
      time_window_ns_(time_window_ns),
      neighbor_delta_t_ns_(neighbor_delta_t_ns),
      max_chi2_ndf_(max_chi2_ndf) {}

// ---------------------------------------------------------------------------

bool StripClusterer::passesSeedCuts(const ldmx::FittedSiStripHit& h) const {
  // Timing window (disabled if time_window_ns_ <= 0)
  if (time_window_ns_ > 0.0) {
    if (std::abs(h.getT0() - mean_time_ns_) > time_window_ns_) return false;
  }
  // Chi2/ndf quality cut (disabled if max_chi2_ndf_ <= 0)
  if (max_chi2_ndf_ > 0.0 && h.getNDF() > 0) {
    if (h.getReducedChi2() > max_chi2_ndf_) return false;
  }
  return true;
}

bool StripClusterer::passesNeighborCuts(const ldmx::FittedSiStripHit& h,
                                        double cluster_weighted_t,
                                        double cluster_total_amp) const {
  if (neighbor_delta_t_ns_ > 0.0 && cluster_total_amp > 0.0) {
    const double cluster_t = cluster_weighted_t / cluster_total_amp;
    if (std::abs(h.getT0() - cluster_t) > neighbor_delta_t_ns_) return false;
  }
  return true;
}

// ---------------------------------------------------------------------------

std::vector<StripClusterer::ClusterCandidate> StripClusterer::findClusters(
    const std::vector<ldmx::FittedSiStripHit>& hits) const {
  // -------------------------------------------------------------------------
  // Build channel → hit map.
  // If two hits land on the same strip, keep the one with smaller |t0|.
  // -------------------------------------------------------------------------
  std::map<int, const ldmx::FittedSiStripHit*> channel_map;
  for (const auto& h : hits) {
    const int ch = h.getStripID();
    auto it = channel_map.find(ch);
    if (it == channel_map.end()) {
      channel_map[ch] = &h;
    } else {
      // Keep the hit closest to the expected hit time
      if (std::abs(h.getT0() - mean_time_ns_) <
          std::abs(it->second->getT0() - mean_time_ns_)) {
        it->second = &h;
      }
    }
  }

  // -------------------------------------------------------------------------
  // Determine which strips are clusterable (≥ neighbor threshold) and which
  // can seed a cluster (≥ seed threshold + timing/chi2 cuts).
  //
  // Thresholds are in units of the per-strip noise RMS.  Each hit may carry its
  // own measured noise (getNoise() > 0, from the real-data pedestal table);
  // when it does not (MC), we fall back to the uniform ctor noise so the MC
  // path is unchanged.
  // -------------------------------------------------------------------------
  std::set<int> clusterable_set;
  std::vector<int> seed_channels;

  for (const auto& [ch, hp] : channel_map) {
    const double amp = hp->getAmplitude();
    const double noise = hitNoise(*hp);
    if (amp >= neighbor_threshold_ * noise) {
      clusterable_set.insert(ch);
    }
    if (amp >= seed_threshold_ * noise && passesSeedCuts(*hp)) {
      seed_channels.push_back(ch);
    }
  }

  // Sort seeds by amplitude (highest first) so the strongest hit initiates.
  std::sort(seed_channels.begin(), seed_channels.end(), [&](int a, int b) {
    return channel_map.at(a)->getAmplitude() >
           channel_map.at(b)->getAmplitude();
  });

  // -------------------------------------------------------------------------
  // BFS expansion from each seed.
  // -------------------------------------------------------------------------
  std::vector<ClusterCandidate> clusters;

  for (int seed_ch : seed_channels) {
    // The seed might have already been claimed by an earlier cluster.
    if (clusterable_set.find(seed_ch) == clusterable_set.end()) continue;

    ClusterCandidate cand;
    double cluster_weighted_t = 0.0;
    double cluster_total_amp = 0.0;
    double cluster_noise_sq = 0.0;

    std::deque<int> unchecked;
    unchecked.push_back(seed_ch);
    clusterable_set.erase(seed_ch);

    while (!unchecked.empty()) {
      const int cur_ch = unchecked.front();
      unchecked.pop_front();

      const ldmx::FittedSiStripHit& hit = *channel_map.at(cur_ch);
      const double amp = hit.getAmplitude();

      // Accumulate cluster quantities.
      const double noise = hitNoise(hit);
      cand.strip_ids.push_back(cur_ch);
      cluster_total_amp += amp;
      cluster_weighted_t += amp * hit.getT0();
      cluster_noise_sq += noise * noise;

      // Check nearest neighbours (strip ± 1).
      for (int delta : {-1, +1}) {
        const int nb_ch = cur_ch + delta;
        if (clusterable_set.find(nb_ch) == clusterable_set.end()) continue;

        // Timing consistency with cluster so far.
        if (!passesNeighborCuts(*channel_map.at(nb_ch), cluster_weighted_t,
                                cluster_total_amp)) {
          continue;
        }

        unchecked.push_back(nb_ch);
        clusterable_set.erase(nb_ch);
      }
    }

    // Cluster S/N cut.
    if (cluster_noise_sq <= 0.0) continue;
    if (cluster_total_amp / std::sqrt(cluster_noise_sq) < cluster_threshold_) {
      continue;
    }

    // -----------------------------------------------------------------------
    // Compute the charge-weighted centroid strip and timing.
    // -----------------------------------------------------------------------
    double sum_amp_strip = 0.0;
    for (int ch : cand.strip_ids) {
      sum_amp_strip += channel_map.at(ch)->getAmplitude() * ch;
    }

    cand.centroid_strip = sum_amp_strip / cluster_total_amp;
    cand.total_amplitude = cluster_total_amp;
    cand.time_ns = cluster_weighted_t / cluster_total_amp;
    cand.n_strips = static_cast<int>(cand.strip_ids.size());

    // Charge-weighted RMS around the centroid [strips].
    // For multi-strip clusters this uses the actual charge-sharing profile,
    // giving sub-pitch resolution when charge is sharply peaked.
    // For single-strip clusters the RMS is zero, so we floor at the binary
    // single-strip uncertainty 1/√12.
    double sum_amp_dsq = 0.0;
    for (int ch : cand.strip_ids) {
      double d = ch - cand.centroid_strip;
      sum_amp_dsq += channel_map.at(ch)->getAmplitude() * d * d;
    }
    constexpr double k_single_strip_sigma = 1.0 / 3.4641;  // 1/√12
    const double rms = std::sqrt(sum_amp_dsq / cluster_total_amp);
    cand.sigma_strip = (rms > 0.0) ? rms : k_single_strip_sigma;
    cand.layer_id = channel_map.at(seed_ch)->getLayerID();

    clusters.push_back(std::move(cand));
  }

  return clusters;
}

}  // namespace tracking::digitization
