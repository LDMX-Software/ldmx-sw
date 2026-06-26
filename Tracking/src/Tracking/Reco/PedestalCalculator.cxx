#include "Tracking/Reco/PedestalCalculator.h"

#include <cmath>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "Tracking/Event/RawSiStripHit.h"
#include "Tracking/Reco/SiStripChannelMap.h"

namespace tracking::reco {

void PedestalCalculator::configure(framework::config::Parameters& ps) {
  input_collection_ = ps.get<std::string>("input_collection", input_collection_);
  input_pass_name_  = ps.get<std::string>("input_pass_name", input_pass_name_);
  output_file_      = ps.get<std::string>("output_file", output_file_);
  output_format_    = ps.get<std::string>("output_format", output_format_);
}

void PedestalCalculator::analyze(const framework::Event& event) {
  const auto& hits =
      event.getCollection<ldmx::RawSiStripHit>(input_collection_, input_pass_name_);

  for (const auto& hit : hits) {
    const auto key = channelmap::channelKey(hit.getFebId(), hit.getHybridId(),
                                            hit.getApvId(), hit.getChannel());
    auto& acc = accumulators_[key];
    acc.n++;
    const auto& samples = hit.getSamples();
    for (int s = 0; s < 3 && s < static_cast<int>(samples.size()); ++s) {
      // Welford's online algorithm: numerically stable incremental mean+variance.
      double v = samples[s];
      double delta = v - acc.mean[s];
      acc.mean[s] += delta / acc.n;
      acc.M2[s]   += delta * (v - acc.mean[s]);
    }
  }
  ++n_events_;
}

void PedestalCalculator::onProcessEnd() {
  // Dispatch on the configured storage backend.  Only JSON is supported today;
  // additional backends (e.g. SQLite) can be added as new write*() methods.
  if (output_format_ == "json") {
    writePedestalsJson();
  } else {
    std::cerr << "[PedestalCalculator] ERROR: unknown output_format '"
              << output_format_ << "' (supported: 'json')" << std::endl;
  }
}

void PedestalCalculator::writePedestalsJson() {
  nlohmann::json channels = nlohmann::json::object();
  for (const auto& [key, acc] : accumulators_) {
    std::array<double, 3> noise{};
    for (int s = 0; s < 3; ++s) {
      // Welford: M2/(n-1) is the sample variance; M2/n is population variance.
      // Use population variance (divide by n) since we want noise of the distribution.
      noise[s] = acc.n > 1 ? std::sqrt(acc.M2[s] / acc.n) : 0.0;
    }
    channels[key] = {{"mean", acc.mean}, {"noise", noise}};
  }

  nlohmann::json doc = {{"n_events", n_events_}, {"channels", channels}};

  std::ofstream out(output_file_);
  if (!out) {
    std::cerr << "[PedestalCalculator] ERROR: cannot write to '" << output_file_
              << "'" << std::endl;
    return;
  }
  out << doc.dump(2) << std::endl;

  std::cout << "[PedestalCalculator] Wrote " << accumulators_.size()
            << " channels from " << n_events_ << " events to '" << output_file_
            << "'" << std::endl;
}

}  // namespace tracking::reco

DECLARE_ANALYZER(tracking::reco::PedestalCalculator)
