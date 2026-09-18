#include "Tracking/Reco/TrigScintMeasurementProducer.h"

#include <algorithm>
#include <fstream>

#include <nlohmann/json.hpp>

#include <cmath>

#include "DetDescr/TrigScintGeometry.h"
#include "TSystem.h"
#include "TrigScint/Event/TrigScintCluster.h"
#include "Tracking/Event/Measurement.h"

namespace tracking::reco {

void TrigScintMeasurementProducer::configure(
    framework::config::Parameters& parameters) {
  // Load the TrigScint_Event ROOT dictionary so the digi collection reads as the
  // compiled type. The accessors are inline (linker drops libTrigScint_Event)
  // and there is no rootmap for autoload, so ROOT would otherwise fall back to
  // an emulated streamer that crashes when cast to the compiled class.
  gSystem->Load("libTrigScint_Event");

  input_collections_ =
      parameters.getParameter<std::vector<std::string>>("input_collections");
  input_pass_ = parameters.getParameter<std::string>("input_pass", "");
  out_collection_ = parameters.getParameter<std::string>("out_collection",
                                                         "TrigScintMeasurements");
  min_pe_ = parameters.getParameter<double>("min_pe", min_pe_);
  sigma_y_ = parameters.getParameter<double>("sigma_y", sigma_y_);

  // Optional TS DAQ map: fixes the decode-collection -> geometry-module
  // (cabling) correspondence, analogous to the tracker's daq_map_file. Entries
  // with geo_module < 0 (e.g. the LYSO pad, not yet in the geometry) are
  // skipped.
  daq_map_file_ = parameters.getParameter<std::string>("daq_map_file", "");
  daq_modules_.clear();
  if (!daq_map_file_.empty()) {
    std::ifstream in(daq_map_file_);
    if (!in) {
      EXCEPTION_RAISE("TrigScintDaqMap",
                      "Could not open TS DAQ map '" + daq_map_file_ + "'.");
    }
    nlohmann::json doc;
    in >> doc;
    for (const auto& m : doc.at("modules")) {
      const int geo_module = m.at("geo_module").get<int>();
      if (geo_module < 0) continue;  // not mapped to geometry (e.g. LYSO)
      daq_modules_.emplace_back(m.at("collection").get<std::string>(),
                                geo_module);
    }
    ldmx_log(info) << "TS DAQ map '" << daq_map_file_ << "' -> "
                   << daq_modules_.size() << " mapped modules.";
  }
}

void TrigScintMeasurementProducer::produce(framework::Event& event) {
  // Bar positions come from the geometry conditions object (not hard-coded).
  const auto& geom = getCondition<ldmx::TrigScintGeometry>(
      ldmx::TrigScintGeometry::CONDITIONS_OBJECT_NAME);
  const int n_bars = geom.getNumBars();

  std::vector<ldmx::Measurement> measurements;

  // (decoded collection, geometry module) pairs to process: from the DAQ map
  // if provided, else the input_collections order (index = geometry module).
  std::vector<std::pair<std::string, int>> to_process = daq_modules_;
  if (to_process.empty()) {
    for (std::size_t i = 0; i < input_collections_.size(); ++i)
      to_process.emplace_back(input_collections_[i], static_cast<int>(i));
  }

  for (const auto& [collection, geo_module] : to_process) {
    // TestBeamClusterProducer only writes the collection for events with >=1
    // cluster, so it is legitimately absent in empty events -- skip those.
    if (!event.exists(collection, input_pass_, false)) continue;
    const auto& clusters =
        event.getCollection<ldmx::TrigScintCluster>(collection, input_pass_);

    for (const auto& cluster : clusters) {
      if (cluster.getNHits() <= 0) continue;
      if (cluster.getPE() < min_pe_) continue;

      // PE-weighted fractional bar centroid (0-based); < 0 means uninitialized.
      const double centroid = cluster.getCentroid();
      if (centroid < 0.) continue;

      // Interpolate the global position between the two bracketing bars. The
      // bars alternate between two staggered layers (bar%2), so the geometry is
      // piecewise in bar parity -- evaluating both integer endpoints and
      // interpolating gives the correct y AND the correct in-between z, and
      // reduces to the exact bar position for an integer centroid.
      int b0 = static_cast<int>(std::floor(centroid));
      if (b0 < 0) b0 = 0;
      if (b0 > n_bars - 1) b0 = n_bars - 1;
      int b1 = std::min(b0 + 1, n_bars - 1);
      double f = centroid - b0;
      if (f < 0.) f = 0.;
      if (f > 1.) f = 1.;

      const auto p0 = geom.getBarPosition(geo_module, b0);
      const auto p1 = geom.getBarPosition(geo_module, b1);
      const double x = (1. - f) * p0.X() + f * p1.X();
      const double y = (1. - f) * p0.Y() + f * p1.Y();
      const double z = (1. - f) * p0.Z() + f * p1.Z();

      ldmx::Measurement meas;
      meas.setGlobalPosition(static_cast<float>(x), static_cast<float>(y),
                             static_cast<float>(z));
      meas.setLocalPosition(static_cast<float>(y), 0.f);
      meas.setLocalCovariance(static_cast<float>(sigma_y_ * sigma_y_), 0.f);
      // encode geometry module + nearest bar so downstream can trace it
      meas.setLayerID(geo_module * 100 +
                      static_cast<int>(std::lround(centroid)));
      meas.setTime(cluster.getTime());
      meas.setClusterAmplitude(static_cast<float>(cluster.getPE()));
      meas.setNStrips(cluster.getNHits());
      measurements.push_back(meas);
    }
  }

  event.add(out_collection_, measurements);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::TrigScintMeasurementProducer)
