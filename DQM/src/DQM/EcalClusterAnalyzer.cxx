#include "DQM/EcalClusterAnalyzer.h"

namespace dqm {

void EcalClusterAnalyzer::configure(framework::config::Parameters& ps) {
  use_simulated_electron_number_ =
      ps.get<bool>("use_simulated_electron_number");
  nbr_of_electrons_ = ps.get<int>("nbr_of_electrons");

  ecal_sim_hit_coll_ = ps.get<std::string>("ecal_sim_hit_coll");
  ecal_sim_hit_pass_ = ps.get<std::string>("ecal_sim_hit_pass");

  rec_hit_coll_name_ = ps.get<std::string>("rec_hit_coll_name");
  rec_hit_pass_name_ = ps.get<std::string>("rec_hit_pass_name");

  cluster_coll_name_ = ps.get<std::string>("cluster_coll_name");
  cluster_pass_name_ = ps.get<std::string>("cluster_pass_name");

  ecal_sp_hits_passname_ = ps.get<std::string>("ecal_sp_hits_passname");
  inverse_skim_ = ps.get<bool>("inverse_skim");
  n_ecal_clusters_min_ = ps.get<int>("n_ecal_clusters_min");
  return;
}

void EcalClusterAnalyzer::analyze(const framework::Event& event) {
  const auto& ecal_rec_hits{event.getCollection<ldmx::EcalHit>(
      rec_hit_coll_name_, rec_hit_pass_name_)};
  const auto& ecal_sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      ecal_sim_hit_coll_, ecal_sim_hit_pass_)};
  const auto& ecal_clusters{event.getCollection<ldmx::EcalCluster>(
      cluster_coll_name_, cluster_pass_name_)};

  // Determine the number of recoil electrons in the event
  // By default from the TS track counting
  int nbr_of_electrons{event.getElectronCount()};
  // If configured to use the simulated electron number, use that instead
  if (use_simulated_electron_number_) {
    nbr_of_electrons = nbr_of_electrons_;
  }

  std::map<int, int> layer_cluster_count;
  for (const auto& cluster : ecal_clusters) {
    auto layer = cluster.getLayer();
    layer_cluster_count[layer]++;
  }

  int total_clusters = 0;
  for (const auto& [layer, count] : layer_cluster_count) {
    total_clusters += count;
  }

  int n_ecal_clusters = 0;
  if (layer_cluster_count.size() != 0) {
    n_ecal_clusters = static_cast<int>(std::round(
        static_cast<double>(total_clusters) / layer_cluster_count.size()));
  }

  ldmx_log(info) << "Avg number of clusters per layer: " << n_ecal_clusters;
  // Fill histograms with the number of clusters
  histograms_.fill("number_of_clusters", total_clusters);
  histograms_.fill("number_of_clusters_per_layer", n_ecal_clusters);
  histograms_.fill("number_of_clusters_first_layer", layer_cluster_count[0]);

  // Fill simplied 3-bin histogram to check the prediction
  if (n_ecal_clusters == nbr_of_electrons) {
    // correct
    histograms_.fill("correctly_predicted_events", 1);
  } else if (n_ecal_clusters < nbr_of_electrons) {
    // undercounting
    histograms_.fill("correctly_predicted_events", 0);
  } else if (n_ecal_clusters > nbr_of_electrons) {
    // overcounting
    histograms_.fill("correctly_predicted_events", 2);
  }

  std::unordered_map<int, std::pair<int, std::vector<double>>> hit_info;
  hit_info.reserve(ecal_rec_hits.size());

  // Determine the truth information for the recoil electron
  std::vector<std::vector<float>> sp_electron_positions;
  const auto& ecal_sp_hits{event.getCollection<ldmx::SimTrackerHit>(
      "EcalScoringPlaneHits", ecal_sp_hits_passname_)};

  std::vector<ldmx::SimTrackerHit> sorted_sp_hits = ecal_sp_hits;
  std::sort(sorted_sp_hits.begin(), sorted_sp_hits.end(),
            [](const ldmx::SimTrackerHit& a, const ldmx::SimTrackerHit& b) {
              return a.getTrackID() < b.getTrackID();
            });

  ldmx_log(trace) << "Number of ECal Scoring Plane Hits: "
                  << sorted_sp_hits.size();

  // Collect positions of all recoil electrons on the SP
  // relying on the track ID to identify them
  unsigned int n_filled = 0;
  for (const ldmx::SimTrackerHit& sp_hit : sorted_sp_hits) {
    if (sp_hit.getPdgID() != 11) continue;
    if (sp_hit.getMomentum()[2] <= 0) continue;
    ldmx::SimSpecialID hit_id(sp_hit.getID());
    // Ecal scoring plane is plane 31
    if (hit_id.plane() != 31) continue;
    if (n_filled < nbr_of_electrons) {
      ldmx_log(trace) << "\tSP Hit to be added with Track ID : "
                      << sp_hit.getTrackID() << ", SP Hit Position ("
                      << sp_hit.getPosition()[0] << ", "
                      << sp_hit.getPosition()[1] << ", "
                      << sp_hit.getPosition()[2] << ") mm";
      sp_electron_positions.push_back(sp_hit.getPosition());
      n_filled++;
    }
  }

  ldmx_log(info) << "Number of ECal CLUE clusters: " << n_ecal_clusters
                 << ", TS counted electrons: " << nbr_of_electrons
                 << ", SP electrons: " << sp_electron_positions.size();

  double sp_ele_dist{9999.};
  if (nbr_of_electrons == 2 && sp_electron_positions.size() > 1) {
    // Measures sp_ele_distance between two electrons in the ECal scoring plane
    // TODO: generalize for n electrons
    std::vector<float> pos1;
    std::vector<float> pos2;
    pos1 = sp_electron_positions[0];
    pos2 = sp_electron_positions[1];
    sp_ele_dist = std::sqrt((pos1[0] - pos2[0]) * (pos1[0] - pos2[0]) +
                            (pos1[1] - pos2[1]) * (pos1[1] - pos2[1]));

  }  // end block about the scoring plane hits

  ldmx_log(trace) << "Distance between the two e- in the ECal scoring plane: "
                  << sp_ele_dist << " mm";

  // Loop over the rechits and find the matching simhits
  ldmx_log(trace) << "Loop over the rechits and find the matching simhits";
  for (const auto& hit : ecal_rec_hits) {
    auto it = std::find_if(
        ecal_sim_hits.begin(), ecal_sim_hits.end(),
        [&hit](const auto& sim_hit) { return sim_hit.getID() == hit.getID(); });
    if (it != ecal_sim_hits.end()) {
      // if found a simhit matching this rechit
      int ancestor = 0;
      int prev_ancestor = 0;
      bool tagged = false;
      int tag = 0;
      std::vector<double> edep;
      edep.resize(nbr_of_electrons + 1);
      for (int i = 0; i < it->getNumberOfContribs(); i++) {
        // for each contrib in this simhit
        const auto& contrib = it->getContrib(i);
        // get origin electron ID
        ancestor = contrib.origin_id_;
        // store energy from this contrib at index = origin electron ID
        if (ancestor <= nbr_of_electrons) edep[ancestor] += contrib.edep_;
        if (!tagged && i != 0 && prev_ancestor != ancestor) {
          // if origin electron ID does not match previous origin electron ID
          // this hit has contributions from several electrons, ie mixed case
          tag = 0;
          tagged = true;
        }
        prev_ancestor = ancestor;
      }
      if (!tagged) {
        // if not tagged, hit was from a single electron
        tag = prev_ancestor;
      }
      histograms_.fill("ancestors", tag);
      hit_info.insert({hit.getID(), std::make_pair(tag, edep)});
    }  // end if simhit found
  }  // end loop on the rechits

  // Loop over the clusters
  int clustered_hits = 0;
  ldmx_log(trace) << "Loop over the clusters, N = " << n_ecal_clusters;
  for (const auto& cl : ecal_clusters) {
    auto layer = cl.getLayer();
    ldmx_log(trace) << "Cluster in layer " << layer
                    << ", energy: " << cl.getEnergy()
                    << ", number of hits: " << cl.getHitIDs().size();
    auto cluster_centroid_x = cl.getCentroidX();
    auto cluster_centroid_y = cl.getCentroidY();
    auto cluster_rms_x = cl.getRMSX();
    auto cluster_rms_y = cl.getRMSY();

    // Find the closest sp_electron_positions to the cluster centroid
    double min_distance = 9999.;
    double sp_clue_x_residuals = 9999.;
    double sp_clue_y_residuals = 9999.;
    for (const auto& sp_pos : sp_electron_positions) {
      double distance = std::sqrt(
          (sp_pos[0] - cluster_centroid_x) * (sp_pos[0] - cluster_centroid_x) +
          (sp_pos[1] - cluster_centroid_y) * (sp_pos[1] - cluster_centroid_y));
      if (distance < min_distance) {
        min_distance = distance;
        sp_clue_x_residuals = sp_pos[0] - cluster_centroid_x;
        sp_clue_y_residuals = sp_pos[1] - cluster_centroid_y;
      }
    }  // end loop on the scoring plane electron positions
    // Fill histogram with the distance to the closest scoring plane electron
    ldmx_log(trace) << "\tCluster centroid: (" << cluster_centroid_x << " +/- "
                    << cluster_rms_x << ", " << cluster_centroid_y << " +/- "
                    << cluster_rms_y
                    << " mm; min distance to SP electron: " << min_distance
                    << " mm";
    if (layer == 0) {
      histograms_.fill("sp_clue_distance", min_distance);
      histograms_.fill("sp_clue_x_residual", sp_clue_x_residuals);
      histograms_.fill("sp_clue_y_residual", sp_clue_y_residuals);
    }
    histograms_.fill("sp_clue_distance_vs_layer", layer, min_distance);

    // for each cluster
    // total number of hits coming from electron, index = electron ID
    std::vector<double> n_hits_from_electron;
    n_hits_from_electron.resize(nbr_of_electrons + 2);
    // total number of energy coming from electron, index = electron ID
    std::vector<double> energy_from_electron;
    energy_from_electron.resize(nbr_of_electrons + 1);
    double energy_sum = 0.;
    double n_sum = 0.;

    const auto& hit_ids = cl.getHitIDs();
    for (const auto& id : hit_ids) {
      // for each hit in cluster, find previously stored info
      auto it = hit_info.find(id);
      if (it != hit_info.end()) {
        auto t = it->second;
        // origin electron ID (or 0 for mixed)
        auto id_electron = t.first;
        // energy vector
        auto energies = t.second;
        // increment number of hits coming from this electron
        n_hits_from_electron[id_electron]++;
        n_sum++;

        double hit_energy_sum = 0.;
        for (int i = 1; i < nbr_of_electrons + 1; i++) {
          // loop through energy vector
          if (energies[i] > 0.) {
            energy_sum += energies[i];
            // add energy from electron i in this hit to total energy from
            // electron i in cluster
            energy_from_electron[i] += energies[i];
          }
        }
        // if mixed hit, add the total energy of this hit to mixed hit energy
        // counter
        if (id_electron == 0) energy_from_electron[0] += hit_energy_sum;
        energy_sum += hit_energy_sum;

        clustered_hits++;
      }  // end if hit info found
    }  // end loop on the hit IDs in the cluster

    if (energy_sum > 0) {
      // get largest energy contribution
      double max_energy_contribution = *max_element(
          energy_from_electron.begin(), energy_from_electron.end());
      // energy purity = largest contribution / all energy
      histograms_.fill("energy_percentage",
                       100. * (max_energy_contribution / energy_sum));
      if (energy_from_electron[0] > 0.)
        histograms_.fill("mixed_hit_energy",
                         100. * (energy_from_electron[0] / energy_sum));

      histograms_.fill("total_energy_vs_hits", energy_sum,
                       cl.getHitIDs().size());
      histograms_.fill("total_energy_vs_purity", energy_sum,
                       100. * (max_energy_contribution / energy_sum));

      if (nbr_of_electrons == 2) {
        histograms_.fill("sp_ele_distance_vs_purity", sp_ele_dist,
                         100. * (max_energy_contribution / energy_sum));
      }
    }
    if (n_sum > 0) {
      double n_max = *max_element(n_hits_from_electron.begin(),
                                  n_hits_from_electron.end());
      histograms_.fill("same_ancestor", 100. * (n_max / n_sum));
    }
  }  // end loop on the clusters

  histograms_.fill("clusterless_hits", (ecal_rec_hits.size() - clustered_hits));
  histograms_.fill("total_rechits_in_event", ecal_rec_hits.size());
  histograms_.fill(
      "clusterless_hits_percentage",
      100. * (ecal_rec_hits.size() - clustered_hits) / ecal_rec_hits.size());

  if (inverse_skim_) {
    // inverse operation: drop events with enough clusters
    if (n_ecal_clusters > n_ecal_clusters_min_) {
      setStorageHint(framework::HINT_SHOULD_DROP);
    } else {
      setStorageHint(framework::HINT_SHOULD_KEEP);
    }
  } else {
    // normal operation: keep events with enough clusters
    if (n_ecal_clusters > n_ecal_clusters_min_) {
      setStorageHint(framework::HINT_SHOULD_KEEP);
    } else {
      setStorageHint(framework::HINT_SHOULD_DROP);
    }
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalClusterAnalyzer)
