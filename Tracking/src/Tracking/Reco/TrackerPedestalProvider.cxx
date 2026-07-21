#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"
#include "Framework/Exception/Exception.h"
#include "Tracking/Reco/SiStripChannelMap.h"
#include "Tracking/Reco/TrackerPedestals.h"

namespace tracking::reco {

const std::string TrackerPedestals::CONDITIONS_NAME = "TrackerPedestals";

/**
 * Conditions provider for the per-channel tracker pedestals/noise.
 *
 * Reads the pedestal file written by PedestalCalculator and serves it as a
 * TrackerPedestals conditions object.  The storage backend ("json"; future:
 * "sqlite", ...) is selected by the pedestal_format parameter, so the choice of
 * source lives entirely here rather than in any consuming processor.
 */
class TrackerPedestalProvider : public framework::ConditionsObjectProvider {
 public:
  TrackerPedestalProvider(const std::string& name, const std::string& tagname,
                          const framework::config::Parameters& parameters,
                          framework::Process& process)
      : framework::ConditionsObjectProvider(TrackerPedestals::CONDITIONS_NAME,
                                            tagname, parameters, process) {
    if (name != TrackerPedestals::CONDITIONS_NAME) {
      EXCEPTION_RAISE("BadConfig",
                      "The name provided to TrackerPedestalProvider '" + name +
                          "' is not the expected '" +
                          TrackerPedestals::CONDITIONS_NAME + "'.");
    }
    pedestal_file_ = parameters.get<std::string>("pedestal_file");
    pedestal_format_ =
        parameters.get<std::string>("pedestal_format", pedestal_format_);
  }

  std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& /*context*/) override {
    auto* peds = new TrackerPedestals();

    // Dispatch on the configured storage backend.  Only JSON is supported
    // today; additional backends (e.g. SQLite) can be added as new load*().
    if (pedestal_format_ == "json") {
      loadJson(*peds);
    } else {
      EXCEPTION_RAISE("BadConfiguration",
                      "TrackerPedestalProvider unknown pedestal_format '" +
                          pedestal_format_ + "' (supported: 'json').");
    }

    std::cout << "[TrackerPedestalProvider] Loaded " << peds->size()
              << " channel pedestals from '" << pedestal_file_ << "'"
              << std::endl;

    // Pedestals are valid for all runs (data and MC) for now.
    return {peds, framework::ConditionsIOV(true, true)};
  }

 private:
  void loadJson(TrackerPedestals& peds) const {
    std::ifstream in(pedestal_file_);
    if (!in) {
      EXCEPTION_RAISE("FileNotFound",
                      "TrackerPedestalProvider could not open pedestal file '" +
                          pedestal_file_ + "'.");
    }

    nlohmann::json doc;
    try {
      in >> doc;
    } catch (const nlohmann::json::parse_error& e) {
      EXCEPTION_RAISE(
          "BadFormat",
          "TrackerPedestalProvider failed to parse pedestal file '" +
              pedestal_file_ + "': " + e.what());
    }

    // Each channel is keyed "feb:hyb:apv:ch" -> {"mean":[...], "noise":[...]}.
    for (const auto& [key, ch] : doc.at("channels").items()) {
      uint8_t feb, hybrid, apv, channel;
      if (!channelmap::parseChannelKey(key, feb, hybrid, apv, channel))
        continue;
      TrackerPedestals::Channel ped;
      ped.mean = ch.at("mean").get<std::array<float, 3>>();
      ped.noise = ch.at("noise").get<std::array<float, 3>>();
      peds.add(feb, hybrid, apv, channel, ped);
    }
  }

  std::string pedestal_file_;
  std::string pedestal_format_{"json"};
};

}  // namespace tracking::reco

DECLARE_CONDITIONS_PROVIDER(tracking::reco::TrackerPedestalProvider)
