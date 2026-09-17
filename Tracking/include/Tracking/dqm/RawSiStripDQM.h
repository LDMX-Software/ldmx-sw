#pragma once

#include <map>
#include <string>
#include <vector>

#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "TH1.h"

namespace tracking::dqm {

/**
 * DQM for silicon strip data read from the DAQ: raw hits, pedestal
 * subtracted hits, the waveforms assembled from them and their pulse fits.
 */
class RawSiStripDQM : public framework::Analyzer {
 public:
  RawSiStripDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {};

  ~RawSiStripDQM() = default;

  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

 private:
  /// histograms by name, looked up once per event
  std::vector<TH1*> getHists(const std::vector<std::string>& names);

  std::string raw_hits_collection_;
  std::string subtracted_hits_collection_;
  std::string waveforms_collection_;
  std::string fitted_hits_collection_;
  std::string input_pass_name_;
  int n_hybrids_{0};

  /// per-hybrid histogram names, built once
  std::vector<std::string> raw_adc_names_;
  std::vector<std::string> subtracted_adc_names_;
  std::vector<std::string> pchannel_names_;
  std::vector<std::string> peak_names_;
  /// fit amplitude histogram name by layer id
  std::map<int, std::string> fit_amplitude_names_;
};
}  // namespace tracking::dqm
