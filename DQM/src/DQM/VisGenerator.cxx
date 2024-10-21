/**
 * @file VisGenerator.cxx
 */

#include "DQM/VisGenerator.h"

#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimTrackerHit.h"
// #include "DQM/json.hpp"

#include <fstream>
#include <iostream>
using json = nlohmann::json;

namespace dqm {

void VisGenerator::configure(framework::config::Parameters& ps) {
  includeGroundTruth_ = ps.getParameter<bool>("includeGroundTruth");
  originIdAvailable_ = ps.getParameter<bool>("originIdAvailable");
  nbrOfElectrons_ = ps.getParameter<int>("nbrOfElectrons");

  ecalSimHitColl_ = ps.getParameter<std::string>("ecalSimHitColl");
  ecalSimHitPass_ = ps.getParameter<std::string>("ecalSimHitPass");

  visHitOrigin_ = ps.getParameter<bool>("visHitOrigin");
  truthFilename_ = ps.getParameter<std::string>("truthFilename");

  includeEcalRecHits_ = ps.getParameter<bool>("includeEcalRecHits");
  ecalRecHitColl_ = ps.getParameter<std::string>("ecalRecHitColl");
  ecalRecHitPass_ = ps.getParameter<std::string>("ecalRecHitPass");

  includeEcalClusters_ = ps.getParameter<bool>("includeEcalClusters");
  ecalClusterColl_ = ps.getParameter<std::string>("ecalClusterColl");
  ecalClusterPass_ = ps.getParameter<std::string>("ecalClusterPass");

  visLayers_ = ps.getParameter<bool>("visLayers");
  layerFilename_ = ps.getParameter<std::string>("layerFilename");

  filename_ = ps.getParameter<std::string>("filename");

  onlyIncludeEvents_ = ps.getParameter<std::vector<int>>("onlyIncludeEvents");
  // if onlyincludeevents = [ -1 ], user has not given custom parameters --
  // clear
  if (onlyIncludeEvents_.size() == 1 &&
      std::find(onlyIncludeEvents_.begin(), onlyIncludeEvents_.end(), -1) !=
          onlyIncludeEvents_.end()) {
    onlyIncludeEvents_.clear();
  }
  // if excludeevents = [ -1 ], user has not given custom parameters -- clear
  if (excludeEvents_.size() == 1 &&
      std::find(excludeEvents_.begin(), excludeEvents_.end(), -1) !=
          excludeEvents_.end()) {
    excludeEvents_.clear();
  }
  return;
}

void VisGenerator::ecalClusterRecHit(const framework::Event& event,
                                     const std::string& eKey) {
  // GROUND TRUTH HIT INFO
  std::vector<ldmx::SimCalorimeterHit> ecalSimHits;
  if (includeGroundTruth_) {
    ecalSimHits = event.getCollection<ldmx::SimCalorimeterHit>(ecalSimHitColl_,
                                                               ecalSimHitPass_);
  }

  // Initialize different collections based on what we want to visualize
  std::vector<ldmx::EcalCluster> ecalClusters;
  std::vector<ldmx::EcalHit> ecalRecHits;
  std::unordered_map<int, std::vector<int>> hitToCluster;
  if (includeEcalClusters_)
    ecalClusters = event.getCollection<ldmx::EcalCluster>(ecalClusterColl_,
                                                          ecalClusterPass_);
  if (includeEcalRecHits_) {
    ecalRecHits =
        event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);
    if (includeEcalClusters_) hitToCluster.reserve(ecalRecHits.size());
    // ----- HIT ORIGIN VISUALIZATION -----
    if (visHitOrigin_) {
      truth[eKey]["event number"] = event.getEventNumber();
      truth[eKey]["run number"] = runNbr_;
      truth[eKey]["Hits"] = json::object();
    }
  }

  int clusterID = 1;
  double clusterSize = 5.0;  // scale to energy?
  double singleClusterSize = 1.0;
  float clusterHitSize = 2.0;
  float clusterlessHitSize = 1.0;

