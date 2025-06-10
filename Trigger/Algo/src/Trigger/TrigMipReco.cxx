#include "Trigger/TrigMipReco.h"

namespace trigger {

void TrigMipReco::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  passCollName_ = ps.getParameter<std::string>("passCollName");
  calorimeterTypeIsHcal_ = ps.getParameter<bool>("calorimeterTypeIsHcal");
  if (calorimeterTypeIsHcal_) {
    minEnergy_ = 8.0f;  // mip peak is 10-11
  } else {
    minEnergy_ = 3.0f;
    maxEnergy_ = 26.0f;
  }
}

void TrigMipReco::produce(framework::Event& event) {
  if (!event.exists(hitCollName_)) return;
  auto caloHits{event.getObject<TrigCaloHitCollection>(hitCollName_)};

  if (calorimeterTypeIsHcal_) {
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
      if (tp.section() > 0 || tp.energy() < minEnergy_ || tp.layer() > 47)
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

    event.add(passCollName_, mips);
  } else {                 // ECAL MIP Reconstruction
    float radiusCut = 50;  // mm
    int maxLayer = 32;
    int minTrackLength = 5;  // example

    std::map<int, std::vector<TrigCaloHit>> layerHits;
    std::set<const TrigCaloHit*> usedHits;
    std::vector<std::vector<const TrigCaloHit*>> candidateTracks;
    std::map<const TrigCaloHit*, size_t> hitToBestTrack;

    // Filter for section = 0, energy range, and hits < 33 layers
    for (const auto& hit : caloHits) {
      if (hit.section() > 0 || hit.energy() < minEnergy_ ||
          hit.energy() > maxEnergy_ || hit.layer() > maxLayer)
        continue;
      layerHits[hit.layer()].push_back(hit);
    }

    // Find mip seeds
    for (const auto& [seedLayer, seeds] : layerHits) {
      for (const auto& seed : seeds) {
        if (usedHits.count(&seed)) continue;  // Skip if hit already used

        std::vector<const TrigCaloHit*> track;
        track.push_back(&seed);

        const TrigCaloHit* last = &seed;  // Most recent hit in track
        int holes = 0;

        // Look layer by layer for next hit within dR
        for (int l = seed.layer() + 1; l <= maxLayer; ++l) {
          const TrigCaloHit* bestHit = nullptr;
          float bestdR2 = radiusCut * radiusCut;

          for (const auto& cand : layerHits[l]) {
            if (usedHits.count(&cand)) continue;

            // corrects for layer shift in x-direction, we calculated that the
            // shift is 4.82 mm
            float layerShiftLast = (last->layer() % 2 == 0) ? 0.0f : 4.82f;
            float layerShiftCand = (cand.layer() % 2 == 0) ? 0.0f : 4.82f;

            float dx =
                (cand.x() - layerShiftCand) - (last->x() - layerShiftLast);
            float dy = cand.y() - last->y();
            float dR2 = dx * dx + dy * dy;
            if (dR2 < bestdR2) {
              bestdR2 = dR2;
              bestHit = &cand;  // closest unused hit in next layer
            }
          }

          if (bestHit) {
            track.push_back(bestHit);  // builds track from best hits
            last = bestHit;
          } else {
            holes++;
            // if (holes > 2) break;
          }
        }

        if (track.size() >= minTrackLength) {
          size_t i = candidateTracks.size();
          candidateTracks.push_back(track);

          // ensures that only the longest track per event is kept
          for (const auto* h : track) {
            if (!hitToBestTrack.count(h) ||
                candidateTracks[i].size() >
                    candidateTracks[hitToBestTrack[h]].size()) {
              hitToBestTrack[h] = i;
              usedHits.insert(
                  h);  // adds used hits to vector so they cannot be used again
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
    // Converts tracks to MIP objects
    for (size_t idx : validTrackIDs) {
      const auto& track = candidateTracks[idx];

      TrigMip mip;
      mip.setStartLayer(track.front()->layer());
      mip.setEndLayer(track.back()->layer());
      mip.setNHits(track.size());
      mip.setLength(track.back()->layer() - track.front()->layer());
      mip.setNHoles(mip.length() - mip.nHits());
      mips.push_back(mip);
    }
    std::sort(mips.begin(), mips.end());

    event.add(passCollName_, mips);
  }

  // std::sort(sortedHits.begin(), sortedHits.end(),
  // 	    [](TrigCaloHit a, TrigCaloHit b) {
  // 	      return a.layer() > b.layer();
  // 	    });

  // run inside-out tracking
  // std::cout << "new"() << std::endl;
  // for (const auto& tp : caloHits) {
  //   std::cout << tp.layer() << std::endl;

  // }

  // double x{0}, y{0}, z{0}; // todo
  // ldmx::HcalTriggerID combo_id(tp.getId());

  // int adc = tp.getPrimitive();
  // double e = adc * 1.2 / 72.961; // ADC to MeV
  // passTrigHits.emplace_back(x, y, z, e);

  // passTrigHits.back().setLayer(combo_id.layer());
  // passTrigHits.back().setStrip(combo_id.superstrip());
  // passTrigHits.back().setSection(combo_id.section());

  // event.add(passCollName_ + "Hits", passTrigHits);
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TrigMipReco);
