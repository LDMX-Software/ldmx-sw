#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace tracking::reco {

/**
 * Turns decoded trigger-scintillator bar digis into geometry-aware
 * ldmx::Measurement objects.
 *
 * The TS bars measure the y coordinate. This producer reads one or more decoded
 * ZCCM pad collections (trigscint::TrigScintQIEDigis, bar-addressed by chanID),
 * applies a simple ADC-amplitude threshold to define a hit, and converts each
 * hit's (module, chanID) into a global (x, y, z) position using a parameterized
 * bar geometry -- so the output measurements live in the same ldmx::Measurement
 * frame the tracker uses and can be correlated with tracker measurements.
 *
 * This is the first, parameterized increment (geometry via python params, tuned
 * against the tracker beam). The follow-up promotes the bar geometry to a proper
 * DetDescr ConditionsObject provider (Ecal/Hcal-style), read from the detector.
 *
 * Input  : one collection of trigscint::TrigScintQIEDigis per module
 * Output : collection of ldmx::Measurement
 */
class TrigScintMeasurementProducer : public framework::Producer {
 public:
  TrigScintMeasurementProducer(const std::string& name,
                               framework::Process& process)
      : framework::Producer(name, process) {}

  void configure(framework::config::Parameters& parameters) override;
  void produce(framework::Event& event) override;

 private:
  /// input digi collections, one per module (index = module number)
  std::vector<std::string> input_collections_;
  /// pass name of the input collections ("" = any)
  std::string input_pass_;
  /// output ldmx::Measurement collection name
  std::string out_collection_;

  // Bar positions come from the TrigScintGeometry conditions object
  // (getCondition), not typed here.

  /// optional TS DAQ-map JSON (decode collection -> geometry module). If unset,
  /// the input_collections index is used as the geometry module.
  std::string daq_map_file_;
  /// parsed DAQ map: (decoded collection name, geometry module index)
  std::vector<std::pair<std::string, int>> daq_modules_;

  /// hit definition: ADC amplitude (max-min over samples) must exceed this
  double amp_threshold_{40.};
  /// assumed y measurement resolution [mm] (local covariance)
  double sigma_y_{0.9};
};

}  // namespace tracking::reco
