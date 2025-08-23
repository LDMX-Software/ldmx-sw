/** @class Service application which creates standard CSV-format pedestal files
 */

#include "Hcal/HcalPedestalAnalyzer.h"

namespace hcal {

void HcalPedestalAnalyzer::analyze(const framework::Event& event) {
  auto const& digis{
      event.getObject<ldmx::HgcrocDigiCollection>(input_name_, input_pass_)};

  for (std::size_t i_digi{0}; i_digi < digis.size(); i_digi++) {
    auto d{digis.getDigi(i_digi)};
    ldmx::HcalDigiID detid(d.id());

    Channel& chan = pedestal_data_[detid];

    bool has_tot = false;
    bool has_toa = false;
    bool has_under = false;
    bool has_over = false;

    for (int i = 0; i < digis.getNumSamplesPerDigi(); i++) {
      if (d.at(i).tot() > 0) has_tot = true;
      if (d.at(i).toa() > 0) has_toa = true;
      if (d.at(i).adcT() < low_cutoff_) has_under = true;
      if (d.at(i).adcT() > high_cutoff_) has_over = true;
    }

    if (has_tot && filter_no_tot_) chan.rejects_[0]++;
    if (has_toa && filter_no_toa_) chan.rejects_[1]++;
    if (has_under) chan.rejects_[2]++;
    if (has_over) chan.rejects_[3]++;

    if (has_tot && filter_no_tot_) continue;  // ignore this
    if (has_toa && filter_no_toa_) continue;  // ignore this
    if (has_under)
      continue;  // ignore this, set threshold to zero to disable requirement
    if (has_over)
      continue;  // ignore this, set threshold larger than 1024 to disable
                 // requirement

    for (int i = 0; i < digis.getNumSamplesPerDigi(); i++) {
      int adc = d.at(i).adcT();

      chan.sum_ += adc;
      chan.sum_sq_ += adc * adc;
      chan.entries_++;
      if (chan.hist_)
        chan.hist_->Fill(adc);
      else if (make_histos_)
        chan.adcs_.push_back(adc);
    }

    // histogram-related business
    if (make_histos_ && !chan.hist_ && chan.entries_ > 250)
      createAndFill(chan, detid);
  }
}

void HcalPedestalAnalyzer::createAndFill(Channel& chan,
                                         ldmx::HcalDigiID detid) {
  if (chan.entries_ == 0) return;

  TDirectory* hdir = getHistoDirectory();
  hdir->cd();
  char hname[120];
  sprintf(hname, "pedestal_%d_%d_%d_%d", detid.section(), detid.layer(),
          detid.strip(), detid.end());
  // logic: 100 bins to +/- 5 sigma based on first 250 events.
  double mean = (chan.sum_ * 1.0) / chan.entries_;
  double rms = sqrt(chan.sum_sq_ / chan.entries_ - mean * mean);
  if (rms * 5 < 50)
    chan.hist_ = new TH1D(hname, hname, 30, int(mean) - 15, int(mean) + 15);
  else
    chan.hist_ = new TH1D(hname, hname, 100, mean - 5 * rms, mean + 5 * rms);
  for (auto x : chan.adcs_) chan.hist_->Fill(x);
  chan.adcs_.clear();
}

void HcalPedestalAnalyzer::onProcessEnd() {
  FILE* fout = fopen(output_file_.c_str(), "w");

  time_t t = time(NULL);
  struct tm* gmtm = gmtime(&t);
  char times[1024];
  strftime(times, sizeof(times), "%Y-%m-%d %H:%M:%S GMT", gmtm);
  fprintf(fout, "# %s\n", comments_.c_str());
  fprintf(fout, "# Produced %s\n", times);
  fprintf(fout, "DetID,PEDESTAL_ADC,PEDESTAL_RMS_ADC\n");

  for (auto ichan : pedestal_data_) {
    if (ichan.second.entries_ == 0) {
      std::cout << "All entries filtered for " << ichan.first << " for TOT "
                << ichan.second.rejects_[0] << " for TOA "
                << ichan.second.rejects_[1] << " for underthreshold "
                << ichan.second.rejects_[2] << " for overthreshold "
                << ichan.second.rejects_[3] << std::endl;
      continue;  // all entries were filtered out
    }

    double mean = ichan.second.sum_ * 1.0 / ichan.second.entries_;
    double rms =
        sqrt(ichan.second.sum_sq_ / ichan.second.entries_ - mean * mean);
    fprintf(fout, "0x%08x,%9.3f,%9.3f\n", ichan.first.raw(), mean, rms);
  }

  fclose(fout);
}

}  // namespace hcal

DECLARE_ANALYZER(hcal::HcalPedestalAnalyzer);
