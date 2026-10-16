/**
 * @file CLUE3D.h
 * @brief 2D CLUE per layer, CLUE3D linking and energy sharing for the ECal
 *
 * Follows the CMS HGCAL layer clustering and TICL CLUE3D, with an added
 * shower profile based energy sharing between close showers.
 */

#ifndef ECAL_CLUE3D_H_
#define ECAL_CLUE3D_H_

#include <vector>

#include "Ecal/Event/EcalHit.h"

namespace ecal {

class CLUE3D {
 public:
  struct Params {
    // hits below this are ignored [MeV]
    double hit_ecut_{0.};
    // 2D: density radius, seed and outlier separation [mm]
    double dc2d_{10.};
    // 2D: weight of neighbors in the density
    double kernel2d_{0.2};
    double deltas2d_{10.};
    double deltao2d_{25.};
    // 2D: min seed density [MeV]
    double rhoc2d_{5.};
    // 3D: layers looked at on each side
    int sibling_layers_{2};
    // 3D: density radius [mm], kernel weight of other layers
    double dens_xy3d_{15.};
    double kernel3d_{0.5};
    // 3D: seed separation [mm], min density [MeV], min self fraction
    double crit_xy3d_{10.};
    double crit_rho3d_{50.};
    double self_frac3d_{0.05};
    // 3D: outlier separation in units of crit_xy3d
    double outlier_mult3d_{2.};
    // orphan layer clusters within this of an axis are attached [mm]
    double attach_radius_{40.};
    // satellites below sc_ratio of a cluster within sc_radius merge [mm]
    double sc_ratio_{0.06};
    double sc_radius_{150.};
    // clusters spanning fewer layers merge regardless of energy
    int sc_min_layers_{5};
    // clusters below this are dropped [MeV]
    double min_cluster_energy_{20.};
    // energy sharing between clusters above share_min_energy [MeV]
    bool share_{true};
    int share_iters_{30};
    double share_min_energy_{300.};
    // shared clusters closer than this are merged [mm]
    double coincide_dist_{4.};
    // smaller than ghost_ratio of a cluster within ghost_dist merges [mm]
    double ghost_ratio_{0.15};
    double ghost_dist_{12.};
    // exponential transverse profile slope per layer [mm]
    std::vector<double> profile_lambda_{};
    // log weight W0 and window around the max cell [mm]
    double w0_{4.};
    double pos_window_{15.};
  };

  struct LayerCluster {
    double e_{0}, x_{0}, y_{0}, z_{0};
    int layer_{0};
    std::vector<int> hits_;
    double rho_{0}, delta_{0};
    int nh_{-1};
    int cid_{-1};
  };

  struct Cluster {
    double e_{0};
    // energy weighted centroid, on the fitted axis in x,y
    double x_{0}, y_{0}, z_{0};
    double dxdz_{0}, dydz_{0};
    double rms_x_{0}, rms_y_{0}, rms_z_{0};
    // first layer centroid
    double x0_{0}, y0_{0}, z0_{0};
    std::vector<int> hits_;
    std::vector<double> frac_;
    // x,y of the axis at z
    double axisX(double zz) const { return x_ + dxdz_ * (zz - z_); }
    double axisY(double zz) const { return y_ + dydz_ * (zz - z_); }
  };

  CLUE3D() = default;
  explicit CLUE3D(const Params& p) : p_{p} {}

  /// Run on all hits; returned hit indices refer to the input vector
  std::vector<Cluster> cluster(const std::vector<ldmx::EcalHit>& hits);

  const std::vector<LayerCluster>& layerClusters() const { return lcs_; }

  /// Profile slope used at a layer [mm]
  double lambda(int layer) const;

 private:
  void cluster2D(const std::vector<int>& layer_hits);
  void cluster3D();
  void fitAxis(Cluster& c, const std::vector<int>& hit_idx,
               const std::vector<double>& frac) const;

  Params p_{};
  const std::vector<ldmx::EcalHit>* hits_{nullptr};
  std::vector<int> layer_of_;
  std::vector<char> core_;
  std::vector<LayerCluster> lcs_;
};

}  // namespace ecal

#endif  // ECAL_CLUE3D_H_
