#include "Tracking/dqm/RawSiStripDQM.h"

#include "Tracking/Event/FittedSiStripHit.h"
#include "Tracking/Event/RawSiStripHit.h"
#include "Tracking/Event/SiStripWaveform.h"

namespace tracking::dqm {

void RawSiStripDQM::configure(framework::config::Parameters& parameters) {
  raw_hits_collection_ = parameters.get<std::string>("raw_hits_collection");
  subtracted_hits_collection_ =
      parameters.get<std::string>("subtracted_hits_collection");
  waveforms_collection_ = parameters.get<std::string>("waveforms_collection");
  fitted_hits_collection_ =
      parameters.get<std::string>("fitted_hits_collection");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  n_hybrids_ = parameters.get<int>("n_hybrids");
  const auto layer_ids{parameters.get<std::vector<int>>("layer_ids")};

  for (int h{0}; h < n_hybrids_; ++h) {
    const std::string suffix{"_h" + std::to_string(h)};
    raw_adc_names_.push_back("raw_adc" + suffix);
    subtracted_adc_names_.push_back("subtracted_adc" + suffix);
    pchannel_names_.push_back("waveform_pchannel" + suffix);
    peak_names_.push_back("waveform_peak" + suffix);
  }
  for (int layer : layer_ids) {
    fit_amplitude_names_[layer] = "fit_amplitude_l" + std::to_string(layer);
  }
}

std::vector<TH1*> RawSiStripDQM::getHists(
    const std::vector<std::string>& names) {
  std::vector<TH1*> hists;
  for (const auto& name : names) hists.push_back(histograms_.get(name));
  return hists;
}

void RawSiStripDQM::analyze(const framework::Event& event) {
  // direct fills, ~1e5 samples per event
  if (event.exists(raw_hits_collection_, input_pass_name_)) {
    const auto& hits{event.getCollection<ldmx::RawSiStripHit>(
        raw_hits_collection_, input_pass_name_)};
    histograms_.fill("n_raw_hits", hits.size());
    auto* read_error{histograms_.get("raw_read_error")};
    const auto raw_adc{getHists(raw_adc_names_)};
    for (const auto& hit : hits) {
      read_error->Fill(hit.getReadError());
      if (hit.getHybridId() >= n_hybrids_) continue;
      for (short adc : hit.getSamples()) {
        raw_adc[hit.getHybridId()]->Fill(adc);
      }
    }
  }

  if (event.exists(subtracted_hits_collection_, input_pass_name_)) {
    const auto& hits{event.getCollection<ldmx::RawSiStripHit>(
        subtracted_hits_collection_, input_pass_name_)};
    const auto subtracted_adc{getHists(subtracted_adc_names_)};
    for (const auto& hit : hits) {
      if (hit.getHybridId() >= n_hybrids_) continue;
      for (short adc : hit.getSamples()) {
        subtracted_adc[hit.getHybridId()]->Fill(adc);
      }
    }
  }

  if (event.exists(waveforms_collection_, input_pass_name_)) {
    const auto& waveforms{event.getCollection<ldmx::SiStripWaveform>(
        waveforms_collection_, input_pass_name_)};
    histograms_.fill("n_waveforms", waveforms.size());
    for (const auto& waveform : waveforms) {
      histograms_.fill("waveform_n_triggers", waveform.getNTriggers());
      histograms_.fill("waveform_peak_trigger", waveform.peakTrigger());
      if (waveform.getHybridId() >= n_hybrids_) continue;
      histograms_.fill(pchannel_names_[waveform.getHybridId()],
                       waveform.getPchannel());
      histograms_.fill(peak_names_[waveform.getHybridId()],
                       waveform.peakAmplitude());
    }
  }

  // fit results live in their own collection, converged fits only
  if (!event.exists(fitted_hits_collection_, input_pass_name_)) return;
  const auto& fitted_hits{event.getCollection<ldmx::FittedSiStripHit>(
      fitted_hits_collection_, input_pass_name_)};
  histograms_.fill("n_fitted_hits", fitted_hits.size());
  for (const auto& hit : fitted_hits) {
    histograms_.fill("fit_t0", hit.getT0());
    if (hit.getNDF() > 0) {
      histograms_.fill("fit_chi2_ndf", hit.getReducedChi2());
    }
    auto name{fit_amplitude_names_.find(hit.getLayerID())};
    if (name != fit_amplitude_names_.end()) {
      histograms_.fill(name->second, hit.getAmplitude());
    }
  }
}
}  // namespace tracking::dqm

DECLARE_ANALYZER(tracking::dqm::RawSiStripDQM)
