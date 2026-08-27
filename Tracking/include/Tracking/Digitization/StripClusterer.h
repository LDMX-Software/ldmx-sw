#pragma once

#include <vector>

#include "Tracking/Event/FittedSiStripHit.h"

namespace tracking::digitization {

/**
 * Nearest-neighbour clustering of fitted silicon-strip hits on a single sensor.
 *
 * Ported from HPS NearestNeighborRMSClusterer (Java).  Thresholds are all
 * expressed in units of the per-strip noise RMS so they are independent of the
 * absolute gain.
 *
 * Algorithm
 * ---------
 * 1. Build a channel→hit map.  If two hits share a strip, keep the one with
 *    smaller |t0| (closest to the expected hit time).
 * 2. Mark strips with amplitude/noise ≥ neighbor_threshold as *clusterable*.
 * 3. Mark clusterable strips that also satisfy amplitude/noise ≥ seed_threshold
 *    and pass the timing and chi2 quality cuts as *seeds*.
 * 4. For each seed (processed in strip-index order), grow a cluster by BFS:
 *      - Pop a strip from the "unchecked" queue and add it to the cluster.
 *      - Examine its two nearest neighbours (strip ± 1).
 *      - If a neighbour is still clusterable and its |t0 − cluster_weighted_t|
 *        is within neighbor_delta_t_ns, add it to the queue and mark it
 *        unavailable.
 * 5. Accept the cluster if  Σ(amp) / √Σ(noise²) > cluster_threshold.
 *
 * Output
 * ------
 * Each accepted cluster is reported as a ClusterCandidate:
 *   centroid_strip  Charge-weighted mean strip index (fractional).
 *   total_amplitude Σ amplitude [ADC counts].
 *   time_ns         Amplitude-weighted mean hit time [ns].
 *   sigma_strip     Position uncertainty [strips]: charge-weighted RMS around
 *                the centroid (uses actual charge-sharing profile); floored
 *                at 1/√12 (single-strip binary limit) when RMS = 0.
 *   n_strips        Number of strips in the cluster.
 *   layer_id        Sensor layer (copied from the input hits).
 *   strip_ids       Constituent strip indices (for diagnostics).
 *
 * Convert to physical coordinates in the caller:
 *   local_u  = centroid_strip * readout_pitch_mm
 *   sigma_u  = sigma_strip    * readout_pitch_mm
 */
class StripClusterer {
 public:
  // I like not having underscores for struct members,
  // but that's not settable until a later version of clang-tidy
  // NOLINTBEGIN(readability-identifier-naming)
  struct ClusterCandidate {
    double centroid_strip{0};   ///< Charge-weighted mean strip index.
    double total_amplitude{0};  ///< Total cluster amplitude [ADC counts].
    double time_ns{0};          ///< Amplitude-weighted mean hit time [ns].
    double sigma_strip{0};      ///< Position uncertainty [strips].
    int n_strips{0};
    int layer_id{-1};
    std::vector<int> strip_ids;
  };
  // NOLINTEND(readability-identifier-naming)

  /**
   * @param seed_threshold      Minimum amplitude/noise to seed a cluster
   * (default 4).
   * @param neighbor_threshold  Minimum amplitude/noise for a strip to join a
   * cluster (default 3).
   * @param cluster_threshold   Minimum Σamp / √Σnoise² for the cluster to be
   * kept (default 4).
   * @param noise_sigma_adc     Per-strip noise RMS [ADC counts].
   * @param mean_time_ns        Expected hit time for the seed timing cut [ns]
   * (default 0).
   * @param time_window_ns      Half-width of seed timing window [ns]; ≤ 0
   * disables (default -1).
   * @param neighbor_delta_t_ns Max |t0_neighbour − cluster_t| to join cluster
   * [ns]; ≤ 0 disables (default -1).
   * @param max_chi2_ndf        Max chi2/ndf for a hit to be used; ≤ 0 disables
   * (default -1).
   */
  StripClusterer(double seed_threshold = 4.0, double neighbor_threshold = 3.0,
                 double cluster_threshold = 4.0, double noise_sigma_adc = 5.0,
                 double mean_time_ns = 0.0, double time_window_ns = -1.0,
                 double neighbor_delta_t_ns = -1.0, double max_chi2_ndf = -1.0);

  /**
   * Cluster a set of fitted strip hits from a single sensor layer.
   *
   * @param hits  All FittedSiStripHits on one sensor (mixed layers are allowed
   *              but each hit's layer_id is preserved in the result).
   * @return      List of cluster candidates, one per accepted cluster.
   */
  std::vector<ClusterCandidate> findClusters(
      const std::vector<ldmx::FittedSiStripHit>& hits) const;

 private:
  bool passesSeedCuts(const ldmx::FittedSiStripHit& h) const;
  bool passesNeighborCuts(const ldmx::FittedSiStripHit& h,
                          double cluster_weighted_t,
                          double cluster_total_amp) const;

  /// Per-strip noise RMS to use for @p h: the hit's own measured noise if set
  /// (> 0), otherwise the uniform noise passed to the constructor.  This is how
  /// real-data hits (with per-channel pedestal noise) and MC hits (uniform)
  /// share one clustering path.
  double hitNoise(const ldmx::FittedSiStripHit& h) const {
    const double n = h.getNoise();
    return n > 0.0 ? n : noise_sigma_adc_;
  }

  double seed_threshold_;
  double neighbor_threshold_;
  double cluster_threshold_;
  double noise_sigma_adc_;
  double mean_time_ns_;
  double time_window_ns_;
  double neighbor_delta_t_ns_;
  double max_chi2_ndf_;
};

}  // namespace tracking::digitization
