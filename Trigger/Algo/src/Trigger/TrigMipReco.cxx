#include "Trigger/TrigMipReco.h"

namespace trigger {

void TrigMipReco::configure(framework::config::Parameters& ps) {
  hit_coll_name_ = ps.getParameter<std::string>("hit_coll_name");
  pass_coll_name_ = ps.getParameter<std::string>("pass_coll_name");
  hit_coll_passname_ = ps.getParameter<std::string>("hit_coll_passname");
  calorimeter_type_is_hcal_ = ps.getParameter<bool>("calorimeter_type_is_hcal");
  if (calorimeter_type_is_hcal_) {
    min_energy_ = 8.0f;  // MIP peak is 10-11
  } else {               // ECAL
    min_energy_ = 3.0f;  // MIP peak around 17
    max_energy_ = 26.0f;
  }
}

void TrigMipReco::produce(framework::Event& event) {
  if (!event.exists(hit_coll_name_, hit_coll_passname_)) return;
  auto caloHits{event.getObject<TrigCaloHitCollection>(hit_coll_name_,
                                                       hit_coll_passname_)};

  if (calorimeter_type_is_hcal_) {  // HCAL MIP Reconstruction
    TrigCaloHitCollection sortedHits;
    int evenMatrix[24][5] = {};
    int evenStart[5] = {99, 99, 99, 99, 99};
    int evenEnd[5] = {};
    int evenCounts[5] = {};
    int oddMatrix[24][5] = {};
    int oddStart[5] = {99, 99, 99, 99, 99};
    int oddEnd[5] = {};
    int oddCounts[5] = {};
    for (const auto& tp : caloHits) {
      if (tp.section() > 0 || tp.energy() < min_energy_ || tp.layer() > 47)
        continue;
      sortedHits.push_back(tp);
      if (tp.layer() % 2) {
        oddMatrix[tp.layer() / 2][tp.strip()] = 1;
        if (tp.layer() < oddStart[tp.strip()])
          oddStart[tp.strip()] = tp.layer();
        if (tp.layer() > oddEnd[tp.strip()]) oddEnd[tp.strip()] = tp.layer();
      } else {
        evenMatrix[tp.layer() / 2][tp.strip()] = 1;
        if (tp.layer() < evenStart[tp.strip()])
          evenStart[tp.strip()] = tp.layer();
        if (tp.layer() > evenEnd[tp.strip()]) evenEnd[tp.strip()] = tp.layer();
      }
    }
    for (int i = 0; i < 24; i++) {
      for (int j = 0; j < 5; j++) {
        if (evenMatrix[i][j]) {
          evenCounts[j]++;
        }
        if (oddMatrix[i][j]) {
          oddCounts[j]++;
        }
      }
    }

    // straight MIP reco
    TrigMipCollection mips;
    for (int i = 0; i < 5; i++) {
      if (oddStart[i] < 80) {  // start in first 5 layers
        TrigMip m;
        m.setStartLayer(oddStart[i]);
        m.setEndLayer(oddEnd[i]);
        m.setNHits(oddCounts[i]);
        m.setLength(oddEnd[i] - oddStart[i] + 1);
        m.setNHoles(m.length() / 2 - m.nHits());
        mips.push_back(m);
      }
      if (evenStart[i] < 80) {  // start in first 5 layers
        TrigMip m;
        m.setStartLayer(evenStart[i]);
        m.setEndLayer(evenEnd[i]);
        m.setNHits(evenCounts[i]);
        m.setLength(evenEnd[i] - evenStart[i] + 1);
        m.setNHoles(m.length() / 2 - m.nHits());
        mips.push_back(m);
      }
    }

    std::sort(mips.begin(), mips.end());

    event.add(pass_coll_name_, mips);
  } else {                 // ECAL MIP Reconstruction
    float radiusCut = 50;  // mm
    int maxLayer = 32;
    int minTrackLength = 5;  // Adjustable
    const float radiusCut2 = radiusCut * radiusCut;

    std::map<int, std::vector<TrigCaloHit>> layerHits;
    std::set<const TrigCaloHit*> usedHits;
    std::vector<std::vector<const TrigCaloHit*>> candidateTracks;
    std::map<const TrigCaloHit*, size_t> hitToBestTrack;

    // Filter for section = 0 and hits < 33 layers
    for (const auto& hit : caloHits) {
      if (hit.section() > 0 || hit.layer() > maxLayer) continue;
      layerHits[hit.layer()].push_back(hit);
    }

    // Find mip seeds
    for (const auto& [seedLayer, seeds] : layerHits) {
      for (const auto& seed : seeds) {
        // Skip if hit already used or outside mip energy range
        if (usedHits.count(&seed) || seed.energy() < min_energy_ ||
            seed.energy() > max_energy_)
          continue;

        std::vector<const TrigCaloHit*> track;
        track.push_back(&seed);

        const TrigCaloHit* last = &seed;  // Most recent hit in track
        int holes = 0;
        float growthFactor = 1.0f;

        // Look layer by layer for next hit within dR
        for (int l = seed.layer() + 1; l <= maxLayer; ++l) {
          const TrigCaloHit* bestHit = nullptr;
          float bestdR2 =
              radiusCut2 *
              (growthFactor *
               growthFactor);  // Grow search window if there is a hole

          for (const auto& cand : layerHits[l]) {
            if (usedHits.count(&cand) || cand.energy() < min_energy_ ||
                cand.energy() > max_energy_)
              continue;

            // Corrects for layer shift in x-direction which we calculated
            // is 4.82 mm
            float layerShiftLast = (last->layer() % 2 == 0) ? 0.0f : 4.82f;
            float layerShiftCand = (cand.layer() % 2 == 0) ? 0.0f : 4.82f;

            float dx =
                (cand.x() - layerShiftCand) - (last->x() - layerShiftLast);
            float dy = cand.y() - last->y();
            float dR2 = dx * dx + dy * dy;
            if (dR2 < bestdR2) {
              bestdR2 = dR2;
              bestHit = &cand;  // Closest unused hit in next layer
            }
          }

          if (bestHit) {
            track.push_back(bestHit);  // Builds track from best hits
            last = bestHit;
            holes = 0;
            growthFactor = 1.0f;  // Reset search window
          } else {
            holes++;
            growthFactor = (holes + 1);  // Keep expanding
          }
        }

        bool isIsolated = true;
        float isolationECut = 180;  // MeV; Change as needed
        float isolationRadius2 = radiusCut * radiusCut;
        if ((track.size() >= minTrackLength)) {
          for (const auto* hit : track) {  // Isolation area energy check
            int layer = hit->layer();
            float hitx = hit->x();
            float hity = hit->y();
            float sumE = 0.0f;

            for (const auto& cand : layerHits[layer]) {
              if (&cand == hit) continue;  // Skips self
              float dx = cand.x() - hitx;
              float dy = cand.y() - hity;
              float dR2 = dx * dx + dy * dy;

              if (dR2 < isolationRadius2) {
                sumE += cand.energy();
              }
            }

            if (sumE >= isolationECut) {
              isIsolated = false;
              usedHits.insert(hit);  // Adds used hits to vector so they cannot
                                     // be used again
              break;
            }
          }
          if (!isIsolated) {
            continue;  // Reject track if any hit is not isolated
          }

          size_t i = candidateTracks.size();
          candidateTracks.push_back(track);

          for (const auto* hit : track) {
            if (!hitToBestTrack.count(hit) ||
                candidateTracks[i].size() >
                    candidateTracks[hitToBestTrack[hit]].size()) {
              hitToBestTrack[hit] = i;
              usedHits.insert(hit);  // Adds used hits to vector so they cannot
                                     // be used again
            }
          }
        }
      }
    }

    std::set<size_t> validTrackIDs;
    for (const auto& [hit, idx] : hitToBestTrack) {
      validTrackIDs.insert(idx);
    }

    TrigMipCollection mips;
    float max_hole_fraction = 0.2f;  // No more than 20% hole fraction ; Change
                                     // as needed Converts tracks to MIP objects
    for (size_t idx : validTrackIDs) {
      const auto& track = candidateTracks[idx];

      TrigMip mip;
      mip.setStartLayer(track.front()->layer());
      mip.setEndLayer(track.back()->layer());
      mip.setNHits(track.size());
      mip.setLength(track.back()->layer() - track.front()->layer());
      mip.setNHoles(mip.length() - mip.nHits());

      float hole_fraction = (static_cast<float>(mip.nHoles()) / mip.length());
      // Remove mip tracks with hole fraction > 0.2
      if (hole_fraction >= max_hole_fraction) continue;

      float totalIsolationESum = 0.0f;
      for (const auto* hit : track) {
        int layer = hit->layer();
        float hitx = hit->x();
        float hity = hit->y();
        for (const auto& cand : layerHits[layer]) {
          if (&cand == hit) continue;
          float dx = cand.x() - hitx;
          float dy = cand.y() - hity;
          float dR2 = dx * dx + dy * dy;
          if (dR2 < radiusCut * radiusCut) {
            totalIsolationESum += cand.energy();
          }
        }
      }
      mip.setSumEinIsolationRegion(totalIsolationESum);
      mips.push_back(mip);
    }
    std::sort(mips.begin(), mips.end());

    event.add(pass_coll_name_, mips);
  }
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::TrigMipReco);
