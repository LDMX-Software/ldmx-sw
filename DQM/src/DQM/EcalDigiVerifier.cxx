
#include "DQM/EcalDigiVerifier.h"

namespace dqm {

void EcalDigiVerifier::configure(framework::config::Parameters &ps) {
  ecal_sim_hit_coll_ = ps.get<std::string>("ecal_sim_hit_coll");
  ecal_sim_hit_pass_ = ps.get<std::string>("ecal_sim_hit_pass");
  ecal_rec_hit_coll_ = ps.get<std::string>("ecal_rec_hit_coll");
  ecal_rec_hit_pass_ = ps.get<std::string>("ecal_rec_hit_pass");
  num_layers_ = ps.get<int>("num_layers");

  return;
}

void EcalDigiVerifier::analyze(const framework::Event &event) {
  // get truth information sorted into an ID based map
  std::vector<ldmx::SimCalorimeterHit> ecal_sim_hits =
      event.getCollection<ldmx::SimCalorimeterHit>(ecal_sim_hit_coll_,
                                                   ecal_sim_hit_pass_);

  // sort sim hits by ID
  std::sort(ecal_sim_hits.begin(), ecal_sim_hits.end(),
            [](const ldmx::SimCalorimeterHit &lhs,
               const ldmx::SimCalorimeterHit &rhs) {
              return lhs.getID() < rhs.getID();
            });

  std::vector<ldmx::EcalHit> ecal_rec_hits = event.getCollection<ldmx::EcalHit>(
      ecal_rec_hit_coll_, ecal_rec_hit_pass_);

  // sort rec hits by ID
  std::sort(ecal_rec_hits.begin(), ecal_rec_hits.end(),
            [](const ldmx::EcalHit &lhs, const ldmx::EcalHit &rhs) {
              return lhs.getID() < rhs.getID();
            });

  int num_rec_hits{0};
  int num_noise_hits{0};
  double total_rec_energy{0.};
  int num_mod_with_0hits{0};
  int num_mod_with_1hits{0};
  int num_mod_with_2hits{0};
  int num_mod_with_more_than_2hits{0};
  std::vector<int> my_costum_mod_ids;
  // I need a set for the case when there are repeated elements
  std::set<int> my_costum_mod_ids_set;

  // Loop on the ecal rechits
  for (const ldmx::EcalHit &recHit : ecal_rec_hits) {
    num_rec_hits++;

    // Building up an ID that has layer + module information
    ldmx::EcalID ecal_id(recHit.getID());
    int layer = ecal_id.layer() + 1;
    int module_id = ecal_id.getModuleID() + 1;
    int my_mod_costum_id = layer * 100 + module_id;

    my_costum_mod_ids.push_back(my_mod_costum_id);
    my_costum_mod_ids_set.insert(my_mod_costum_id);

    // Measure the sum energy of all rechits (inc noise)
    total_rec_energy += recHit.getEnergy();

    // skip anything that digi flagged as noise
    if (recHit.isNoise()) {
      num_noise_hits++;
      histograms_.fill("is_noise_hit", 1.);
      continue;
    }  // end if noise
    histograms_.fill("is_noise_hit", 0.);

    int raw_id = recHit.getID();

    // energy weighted sim hit positions
    double sim_pos_x_weighted = 0.;
    double sim_pos_y_weighted = 0.;
    double sim_pos_z_weighted = 0.;

    // get information for this hit
    int num_sim_hits = 0;
    double total_sim_energy_dep = 0.;
    for (const ldmx::SimCalorimeterHit &sim_hit : ecal_sim_hits) {
      if (raw_id == sim_hit.getID()) {
        num_sim_hits += sim_hit.getNumberOfContribs();
        total_sim_energy_dep += sim_hit.getEdep();
        sim_pos_x_weighted += sim_hit.getPosition()[0] * sim_hit.getEdep();
        sim_pos_y_weighted += sim_hit.getPosition()[1] * sim_hit.getEdep();
        sim_pos_z_weighted += sim_hit.getPosition()[2] * sim_hit.getEdep();

      } else if (raw_id < sim_hit.getID()) {
        // later sim hits - all done
        break;
      }
    }  // end loop on sim hits

    sim_pos_x_weighted /= total_sim_energy_dep;
    sim_pos_y_weighted /= total_sim_energy_dep;
    sim_pos_z_weighted /= total_sim_energy_dep;
    auto residualX = recHit.getXPos() - sim_pos_x_weighted;
    auto residualY = recHit.getYPos() - sim_pos_y_weighted;
    auto residualZ = recHit.getZPos() - sim_pos_z_weighted;
    histograms_.fill("rec_sim_hit_residual_x", residualX);
    histograms_.fill("rec_sim_hit_residual_y", residualY);
    histograms_.fill("rec_sim_hit_residual_z", residualZ);
    histograms_.fill("rec_sim_hit_residual_x:layer", residualX, layer);
    histograms_.fill("rec_sim_hit_residual_y:layer", residualY, layer);
    histograms_.fill("rec_sim_hit_residual_z:layer", residualZ, layer);
    histograms_.fill("num_sim_hits_per_cell", num_sim_hits);
    histograms_.fill("sim_edep:rec_amplitude", total_sim_energy_dep,
                     recHit.getAmplitude());
    histograms_.fill("sim_edep:rec_energy", total_sim_energy_dep,
                     recHit.getEnergy());
  }  // end loop on rec hits

  std::map<int, int> module_hits;
  for (const int &my_costum_mod_id : my_costum_mod_ids) {
    module_hits[my_costum_mod_id]++;
  }

  // all modules is 34*7 = 238
  // this would be nice if not hardcoded...
  num_mod_with_0hits = num_layers_ * 7 - my_costum_mod_ids_set.size();

  for (const auto &moduleHit : module_hits) {
    if (moduleHit.second == 1) {
      num_mod_with_1hits++;
    } else if (moduleHit.second == 2) {
      num_mod_with_2hits++;
    } else if (moduleHit.second > 2) {
      histograms_.fill("num_hit_if_more_than_2hits", moduleHit.second);
      num_mod_with_more_than_2hits++;
    }
  }

  histograms_.fill("num_rec_hits", num_rec_hits);
  histograms_.fill("num_noise_hits", num_noise_hits);
  histograms_.fill("total_rec_energy", total_rec_energy);

  histograms_.fill("num_mod_with_0hits", num_mod_with_0hits);
  // only fill the histograms in the case there are hits, otherwise it goes to
  // the other categories
  if (num_mod_with_1hits > 0)
    histograms_.fill("num_mod_with_1hits", num_mod_with_1hits);
  if (num_mod_with_2hits > 0)
    histograms_.fill("num_mod_with_2hits", num_mod_with_2hits);
  if (num_mod_with_more_than_2hits > 0)
    histograms_.fill("num_mod_with_more_than_2hits",
                     num_mod_with_more_than_2hits);

  if (total_rec_energy > 6000.) {
    setStorageHint(framework::hint_should_keep);
  } else {
    setStorageHint(framework::hint_should_drop);
  }

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalDigiVerifier);
