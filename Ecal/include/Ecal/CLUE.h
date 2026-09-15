/**
 * @file CLUE.h
 * @brief A version of CLUE (CMS) for clustering in ECal
 * @author Ella Viirola, Lund University
 */

#ifndef ECAL_CLUE_H_
#define ECAL_CLUE_H_

#include <math.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stack>

#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/IntermediateCluster.h"
#include "Framework/Logger.h"

namespace ecal {

class CLUE {
  enableLogging("CLUE");

 public:
  struct Density {
    double x_;
    double y_;
    double z_;
    double total_energy_;
    int index_;

    // index of density this density is follower of
    // set to index of spatially closest density with higher energy; -1 if seed
    int follower_of_;
    // 2D separation distance to density that this is follower of
    double delta_;
    // separation in z to density that this is follower of
    double z_delta_;
    // 2D cluster ID
    int cluster_id_;
    // layer of density
    int layer_;
    // hits in this density
    std::vector<const ldmx::EcalHit*> hits_;

    Density() {}

    Density(float xx, float yy) : x_(xx), y_(yy) {
      total_energy_ = 0.;
      index_ = -1;
      follower_of_ = -1;
      delta_ = std::numeric_limits<float>::max();
      z_delta_ = std::numeric_limits<float>::max();
      cluster_id_ = -1;
      layer_ = -1;
      z_ = 0.;
      hits_ = {};
    }
  };

  /**
   * Euclidean distance between two points
   * @tparam T floating point type
   */
  template <typename T>
  T dist(T x1, T y1, T x2, T y2);
  // 3D version
  template <typename T>
  T dist(T x1, T y1, T z1, T x2, T y2, T z2);
  std::vector<std::vector<const ldmx::EcalHit*>> createLayers(
      const std::vector<const ldmx::EcalHit*>& hits);
  float roundToDecimal(float x, int num_decimal_precision_digits);
  std::vector<std::shared_ptr<Density>> setup(
      const std::vector<const ldmx::EcalHit*>& hits);

  // get distance between clusters in the first layers, proxy for electron sep.
  void electronSeparation(std::vector<ldmx::EcalHit> hits);

  // connectingLayers marks if we're currently doing 3D clustering (i.e.
  // connecting seeds between layers) otherwise, layerTag tells us which layer
  // number we're working on
  std::vector<std::vector<const ldmx::EcalHit*>> clustering(
      std::vector<std::shared_ptr<Density>>& densities, bool connectingLayers,
      int layerTag = 0);

  std::vector<std::shared_ptr<Density>> setupForClue3D();

  void convertToIntermediateClusters(
      std::vector<std::vector<const ldmx::EcalHit*>>& clusters);

  void cluster(const std::vector<ldmx::EcalHit>& hits, double dc, double rc,
               double deltac, double deltao, int nbrOfLayers,
               bool reclustering);

  std::vector<double> getCentroidDistances() const {
    return centroid_distances_;
  }

  int getNLoops() const { return clustering_loops_; }

  int getInitialClusterNbr() const { return initial_cluster_nbr_; }

  std::vector<IntermediateCluster> getClusters() const {
    return final_clusters_;
  }

  // First layer centroids are available for potential future combination with
  // TS
  std::vector<IntermediateCluster> getFirstLayerCentroids() const {
    return first_layer_centroids_;
  }

 private:
  int clustering_loops_;

  bool reclustering_;

  double dc_;
  double rhoc_;
  double deltac_;
  double deltao_;
  double dm_;

  // layers in Ecal
  int max_layers_{32};
  int nbr_of_layers_;

  std::vector<double> layer_rho_c_;
  std::vector<double> layer_delta_c_;
  // containment radius for the different layers of the ECal
  std::vector<double> radius_{
      5.723387467629167,  5.190678018534044,  5.927290663506518,
      6.182560329200212,  7.907549398117859,  8.606100542857211,
      10.93381822596916,  12.043201938160239, 14.784548371508041,
      16.102403056546482, 18.986402399412817, 20.224453740305716,
      23.048820910305643, 24.11202594672678,  26.765135236851666,
      27.78700483852502,  30.291794353801293, 31.409870873194464,
      33.91006482486666,  35.173073672355926, 38.172422630271,
      40.880288341493205, 44.696485719120005, 49.23802839743545,
      53.789910813378675, 60.87843355562641,  66.32931132415688,
      75.78117972604727,  86.04697356716805,  96.90360704034346};

  std::vector<double> centroid_distances_;
  IntermediateCluster event_centroid_;

  std::vector<IntermediateCluster> first_layer_centroids_;

  int seed_index_{0};
  std::vector<std::vector<std::shared_ptr<Density>>> seeds_;

  int initial_cluster_nbr_{-1};
  std::vector<IntermediateCluster> final_clusters_;
  std::vector<std::pair<double, double>> layer_centroid_separations_;
};
}  // namespace ecal

#endif