  if (includeEcalClusters_) {
    // Clusters with zero hits should be removed by clusterproducer, this is for
    // debug
    j[eKey]["Hits"]["empty_clusters"] = json::array();

    for (auto const& cl : ecalClusters) {
      json cluster = json::object();
      cluster["type"] = "Box";
      if (cl.getHitIDs().size() != 0) {  // if cluster is not empty
        // create collection for cluster
        std::string& hex = colors[clusterID % colors.size()];
        std::string cKey = "cluster_" + std::to_string(clusterID);
        j[eKey]["Hits"][cKey] = json::array();
        // create centroid object
        cluster["energy"] = cl.getEnergy();
        cluster["color"] = hex;
        cluster["col"] = colorstrings[clusterID % colorstrings.size()];
        cluster["pos"] = {cl.getCentroidX(), cl.getCentroidY(),
                          cl.getCentroidZ(), clusterSize,
                          clusterSize,       clusterSize};
        if (includeEcalRecHits_) {
          // These need to be initialized as rechits will have this info
          cluster["ID"] = -1;
          if (includeGroundTruth_) {
            if (originIdAvailable_)
              cluster["originID"] = -1;
            else {
              cluster["incidentID"] = -1;
              cluster["E from unknown (%)"] = -1.;
            }
            for (int i = 1; i < nbrOfElectrons_ + 1; i++) {
              cluster[std::string("E from e") + std::to_string(i) + " (%)"] =
                  -1.;
            }
            cluster["immediate_child"] = false;
          }
          for (auto const& clHitID : cl.getHitIDs()) {
            // map hit id to cluster it belongs to
            auto it = hitToCluster.find(clHitID);
            if (it != hitToCluster.end()) {
              auto& vec = it->second;
              vec.push_back(clusterID);
            } else
              hitToCluster.insert({clHitID, {clusterID}});
          }
        }
        j[eKey]["Hits"][cKey].push_back(cluster);
        clusterID++;
      } else {  // empty cluster
        cluster["color"] = "0x000000";
        cluster["pos"] = {cl.getCentroidX(), cl.getCentroidY(),
                          cl.getCentroidZ(), singleClusterSize,
                          singleClusterSize, singleClusterSize};
        j[eKey]["Hits"]["empty_clusters"].push_back(cluster);
      }
    }
  }

  if (includeEcalRecHits_) {
    std::string hit_coll_name;
    if (includeEcalClusters_) {
      hit_coll_name = "clusterless_hits";
      j[eKey]["Hits"]["shared_hits"] = json::array();
    } else
      hit_coll_name = "ecal_rec_hits";
    j[eKey]["Hits"][hit_coll_name] = json::array();

    if (visHitOrigin_) {
      // double e1 = 0;
      // double e2 = 0;
      // make collection for each electron
      for (int i = 1; i < nbrOfElectrons_ + 1; i++) {
        truth[eKey]["Hits"][std::string("e") + std::to_string(i)] =
            json::array();
        ;
      }
      truth[eKey]["Hits"]["mixed"] = json::array();
    }

    for (auto const& hit : ecalRecHits) {
      json h = json::object();
      h["ID"] = hit.getID();
      h["type"] = "Box";
      h["energy"] = hit.getEnergy();
      if (includeGroundTruth_) {
        // find simhit matching to this rechit
        auto it = std::find_if(ecalSimHits.begin(), ecalSimHits.end(),
                               [&hit](const auto& simHit) {
                                 return simHit.getID() == hit.getID();
                               });
        if (it != ecalSimHits.end()) {
          std::vector<double> e;
          e.resize(nbrOfElectrons_ + 1);
          double eSum = 0.;
          int tag = 0;
          bool immediate_child = true;
          if (originIdAvailable_) {
            h["originID"] = json::array();
            for (int i = 0; i < it->getNumberOfContribs(); i++) {
              // for each contrib
              auto c = it->getContrib(i);
              h["originID"].push_back(c.originID);
              if (i != 0 && c.originID != tag)
                tag = 0;  // mixed hit
              else
                tag = c.originID;
              e[c.originID] += c.edep;  // add edep to correct electron
              eSum += c.edep;
              if (c.incidentID > nbrOfElectrons_) {
                immediate_child =
                    false;  // not an immediate child of initial electrons
              }
            }
          } else {
            double eUnknown = 0.;
            h["incidentID"] = json::array();
            for (int i = 0; i < it->getNumberOfContribs(); i++) {
              auto c = it->getContrib(i);
              h["incidentID"].push_back(c.incidentID);
              if (i != 0 && c.incidentID != tag)
                tag = 0;  // mixed hit
              else
                tag = c.incidentID;
              if (c.incidentID > nbrOfElectrons_) {
                // if incident != initial electron
                // we don't know which electron this contrib comes from
                eUnknown += c.edep;
                immediate_child = false;
              } else
                e[c.incidentID] += c.edep;
              eSum += c.edep;
            }
            if (eSum > 0) {
              h["E from unknown (%)"] = 100. * eUnknown / eSum;
            }
          }
          if (eSum > 0) {
            for (int i = 1; i < nbrOfElectrons_ + 1; i++) {
              h[std::string("E from e") + std::to_string(i) + " (%)"] =
                  100. * e[i] / eSum;
            }
          }
          h["immediate_child"] = immediate_child;

          if (visHitOrigin_) {
            // add hit to truth json
            clusterHitSize = 2.0;
            json t = json::object();
            t["ID"] = hit.getID();
            t["type"] = "Box";
            t["energy"] = hit.getEnergy();
            t["originID"] = tag;
            t["immediate_child"] = immediate_child;
            t["pos"] = {hit.getXPos(),  hit.getYPos(),  hit.getZPos(),
                        clusterHitSize, clusterHitSize, clusterHitSize};
            t["color"] = colors[tag % nbrOfElectrons_];
            if (tag == 0)
              truth[eKey]["Hits"]["mixed"].push_back(t);
            else
              truth[eKey]["Hits"][std::string("e") + std::to_string(tag)]
                  .push_back(t);
          }
        }
      }
      if (includeEcalClusters_) {
        // Add hit to cluster it belongs to
        auto it = hitToCluster.find(hit.getID());
        if (it != hitToCluster.end()) {  // if hit is associated to a cluster
          auto& vec = it->second;
          if (vec.size() != 1) {  // if hit is shared between many clusters
            // Create a shared hit object
            json sh = json::object();
            clusterHitSize = 4.0;
            sh["ID"] = hit.getID();
            sh["type"] = "Box";
            sh["energy"] = hit.getEnergy();
            sh["color"] = "0xA9A9A9";
            sh["pos"] = {hit.getXPos(),  hit.getYPos(),  hit.getZPos(),
                         clusterHitSize, clusterHitSize, clusterHitSize};
            j[eKey]["Hits"]["shared_hits"].push_back(sh);
          }
          // add hit to cluster collection
          // color will automatically be set to same as centroid
          clusterHitSize = 2.0;
          h["pos"] = {hit.getXPos(),  hit.getYPos(),  hit.getZPos(),
                      clusterHitSize, clusterHitSize, clusterHitSize};
          for (int i = 0; i < vec.size(); i++) {
            std::string cKey = "cluster_" + std::to_string(vec[0]);
            j[eKey]["Hits"][cKey].push_back(h);
          }
          continue;
        }
      }
      // if hit is not associated to a cluster
      h["color"] = "0xFF0000";
      h["pos"] = {hit.getXPos(),      hit.getYPos(),      hit.getZPos(),
                  clusterlessHitSize, clusterlessHitSize, clusterlessHitSize};
      j[eKey]["Hits"][hit_coll_name].push_back(h);
    }
  }

