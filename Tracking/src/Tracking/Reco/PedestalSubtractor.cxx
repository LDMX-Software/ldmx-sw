#include "Tracking/Reco/PedestalSubtractor.h"

#include <iostream>
#include <vector>

#include "Tracking/Event/RawSiStripHit.h"
#include "Tracking/Reco/SiStripChannelMap.h"
#include "Tracking/Reco/TrackerPedestals.h"

namespace tracking::reco {

void PedestalSubtractor::configure(framework::config::Parameters& ps) {
  input_collection_ =
      ps.get<std::string>("input_collection", input_collection_);
  input_pass_name_ = ps.get<std::string>("input_pass_name", input_pass_name_);
  output_collection_ =
      ps.get<std::string>("output_collection", output_collection_);
}

void PedestalSubtractor::produce(framework::Event& event) {
  const auto& peds =
      getCondition<TrackerPedestals>(TrackerPedestals::CONDITIONS_NAME);

  const auto& raw = event.getCollection<ldmx::RawSiStripHit>(input_collection_,
                                                             input_pass_name_);

  std::vector<ldmx::RawSiStripHit> hits;
  hits.reserve(raw.size());

  for (const auto& raw_hit : raw) {
    const auto* ped = peds.find(raw_hit.getFebId(), raw_hit.getHybridId(),
                                raw_hit.getApvId(), raw_hit.getChannel());

    const auto& s = raw_hit.getSamples();
    std::vector<short> sub(s.begin(), s.end());

    if (!ped) {
      // Warn once per unknown channel, pass hit through un-subtracted.
      const auto key =
          channelmap::channelKey(raw_hit.getFebId(), raw_hit.getHybridId(),
                                 raw_hit.getApvId(), raw_hit.getChannel());
      if (warned_channels_.insert(key).second) {
        std::cout << "[PedestalSubtractor] WARN: no pedestal for channel "
                  << key << " — passing through un-subtracted" << std::endl;
      }
    } else {
      for (int i = 0; i < 3 && i < static_cast<int>(sub.size()); ++i)
        sub[i] = static_cast<short>(s[i] - ped->mean[i]);
    }

    // Emit a pedestal-subtracted RawSiStripHit that keeps the same electronics
    // identity as the input; only the ADC samples differ.
    ldmx::RawSiStripHit out(raw_hit.getChannel(), sub, raw_hit.getTime());
    out.setApvId(raw_hit.getApvId());
    out.setHybridId(raw_hit.getHybridId());
    out.setFebId(raw_hit.getFebId());
    out.setApvTrigger(raw_hit.getApvTrigger());
    hits.push_back(std::move(out));
  }

  event.add(output_collection_, hits);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::PedestalSubtractor)
