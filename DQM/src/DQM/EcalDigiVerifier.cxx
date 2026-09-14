
#include "DQM/EcalDigiVerifier.h"

namespace dqm {

void EcalDigiVerifier::configure(framework::config::Parameters& ps) {
  ecal_sim_hit_coll_ = ps.get<std::string>("ecal_sim_hit_coll");
  ecal_sim_hit_pass_ = ps.get<std::string>("ecal_sim_hit_pass");
  ecal_rec_hit_coll_ = ps.get<std::string>("ecal_rec_hit_coll");
  ecal_rec_hit_pass_ = ps.get<std::string>("ecal_rec_hit_pass");
  ecal_presel_coll_ = ps.get<std::string>("ecal_presel_coll");
  ecal_presel_pass_ = ps.get<std::string>("ecal_presel_pass");
  num_layers_ = ps.get<int>("num_layers");

  return;
}

void EcalDigiVerifier::analyze(const framework::Event& event) {
  // get truth information sorted into an ID based map
  std::vector<ldmx::SimCalorimeterHit> ecal_sim_hits =
      event.getCollection<ldmx::SimCalorimeterHit>(ecal_sim_hit_coll_,
                                                   ecal_sim_hit_pass_);

  // sort sim hits by ID
  std::sort(ecal_sim_hits.begin(), ecal_sim_hits.end(),
            [](const ldmx::SimCalorimeterHit& lhs,
               const ldmx::SimCalorimeterHit& rhs) {
              return lhs.getID() < rhs.getID();
            });

  std::vector<ldmx::EcalHit> ecal_rec_hits = event.getCollection<ldmx::EcalHit>(
      ecal_rec_hit_coll_, ecal_rec_hit_pass_);

  // sort rec hits by ID
  std::sort(ecal_rec_hits.begin(), ecal_rec_hits.end(),
            [](const ldmx::EcalHit& lhs, const ldmx::EcalHit& rhs) {
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
  for (const ldmx::EcalHit& rec_hit : ecal_rec_hits) {
    num_rec_hits++;

    // Building up an ID that has layer + module information
    ldmx::EcalID ecal_id(rec_hit.getID());
    int layer = ecal_id.layer() + 1;
    int module_id = ecal_id.getModuleID() + 1;
    int my_mod_costum_id = layer * 100 + module_id;

    my_costum_mod_ids.push_back(my_mod_costum_id);
    my_costum_mod_ids_set.insert(my_mod_costum_id);

    // Measure the sum energy of all rechits (inc noise)
    total_rec_energy += rec_hit.getEnergy();

    // skip anything that digi flagged as noise
    if (rec_hit.isNoise()) {
      num_noise_hits++;
      histograms_.fill("is_noise_hit", 1.);
      continue;
    }  // end if noise
    histograms_.fill("is_noise_hit", 0.);

    int raw_id = rec_hit.getID();

    // energy weighted sim hit positions
    double sim_pos_x_weighted = 0.;
    double sim_pos_y_weighted = 0.;
    double sim_pos_z_weighted = 0.;

    // get information for this hit
    int num_sim_hits = 0;
    double total_sim_energy_dep = 0.;
    for (const ldmx::SimCalorimeterHit& sim_hit : ecal_sim_hits) {
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
    auto residual_x = rec_hit.getXPos() - sim_pos_x_weighted;
    auto residual_y = rec_hit.getYPos() - sim_pos_y_weighted;
    auto residual_z = rec_hit.getZPos() - sim_pos_z_weighted;
    histograms_.fill("rec_sim_hit_residual_x", residual_x);
    histograms_.fill("rec_sim_hit_residual_y", residual_y);
    histograms_.fill("rec_sim_hit_residual_z", residual_z);
    histograms_.fill("rec_sim_hit_residual_x:layer", residual_x, layer);
    histograms_.fill("rec_sim_hit_residual_y:layer", residual_y, layer);
    histograms_.fill("rec_sim_hit_residual_z:layer", residual_z, layer);
    histograms_.fill("num_sim_hits_per_cell", num_sim_hits);
    histograms_.fill("sim_edep:rec_amplitude", total_sim_energy_dep,
                     rec_hit.getAmplitude());
    histograms_.fill("sim_edep:rec_energy", total_sim_energy_dep,
                     rec_hit.getEnergy());
  }  // end loop on rec hits

  std::map<int, int> module_hits;
  for (const int& my_costum_mod_id : my_costum_mod_ids) {
    module_hits[my_costum_mod_id]++;
  }

  // all modules is 34*7 = 238
  // this would be nice if not hardcoded...
  num_mod_with_0hits = num_layers_ * 7 - my_costum_mod_ids_set.size();

  for (const auto& module_hit : module_hits) {
    if (module_hit.second == 1) {
      num_mod_with_1hits++;
    } else if (module_hit.second == 2) {
      num_mod_with_2hits++;
    } else if (module_hit.second > 2) {
      histograms_.fill("num_hit_if_more_than_2hits", module_hit.second);
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

  // Check if preselection decision exists and fill histogram
  if (event.exists(ecal_presel_coll_, ecal_presel_pass_)) {
    bool presel_passed =
        event.getObject<bool>(ecal_presel_coll_, ecal_presel_pass_);
    histograms_.fill("preselection_passed", presel_passed ? 1. : 0.);
  }

  if (total_rec_energy > 6000.) {
    setStorageHint(framework::HINT_SHOULD_KEEP);
  } else {
    setStorageHint(framework::HINT_SHOULD_DROP);
  }

  return;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalDigiVerifier);