  // For scoring plane hits
  // auto
  // ecalSpHits{event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits")};
  // j[eKey]["Hits"]["plane_hits"] = json::array();
  // truth[eKey]["Hits"]["plane_hits"] = json::array();
  // for (ldmx::SimTrackerHit &spHit : ecalSpHits) {
  //   // std::cout << hit_id.plane() << std::endl;
  //   if ((spHit.getTrackID() == 1 || spHit.getTrackID() == 2)) {
  //     json ph = json::object();
  //     ph["ID"] = spHit.getTrackID();
  //     ph["type"] = "Box";
  //     std::vector<float> pos = spHit.getPosition();
  //     ph["pos"] = { double(pos[0]), double(pos[1]),
  //     double(pos[2]), 5.0, 5.0, 5.0 }; if (spHit.getTrackID() == 1) {
  //       ph["color"] = "0xEADE76";
  //     } else {
  //       ph["color"] = "0x76BDEA";
  //     }
  //     j[eKey]["Hits"]["plane_hits"].push_back(ph);
  //     truth[eKey]["Hits"]["plane_hits"].push_back(ph);
  //     // std::cout << hit_id.plane() << std::endl;
  //     // std::cout << pos[0] << ":" << pos[1] << ":" << pos[2] << std::endl;
  //   }
  //   if (hit_id.plane() != 31 || spHit.getMomentum()[2] <= 0) continue;

  //   if (spHit.getTrackID() == recoilTrackID) {
  //     if (sqrt(pow(spHit.getMomentum()[0], 2) +
  //               pow(spHit.getMomentum()[1], 2) +
  //               pow(spHit.getMomentum()[2], 2)) > pmax) {
  //       recoilP = spHit.getMomentum();
  //       recoilPos = spHit.getPosition();
  //       pmax = sqrt(pow(recoilP[0], 2) + pow(recoilP[1], 2) +
  //                   pow(recoilP[2], 2));
  //     }
  //   }
  // }
}

void VisGenerator::groundTruthTracks(const framework::Event& event,
                                     const std::string& eKey) {
  j[eKey]["Tracks"]["ground_truth_tracks"] = json::array();
  if (visHitOrigin_) {
    truth[eKey]["Tracks"] = json::object();
    truth[eKey]["Tracks"]["ground_truth_tracks"] = json::array();
  }
  auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};
  for (const auto& it : particle_map) {
    const auto& start = it.second.getVertex();
    const auto& end = it.second.getEndPoint();
    // only visualize incoming electrons
    if (it.second.getParents()[0] == 0) {
      json track = json::object();
      track["trackID"] = it.first;
      track["parentID"] = {it.second.getParents()};
      track["pos"] = {{start[0], start[1], start[2]}, {end[0], end[1], end[2]}};
      j[eKey]["Tracks"]["ground_truth_tracks"].push_back(track);
      if (visHitOrigin_) {
        truth[eKey]["Tracks"]["ground_truth_tracks"].push_back(track);
      }
    }
  }
}

