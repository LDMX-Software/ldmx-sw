/**
 * @file HcalWABVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal.
 * @author Sophie Middleton, Caltech
 */

#include "Hcal/HcalWABVetoProcessor.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include <numeric>

#include "DetDescr/HcalID.h"
using namespace std;
namespace hcal {

HcalWABVetoProcessor::HcalWABVetoProcessor(const std::string& name,
                                           framework::Process& process)
    : Producer(name, process) {}

void HcalWABVetoProcessor::configure(
    framework::config::Parameters& parameters) {
  maxtotal_energy_compare_ = parameters.get<double>("max_total_energy_compare");
  mintotal_energy_compare_ = parameters.get<double>("min_total_energy_compare");
  maxn_clusters_ = parameters.get<double>("n_clusters");
  max_mean_hits_per_cluster_ = parameters.get<double>("mean_hits_per_cluster");
  max_mean_energy_per_cluster_ =
      parameters.get<double>("mean_energy_per_cluster");
  output_coll_name_ = parameters.get<std::string>("output_coll_name");
  input_hcal_cluster_coll_name_ =
      parameters.get<std::string>("input_hcal_cluster_coll_name");
  input_hcal_hit_coll_name_ =
      parameters.get<std::string>("input_hcal_hit_coll_name");
  input_ecal_hit_coll_name_ =
      parameters.get<std::string>("input_ecal_hit_coll_name");
  hcal_hit_passname_ = parameters.get<std::string>("hcal_hit_passname");
  ecal_hit_passname_ = parameters.get<std::string>("ecal_hit_passname");
  hcal_cluster_passname_ = parameters.get<std::string>("hcal_cluster_passname");
}

void HcalWABVetoProcessor::produce(framework::Event& event) {
  // Get the collection of sim particles from the event
  // HCAL:
  const std::vector<ldmx::HcalHit> hcal_rec_hits =
      event.getCollection<ldmx::HcalHit>(input_hcal_hit_coll_name_,
                                         hcal_hit_passname_);
  // ECAL:
  const std::vector<ldmx::EcalHit> ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(input_ecal_hit_coll_name_,
                                         ecal_hit_passname_);

  // Clusters:
  const std::vector<ldmx::HcalCluster> hcal_clusters =
      event.getCollection<ldmx::HcalCluster>(input_hcal_cluster_coll_name_,
                                             hcal_cluster_passname_);

  // Loop over all of the Hcal hits_ and calculate to total photoelectrons
  // in the event.
  float total_hcal_energy{0};
  float total_ecal_energy{0};
  float max_pe{-1000};
  const ldmx::HcalHit* max_pe_hit = nullptr;
  for (const ldmx::HcalHit& hcal_hit : hcal_rec_hits) {
    if (hcal_hit.isNoise() == 0) {
      total_hcal_energy += hcal_hit.getPE();
    }

    // Find the maximum PE in the list
    if (max_pe < hcal_hit.getPE()) {
      max_pe = hcal_hit.getPE();
      max_pe_hit = const_cast<ldmx::HcalHit*>(&hcal_hit);
    }
  }

  for (const ldmx::EcalHit& ecal_hit : ecal_rec_hits) {
    if (ecal_hit.isNoise() == 0) {
      total_ecal_energy += ecal_hit.getEnergy();
    }
  }
  std::vector<double> nhits;
  std::vector<double> energies;
  unsigned int n_clusters = 0;
  for (const ldmx::HcalCluster& hcal_cluster : hcal_clusters) {
    n_clusters += 1;
    energies.push_back(hcal_cluster.getEnergy());
    nhits.push_back(hcal_cluster.getNHits());
  }

  double mean_energy =
      std::accumulate(energies.begin(), energies.end(), 0.0) / energies.size();
  double mean_nhits =
      std::accumulate(nhits.begin(), nhits.end(), 0.0) / nhits.size();
  bool passes_energy_combo =
      (((total_ecal_energy + total_hcal_energy) < maxtotal_energy_compare_));
  bool passesn_clusters = (n_clusters < maxn_clusters_);
  bool passes_n_hits =
      ((mean_nhits < max_mean_hits_per_cluster_) or isnan(mean_nhits));
  bool passes_energy =
      ((mean_energy < max_mean_energy_per_cluster_) or isnan(mean_energy));

  // total veto:
  bool passes_veto = (passes_energy_combo and passesn_clusters and
                      passes_n_hits and passes_energy);
  // set result:
  ldmx::HcalVetoResult result;
  result.setVetoResult(passes_veto);
  result.setMaxPEHit(*max_pe_hit);
  if (passes_veto) {
    setStorageHint(framework::HINT_SHOULD_KEEP);

  } else {
    setStorageHint(framework::HINT_SHOULD_DROP);
  }

  event.add(output_coll_name_, result);
}
}  // namespace hcal

DECLARE_PRODUCER(hcal::HcalWABVetoProcessor);
