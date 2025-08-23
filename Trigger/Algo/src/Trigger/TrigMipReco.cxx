#include "Trigger/TrigMipReco.h"

namespace trigger {

void TrigMipReco::onNewRun(const ldmx::RunHeader& rh) {
  profiling_map_["processing_time_"] = 0;
}

void TrigMipReco::configure(framework::config::Parameters& ps) {
  hit_coll_name_ = ps.get<std::string>("hit_coll_name");
  pass_coll_name_ = ps.get<std::string>("pass_coll_name");
  hit_coll_passname_ = ps.get<std::string>("hit_coll_passname");
  calorimeter_type_is_hcal_ = ps.get<bool>("calorimeter_type_is_hcal");

  max_layer_ = ps.get<int>("max_layer", 32);
  // mm
  search_radius_ = ps.get<float>("search_radius", 50.0f);
  min_track_length_ = ps.get<int>("min_track_length", 5);
  // MeV; Change as needed
  isolation_e_cut_ = ps.get<float>("isolation_e_cut", 180.0f);
  hole_fraction_max_ = ps.get<float>("hole_fraction_max", 0.2f);

  if (calorimeter_type_is_hcal_) {
    // MIP peak is 10-11 MeV
    hcal_min_energy_ = ps.get<float>("hcal_min_energy", 8.0f);
  } else {  // ECAL
    // MIP peak around 17 MeV
    ecal_min_energy_ = ps.get<float>("ecal_min_energy", 3.0f);
    ecal_max_energy_ = ps.get<float>("ecal_max_energy", 26.0f);
  }
}

void TrigMipReco::produce(framework::Event& event) {
  auto start = std::chrono::high_resolution_clock::now();
  nevents_++;

  if (!event.exists(hit_coll_name_, hit_coll_passname_)) return;

  const auto calo_hits = event.getObject<std::vector<TrigCaloHit>>(
      hit_coll_name_, hit_coll_passname_);

  if (calorimeter_type_is_hcal_) {  // HCAL MIP Reconstruction
    std::vector<TrigCaloHit> sorted_hits;
    int even_matrix[24][5] = {};
    int even_start[5] = {99, 99, 99, 99, 99};
    int even_end[5] = {};
    int even_counts[5] = {};
    int odd_matrix[24][5] = {};
    int odd_start[5] = {99, 99, 99, 99, 99};
    int odd_end[5] = {};
    int odd_counts[5] = {};
    // Start in first 5 layers
    constexpr int layer_start = 80;

    for (const auto& tp : calo_hits) {
      if (tp.section() > 0 || tp.energy() < hcal_min_energy_ ||
          tp.layer() > 47) {
        continue;
      }

      sorted_hits.push_back(tp);
      const int layer_index = tp.layer() / 2;
      const int strip = tp.strip();
      if (tp.layer() % 2) {
        odd_matrix[layer_index][strip] = 1;
        odd_start[strip] = std::min(odd_start[strip], tp.layer());
        odd_end[strip] = std::max(odd_end[strip], tp.layer());
      } else {
        even_matrix[layer_index][strip] = 1;
        even_start[strip] = std::min(even_start[strip], tp.layer());
        even_end[strip] = std::max(even_end[strip], tp.layer());
      }
    }

    for (int i = 0; i < 24; i++) {
      for (int j = 0; j < 5; j++) {
        if (even_matrix[i][j]) {
          even_counts[j]++;
        }
        if (odd_matrix[i][j]) {
          odd_counts[j]++;
        }
      }
    }

    // straight MIP reco
    std::vector<TrigMip> mips;
    // 5 elements in the even/odd_start matrices
    for (int i = 0; i < 5; i++) {
      if (odd_start[i] < layer_start) {
        TrigMip m;
        m.setStartLayer(odd_start[i]);
        m.setEndLayer(odd_end[i]);
        m.setNHits(odd_counts[i]);
        m.setLength(odd_end[i] - odd_start[i] + 1);
        int holes = m.length() / 2 - m.nHits();
        if (holes < 0) {
          holes = 0;
        }
        m.setNHoles(holes);
        mips.push_back(m);
      }
      if (even_start[i] < layer_start) {
        TrigMip m;
        m.setStartLayer(even_start[i]);
        m.setEndLayer(even_end[i]);
        m.setNHits(even_counts[i]);
        m.setLength(even_end[i] - even_start[i] + 1);
        int holes = m.length() / 2 - m.nHits();
        if (holes < 0) {
          holes = 0;
        }
        m.setNHoles(holes);
        mips.push_back(m);
      }
    }

    std::sort(mips.begin(), mips.end());
    event.add(pass_coll_name_, mips);

    auto end = std::chrono::high_resolution_clock::now();
    auto time_diff = end - start;
    processing_time_ +=
        std::chrono::duration<double, std::milli>(time_diff).count();

    return;
    // ECAL MIP Reconstruction
  } else {
    const float radius_cut_2 = search_radius_ * search_radius_;
    std::map<int, std::vector<TrigCaloHit>> layer_hits;
    std::set<const TrigCaloHit*> used_hits;
    std::vector<std::vector<const TrigCaloHit*>> candidate_tracks;
    std::map<const TrigCaloHit*, size_t> hit_to_best_track;

    // Filter for section = 0 and hits < 33 layers
    for (const auto& hit : calo_hits) {
      if (hit.section() > 0 || hit.layer() > max_layer_) continue;
      layer_hits[hit.layer()].push_back(hit);
    }

    // Find mip seeds
    for (const auto& [seed_layer, seeds] : layer_hits) {
      for (const auto& seed : seeds) {
        // Skip if hit already used or outside mip energy range
        if (used_hits.count(&seed) || seed.energy() < ecal_min_energy_ ||
            seed.energy() > ecal_max_energy_) {
          continue;
        }

        std::vector<const TrigCaloHit*> track{&seed};
        // Most recent hit in track
        const TrigCaloHit* last = &seed;
        int holes = 0;
        float growth_factor = 1.0f;

        // Look layer by layer for next hit within dR
        for (int l = seed.layer() + 1; l <= max_layer_; ++l) {
          const TrigCaloHit* best_hit = nullptr;
          // Grow search window if there is a hole
          float best_d_r_2 = radius_cut_2 * growth_factor * growth_factor;

          for (const auto& cand : layer_hits[l]) {
            if (used_hits.count(&cand) || cand.energy() < ecal_min_energy_ ||
                cand.energy() > ecal_max_energy_) {
              continue;
            }

            // Corrects for layer shift in x-direction, calculated as 4.82 mm
            const float layer_shift_last =
                (last->layer() % 2 == 0) ? 0.0f : 4.82f;
            const float layer_shift_cand =
                (cand.layer() % 2 == 0) ? 0.0f : 4.82f;
            const float dx = (cand.positionX() - layer_shift_cand) -
                             (last->positionX() - layer_shift_last);
            const float dy = cand.positionY() - last->positionY();
            const float d_r_2 = dx * dx + dy * dy;

            if (d_r_2 < best_d_r_2) {
              best_d_r_2 = d_r_2;
              // Closest unused hit in next layer
              best_hit = &cand;
            }
          }

          if (best_hit) {
            // Builds track from best hits
            track.push_back(best_hit);
            last = best_hit;
            holes = 0;
            // Reset search window
            growth_factor = 1.0f;
          } else {
            holes++;
            // Keep expanding
            growth_factor = static_cast<float>(holes + 1);
          }
        }

        bool is_isolated = true;
        if (track.size() >= min_track_length_) {
          // Isolation area energy check
          for (const auto* hit : track) {
            const int layer = hit->layer();
            const float hit_x = hit->positionX();
            const float hit_y = hit->positionY();
            float sum_e = 0.0f;

            for (const auto& cand : layer_hits[layer]) {
              // Skips self
              if (&cand == hit) continue;

              const float dx = cand.positionX() - hit_x;
              const float dy = cand.positionY() - hit_y;
              const float d_r_2 = dx * dx + dy * dy;

              if (d_r_2 < radius_cut_2) {
                sum_e += cand.energy();
              }
            }

            if (sum_e >= isolation_e_cut_) {
              is_isolated = false;
              // Adds used hits to vector so they cannot be used again
              used_hits.insert(hit);
              break;
            }
          }

          if (!is_isolated) {
            // Reject track if any hit is not isolated
            continue;
          }

          const size_t i = candidate_tracks.size();
          candidate_tracks.push_back(track);

          for (const auto* hit : track) {
            if (!hit_to_best_track.count(hit) ||
                candidate_tracks[i].size() >
                    candidate_tracks[hit_to_best_track[hit]].size()) {
              hit_to_best_track[hit] = i;
              // Adds used hits to vector so they cannot be used again
              used_hits.insert(hit);
            }
          }
        }
      }
    }

    std::set<size_t> valid_track_i_ds;
    for (const auto& [hit, idx] : hit_to_best_track) {
      valid_track_i_ds.insert(idx);
    }

    std::vector<TrigMip> mips;
    for (const size_t idx : valid_track_i_ds) {
      const auto& track = candidate_tracks[idx];
      TrigMip mip;
      mip.setStartLayer(track.front()->layer());
      mip.setEndLayer(track.back()->layer());
      mip.setNHits(track.size());
      mip.setLength(track.back()->layer() - track.front()->layer());
      int holes = mip.length() - mip.nHits();
      if (holes < 0) {
        holes = 0;
      }
      mip.setNHoles(holes);

      const float hole_fraction =
          static_cast<float>(mip.nHoles()) / mip.length();
      // Remove mip tracks with hole fraction > 0.2
      if (hole_fraction >= hole_fraction_max_) continue;

      float total_isolation_e_sum = 0.0f;
      for (const auto* hit : track) {
        const int layer = hit->layer();
        const float hit_x = hit->positionX();
        const float hit_y = hit->positionY();
        for (const auto& cand : layer_hits[layer]) {
          if (&cand == hit) continue;
          const float dx = cand.positionX() - hit_x;
          const float dy = cand.positionY() - hit_y;
          const float d_r_2 = dx * dx + dy * dy;
          if (d_r_2 < radius_cut_2) {
            total_isolation_e_sum += cand.energy();
          }
        }
      }
      mip.setSumEinIsolationRegion(total_isolation_e_sum);
      mips.push_back(mip);
    }

    std::sort(mips.begin(), mips.end());
    event.add(pass_coll_name_, mips);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto time_diff = end - start;
  processing_time_ +=
      std::chrono::duration<double, std::milli>(time_diff).count();
}

void TrigMipReco::onProcessEnd() {
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(3)
                 << processing_time_ / nevents_ << " ms";
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::TrigMipReco);
