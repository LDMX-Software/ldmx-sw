/**
 * @file EcalClusterAnalyzer.cxx
 * @brief Analysis of cluster performance
 * @author Ella Viirola, Lund University
 */

#include "DQM/EcalClusterAnalyzer.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace dqm {

void EcalClusterAnalyzer::configure(framework::config::Parameters& ps) {
  nbr_of_electrons_ = ps.getParameter<int>("nbr_of_electrons");

  ecal_sim_hit_coll_ = ps.getParameter<std::string>("ecal_sim_hit_coll");
  ecal_sim_hit_pass_ = ps.getParameter<std::string>("ecal_sim_hit_pass");

  rec_hit_coll_name_ = ps.getParameter<std::string>("rec_hit_coll_name");
  rec_hit_pass_name_ = ps.getParameter<std::string>("rec_hit_pass_name");

  cluster_coll_name_ = ps.getParameter<std::string>("cluster_coll_name");
  cluster_pass_name_ = ps.getParameter<std::string>("cluster_pass_name");
  return;
}

void EcalClusterAnalyzer::analyze(const framework::Event& event) {
  const auto& ecal_rec_hits{event.getCollection<ldmx::EcalHit>(
      rec_hit_coll_name_, rec_hit_pass_name_)};
  const auto& ecal_sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      ecal_sim_hit_coll_, ecal_sim_hit_pass_)};
  const auto& ecal_clusters{event.getCollection<ldmx::EcalCluster>(
      cluster_coll_name_, cluster_pass_name_)};

  if (ecal_clusters.size() == nbr_of_electrons_)
    histograms_.fill("correctly_predicted_events", 1);  // correct
  else if (ecal_clusters.size() < nbr_of_electrons_)
    histograms_.fill("correctly_predicted_events", 0);  // undercounting
  else if (ecal_clusters.size() > nbr_of_electrons_)
    histograms_.fill("correctly_predicted_events", 2);  // overcounting

  std::unordered_map<int, std::pair<int, std::vector<double>>> hitInfo;
  hitInfo.reserve(ecal_rec_hits.size());

  double dist;
  if (nbr_of_electrons_ == 2) {
    // Measures distance between two electrons in the ECal scoring plane
    // TODO: generalize for n electrons
    std::vector<float> pos1;
    std::vector<float> pos2;
    bool p1 = false;
    bool p2 = false;

    const auto& ecal_sp_hits{
        event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits")};
    for (const ldmx::SimTrackerHit& spHit : ecal_sp_hits) {
      if (spHit.getTrackID() == 1) {
        pos1 = spHit.getPosition();
        p1 = true;
      } else if (spHit.getTrackID() == 2) {
        pos2 = spHit.getPosition();
        p2 = true;
      }
    }
    if (p1 && p2)
      dist = std::sqrt(std::pow((pos1[0] - pos2[0]), 2) +
                       std::pow((pos1[1] - pos2[1]), 2));
  }

  for (const auto& hit : ecal_rec_hits) {
    auto it = std::find_if(
        ecal_sim_hits.begin(), ecal_sim_hits.end(),
        [&hit](const auto& simHit) { return simHit.getID() == hit.getID(); });
    if (it != ecal_sim_hits.end()) {
      // if found a simhit matching this rechit
      int ancestor = 0;
      int prevAncestor = 0;
      bool tagged = false;
      int tag = 0;
      std::vector<double> edep;
      edep.resize(nbr_of_electrons_ + 1);
      for (int i = 0; i < it->getNumberOfContribs(); i++) {
        // for each contrib in this simhit
        const auto& c = it->getContrib(i);
        // get origin electron ID
        ancestor = c.originID;
        // store energy from this contrib at index = origin electron ID
        if (ancestor <= nbr_of_electrons_) edep[ancestor] += c.edep;
        if (!tagged && i != 0 && prevAncestor != ancestor) {
          // if origin electron ID does not match previous origin electron ID
          // this hit has contributions from several electrons, ie mixed case
          tag = 0;
          tagged = true;
        }
        prevAncestor = ancestor;
      }
      if (!tagged) {
        // if not tagged, hit was from a single electron
        tag = prevAncestor;
      }
      histograms_.fill("ancestors", tag);
      hitInfo.insert({hit.getID(), std::make_pair(tag, edep)});
    }
  }

  int clusteredHits = 0;

  for (const auto& cl : ecal_clusters) {
    // for each cluster
    // total number of hits coming from electron, index = electron ID
    std::vector<double> n;
    n.resize(nbr_of_electrons_ + 1);
    // total number of energy coming from electron, index = electron ID
    std::vector<double> e;
    e.resize(nbr_of_electrons_ + 1);
    double eSum = 0.;
    double nSum = 0.;

    const auto& hitIDs = cl.getHitIDs();
    for (const auto& id : hitIDs) {
      // for each hit in cluster, find previously stored info
      auto it = hitInfo.find(id);
      if (it != hitInfo.end()) {
        auto t = it->second;
        auto eId = t.first;        // origin electron ID (or 0 for mixed)
        auto energies = t.second;  // energy vector
        n[eId]++;  // increment number of hits coming from this electron
        nSum++;

        double hitESum = 0.;
        for (int i = 1; i < nbr_of_electrons_ + 1; i++) {
          // loop through energy vector
          if (energies[i] > 0.) {
            hitESum += energies[i];
            // add energy from electron i in this hit to total energy from
            // electron i in cluster
            e[i] += energies[i];
          }
        }
        // if mixed hit, add the total energy of this hit to mixed hit energy
        // counter
        if (eId == 0) e[0] += hitESum;
        eSum += hitESum;

        clusteredHits++;
      }
    }

    if (eSum > 0) {
      // get largest energy contribution
      double eMax = *max_element(e.begin(), e.end());
      // energy purity = largest contribution / all energy
      histograms_.fill("energy_percentage", 100. * (eMax / eSum));
      if (e[0] > 0.) histograms_.fill("mixed_hit_energy", 100. * (e[0] / eSum));

      histograms_.fill("total_energy_vs_hits", eSum, cl.getHitIDs().size());
      histograms_.fill("total_energy_vs_purity", eSum, 100. * (eMax / eSum));

      if (nbr_of_electrons_ == 2)
        histograms_.fill("distance_energy_purity", dist, 100. * (eMax / eSum));
    }
    if (nSum > 0) {
      double nMax = *max_element(n.begin(), n.end());
      histograms_.fill("same_ancestor", 100. * (nMax / nSum));
    }
  }

  histograms_.fill("clusterless_hits", (ecal_rec_hits.size() - clusteredHits));
  histograms_.fill("total_rechits_in_event", ecal_rec_hits.size());
  histograms_.fill(
      "clusterless_hits_percentage",
      100. * (ecal_rec_hits.size() - clusteredHits) / ecal_rec_hits.size());
}

}  // namespace dqm
DECLARE_ANALYZER_NS(dqm, EcalClusterAnalyzer)
