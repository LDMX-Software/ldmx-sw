/**
 * @file EcalClusterProducer.cxx
 * @brief Producer that performs clustering in the ECal
 * @author Josh Hiltbrand, University of Minnesota
 */

#include "Ecal/EcalClusterProducer.h"

#include "Ecal/CLUE.h"
#include "Ecal/CLUE3D.h"
#include "Ecal/Event/ClusterAlgoResult.h"
#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/IntermediateCluster.h"
#include "Ecal/MyClusterWeight.h"
#include "Recon/TemplatedClusterFinder.h"

namespace ecal {

EcalClusterProducer::EcalClusterProducer(const std::string& name,
                                         framework::Process& process)
    : Producer(name, process) {}

void EcalClusterProducer::configure(framework::config::Parameters& ps) {
  cutoff_ = ps.get<double>("cutoff");
  seed_threshold_ = ps.get<double>("seed_threshold");

  dc_ = ps.get<double>("dc");
  rhoc_ = ps.get<double>("rhoc");
  deltac_ = ps.get<double>("deltac");
  deltao_ = ps.get<double>("deltao");

  rec_hit_coll_name_ = ps.get<std::string>("rec_hit_coll_name");
  rec_hit_pass_name_ = ps.get<std::string>("rec_hit_pass_name");
  algo_coll_name_ = ps.get<std::string>("algo_coll_name");
  algo_name_ = ps.get<std::string>("algo_name");
  cluster_coll_name_ = ps.get<std::string>("cluster_coll_name");
  clue_ = ps.get<bool>("clue");
  nbr_of_layers_ = ps.get<int>("nbr_of_layers");
  reclustering_ = ps.get<bool>("reclustering");
  algorithm_ = ps.get<std::string>("algorithm", "clue");

  clue3d_.hit_ecut = ps.get<double>("clue3d_hit_ecut", clue3d_.hit_ecut);
  clue3d_.dc2d = ps.get<double>("clue3d_dc2d", clue3d_.dc2d);
  clue3d_.kernel2d = ps.get<double>("clue3d_kernel2d", clue3d_.kernel2d);
  clue3d_.deltas2d = ps.get<double>("clue3d_deltas2d", clue3d_.deltas2d);
  clue3d_.deltao2d = ps.get<double>("clue3d_deltao2d", clue3d_.deltao2d);
  clue3d_.rhoc2d = ps.get<double>("clue3d_rhoc2d", clue3d_.rhoc2d);
  clue3d_.sibling_layers =
      ps.get<int>("clue3d_sibling_layers", clue3d_.sibling_layers);
  clue3d_.dens_xy3d = ps.get<double>("clue3d_dens_xy3d", clue3d_.dens_xy3d);
  clue3d_.kernel3d = ps.get<double>("clue3d_kernel3d", clue3d_.kernel3d);
  clue3d_.crit_xy3d = ps.get<double>("clue3d_crit_xy3d", clue3d_.crit_xy3d);
  clue3d_.crit_rho3d = ps.get<double>("clue3d_crit_rho3d", clue3d_.crit_rho3d);
  clue3d_.self_frac3d =
      ps.get<double>("clue3d_self_frac3d", clue3d_.self_frac3d);
  clue3d_.outlier_mult3d =
      ps.get<double>("clue3d_outlier_mult3d", clue3d_.outlier_mult3d);
  clue3d_.attach_radius =
      ps.get<double>("clue3d_attach_radius", clue3d_.attach_radius);
  clue3d_.sc_ratio = ps.get<double>("clue3d_sc_ratio", clue3d_.sc_ratio);
  clue3d_.sc_radius = ps.get<double>("clue3d_sc_radius", clue3d_.sc_radius);
  clue3d_.sc_min_layers =
      ps.get<int>("clue3d_sc_min_layers", clue3d_.sc_min_layers);
  clue3d_.coincide_dist =
      ps.get<double>("clue3d_coincide_dist", clue3d_.coincide_dist);
  clue3d_.ghost_ratio =
      ps.get<double>("clue3d_ghost_ratio", clue3d_.ghost_ratio);
  clue3d_.ghost_dist = ps.get<double>("clue3d_ghost_dist", clue3d_.ghost_dist);
  clue3d_.min_cluster_energy =
      ps.get<double>("clue3d_min_cluster_energy", clue3d_.min_cluster_energy);
  clue3d_.share = ps.get<bool>("clue3d_share", clue3d_.share);
  clue3d_.share_iters = ps.get<int>("clue3d_share_iters", clue3d_.share_iters);
  clue3d_.share_min_energy =
      ps.get<double>("clue3d_share_min_energy", clue3d_.share_min_energy);
  clue3d_.profile_lambda = ps.get<std::vector<double>>("clue3d_profile_lambda",
                                                       clue3d_.profile_lambda);
  clue3d_.w0 = ps.get<double>("clue3d_w0", clue3d_.w0);
  clue3d_.pos_window = ps.get<double>("clue3d_pos_window", clue3d_.pos_window);
}

void EcalClusterProducer::produce(framework::Event& event) {
  const auto& ecal_hits{event.getCollection<ldmx::EcalHit>(rec_hit_coll_name_,
                                                           rec_hit_pass_name_)};
  if (ecal_hits.size() == 0) {
    // keep the collection present for downstream readers
    ldmx_log(debug) << "No ECal hits found";
    std::vector<ldmx::EcalCluster> empty;
    event.add(cluster_coll_name_, empty);
    return;
  }

  if (algorithm_ == "clue3d") {
    CLUE3D algo(clue3d_);
    auto clusters = algo.cluster(ecal_hits);
    std::vector<ldmx::EcalCluster> ecal_clusters;
    for (const auto& c : clusters) {
      ldmx::EcalCluster cluster;
      cluster.setEnergy(c.e);
      cluster.setCentroidXYZ(c.x, c.y, c.z);
      cluster.setDXDZ(c.dxdz);
      cluster.setDYDZ(c.dydz);
      cluster.setRMSXYZ(c.rms_x, c.rms_y, c.rms_z);
      cluster.setFirstLayerCentroidXYZ(c.x0, c.y0, c.z0);
      // 3D clusters span all layers
      cluster.setLayer(0);
      std::vector<unsigned int> ids;
      std::vector<float> hx, hy, hz, he;
      for (std::size_t i = 0; i < c.hits.size(); i++) {
        const auto& h = ecal_hits[c.hits[i]];
        ids.push_back(h.getID());
        hx.push_back(h.getXPos());
        hy.push_back(h.getYPos());
        hz.push_back(h.getZPos());
        // shared energy of the hit
        he.push_back(c.frac[i] * h.getEnergy());
      }
      cluster.setNHits(ids.size());
      cluster.setIDs(ids);
      cluster.setHitValsX(hx);
      cluster.setHitValsY(hy);
      cluster.setHitValsZ(hz);
      cluster.setHitValsE(he);
      ecal_clusters.push_back(cluster);
    }
    event.add(cluster_coll_name_, ecal_clusters);
  } else if (clue_) {
    ldmx_log(info) << "Using CLUE clustering algorithm";
    CLUE cf;
    cf.cluster(ecal_hits, dc_, rhoc_, deltac_, deltao_, nbr_of_layers_,
               reclustering_);
    ldmx_log(debug) << "CLUE algorithm finished clustering";
    std::vector<IntermediateCluster> interm_cluster = cf.getClusters();
    std::vector<IntermediateCluster> f_interm_cluster =
        cf.getFirstLayerCentroids();
    ldmx_log(debug) << "Got " << interm_cluster.size() << " clusters with "
                    << f_interm_cluster.size() << " first layer centroids";

    auto n_loops = cf.getNLoops();
    ldmx_log(debug) << "Number of clustering loops: " << n_loops;

    if (reclustering_) {
      ldmx_log(debug) << "Reclustererd initial number of clusters: "
                      << cf.getInitialClusterNbr()
                      << ", final number of clusters: "
                      << interm_cluster.size();
    }

    std::vector<ldmx::EcalCluster> ecal_clusters;
    ldmx_log(debug) << "Filling " << interm_cluster.size()
                    << " clusters into ecal_clusters";
    for (size_t cluster_indx = 0; cluster_indx < interm_cluster.size();
         cluster_indx++) {
      ldmx::EcalCluster cluster;

      cluster.setEnergy(interm_cluster[cluster_indx].energy());
      cluster.setCentroidXYZ(interm_cluster[cluster_indx].centroidX(),
                             interm_cluster[cluster_indx].centroidY(),
                             interm_cluster[cluster_indx].centroidZ());
      cluster.setFirstLayerCentroidXYZ(
          f_interm_cluster[cluster_indx].centroidX(),
          f_interm_cluster[cluster_indx].centroidY(),
          f_interm_cluster[cluster_indx].centroidZ());
      cluster.setNHits(interm_cluster[cluster_indx].hits().size());
      cluster.addHits(interm_cluster[cluster_indx].hits());
      cluster.addFirstLayerHits(f_interm_cluster[cluster_indx].hits());

      float cl_x(0), cl_y(0), cl_z(0), cl_xx(0), cl_yy(0), cl_zz(0);
      float cl_w = 1;  // weight
      float sumw = 0;

      for (auto hit : interm_cluster[cluster_indx].hits()) {
        if (hit->getEnergy() < min_hit_energy_) continue;
        cl_w = log(hit->getEnergy()) - log(min_hit_energy_);
        cl_x += cl_w * hit->getXPos();
        cl_y += cl_w * hit->getYPos();
        cl_z += cl_w * hit->getZPos();
        cl_xx += cl_w * hit->getXPos() * hit->getXPos();
        cl_yy += cl_w * hit->getYPos() * hit->getYPos();
        cl_zz += cl_w * hit->getZPos() * hit->getZPos();
        sumw += cl_w;
      }  // over hits
      // could probably get this as cluster.getCentroidX() instead
      cl_x /= sumw;  // now is <x_>
      cl_y /= sumw;
      cl_z /= sumw;
      cl_xx /= sumw;  // now is <x_^2>
      cl_yy /= sumw;
      cl_zz /= sumw;
      cl_xx = sqrt(cl_xx - cl_x * cl_x);  // now is sqrt(<x_^2>-<x_>^2)
      cl_yy = sqrt(cl_yy - cl_y * cl_y);
      cl_zz = sqrt(cl_zz - cl_z * cl_z);

      cluster.setRMSXYZ(cl_xx, cl_yy, cl_zz);
      cluster.setLayer(interm_cluster[cluster_indx].layer());
      ldmx_log(trace) << "Cluster " << cluster_indx
                      << " energy: " << cluster.getEnergy()
                      << ", nHits: " << cluster.getNHits() << ", centroid: ("
                      << cluster.getCentroidX() << ", "
                      << cluster.getCentroidY() << ", "
                      << cluster.getCentroidZ() << "), " << "RMS : ("
                      << cluster.getRMSX() << ", " << cluster.getRMSY() << ", "
                      << cluster.getRMSZ() << ")" << ", first layer centroid: ("
                      << cluster.getFirstLayerCentroidX() << ", "
                      << cluster.getFirstLayerCentroidY() << ", "
                      << cluster.getFirstLayerCentroidZ() << ")";

      ecal_clusters.push_back(cluster);
    }
    ldmx_log(debug) << "Filled " << ecal_clusters.size()
                    << " clusters into ecal_clusters";
    event.add(cluster_coll_name_, ecal_clusters);
  } else {
    ldmx_log(info) << "Using simple clustering algorithm " << algo_name_;
    recon::TemplatedClusterFinder<ldmx::EcalHit, MyClusterWeight> cf;

    for (const ldmx::EcalHit& hit : ecal_hits) {
      // Skip zero energy digis.
      if (hit.getEnergy() == 0) {
        continue;
      }
      cf.add(hit);
    }

    cf.cluster(seed_threshold_, cutoff_);
    auto interm_cluster = cf.getClusters();
    std::map<int, double> c_weights = cf.getWeights();

    ldmx::ClusterAlgoResult algo_result;
    algo_result.set(algo_name_, 3, c_weights.rbegin()->first);
    algo_result.setAlgoVar(0, cutoff_);
    algo_result.setAlgoVar(1, seed_threshold_);
    algo_result.setAlgoVar(2, cf.getNSeeds());

    std::map<int, double>::iterator it = c_weights.begin();
    for (it = c_weights.begin(); it != c_weights.end(); it++) {
      algo_result.setWeight(it->first, it->second / 100);
    }

    std::vector<ldmx::EcalCluster> ecal_clusters;
    for (size_t cluster_indx = 0; cluster_indx < interm_cluster.size();
         cluster_indx++) {
      ldmx::EcalCluster cluster;

      cluster.setEnergy(interm_cluster[cluster_indx].energy());
      cluster.setCentroidXYZ(interm_cluster[cluster_indx].centroidX(),
                             interm_cluster[cluster_indx].centroidY(),
                             interm_cluster[cluster_indx].centroidZ());
      cluster.setNHits(interm_cluster[cluster_indx].hits().size());
      cluster.addHits(interm_cluster[cluster_indx].hits());
      ecal_clusters.push_back(cluster);
    }

    event.add(cluster_coll_name_, ecal_clusters);
    event.add(algo_coll_name_, algo_result);
  }  // end on simple clustering
}
}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalClusterProducer);