static bool compZ(const ldmx::EcalHit& a, const ldmx::EcalHit& b) {
  return a.getZPos() < b.getZPos();
}

void VisGenerator::extractLayers(const framework::Event& event,
                                 const std::string& eKey) {
  layer[eKey]["event number"] = event.getEventNumber();
  layer[eKey]["run number"] = runNbr_;
  layer[eKey]["Hits"] = json::object();
  std::vector<double> layerThickness = {2.,   3.5,  5.3,  5.3, 5.3, 5.3,
                                        5.3,  5.3,  5.3,  5.3, 5.3, 10.5,
                                        10.5, 10.5, 10.5, 10.5};
  std::vector<ldmx::EcalHit> ecalRecHits =
      event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);
  std::sort(ecalRecHits.begin(), ecalRecHits.end(), compZ);
  int layerTag = 0;
  double z0 = ecalRecHits[0].getZPos();
  double layerZ = ecalRecHits[0].getZPos();
  std::string hex = colors[layerTag % colors.size()];
  std::string col = colorstrings[layerTag % colorstrings.size()];
  std::string layerKey = "layer_0";
  layer[eKey]["Hits"][layerKey] = json::array();
  double air = 10.;
  for (const auto& hit : ecalRecHits) {
    if (layerTag != 17 &&
        hit.getZPos() > layerZ + layerThickness[layerTag] + air) {
      layerZ = hit.getZPos();
      layerTag++;
      layerKey = "layer_" + std::to_string(layerTag);
      hex = colors[layerTag % colors.size()];
      col = colorstrings[layerTag % colorstrings.size()];
      layer[eKey]["Hits"][layerKey] = json::array();
    }
    json h = json::object();
    h["ID"] = hit.getID();
    h["type"] = "Box";
    h["energy"] = hit.getEnergy();
    h["color"] = hex;
    h["col"] = col;
    h["pos"] = {hit.getXPos(), hit.getYPos(), hit.getZPos(), 2.0, 2.0, 2.0};
    layer[eKey]["Hits"][layerKey].push_back(h);
  }
}

void VisGenerator::analyze(const framework::Event& event) {
  // Check if event is in include only or exclude lists
  if (!onlyIncludeEvents_.empty() &&
      std::find(onlyIncludeEvents_.begin(), onlyIncludeEvents_.end(),
                event.getEventNumber()) == onlyIncludeEvents_.end())
    return;
  if (!excludeEvents_.empty() &&
      std::find(excludeEvents_.begin(), excludeEvents_.end(),
                event.getEventNumber()) != excludeEvents_.end())
    return;
  // ----- EVENT HEADER -----
  const std::string eKey =
      "EVENT_KEY_" + std::to_string(event.getEventNumber());
  j[eKey]["event number"] = event.getEventNumber();
  j[eKey]["run number"] = runNbr_;

  // checks
  if (!originIdAvailable_)
    visHitOrigin_ = false;  // cannot vis hit origin if no origin ID available
  if (includeGroundTruth_ && nbrOfElectrons_ == -1)
    return;  // for ground truth, need nbr of electrons

  // ----- HITS -----
  j[eKey]["Hits"] = json::object();

  // CLUSTER-RECHIT CONNECTION
  if (includeEcalClusters_ || includeEcalRecHits_)
    ecalClusterRecHit(event, eKey);

  if (visLayers_) extractLayers(event, eKey);

  // ----- TRACKS -----
  j[eKey]["Tracks"] = json::object();

  // GROUND TRUTH (SIMULATED) PATHS
  if (includeGroundTruth_) groundTruthTracks(event, eKey);
  return;
}

void VisGenerator::onNewRun(const ldmx::RunHeader& runHeader) {
  runNbr_ = runHeader.getRunNumber();
}

void VisGenerator::onProcessEnd() {
  // Write to file
  std::ofstream file(filename_);
  file << std::setw(2) << j << std::endl;
  file.close();
  if (visHitOrigin_) {
    std::ofstream truthfile(truthFilename_);
    truthfile << std::setw(2) << truth << std::endl;
    truthfile.close();
  }

  if (visLayers_) {
    std::ofstream layerfile(layerFilename_);
    layerfile << std::setw(2) << layer << std::endl;
    layerfile.close();
  }
  return;
};

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, VisGenerator)
