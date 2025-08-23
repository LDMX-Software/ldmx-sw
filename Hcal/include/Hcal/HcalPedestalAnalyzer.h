#ifndef HCALPEDESTALANALYZER_H
#define HCALPEDESTALANALYZER_H

#include "DetDescr/HcalDigiID.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"
namespace hcal {

class HcalPedestalAnalyzer : public framework::Analyzer {
  std::string input_name_, input_pass_;
  std::string output_file_, comments_;
  bool make_histos_;
  bool filter_no_tot_;
  bool filter_no_toa_;
  int low_cutoff_, high_cutoff_;

  struct Channel {
    Channel() : hist_{0}, sum_{0}, sum_sq_{0}, entries_{0}, rejects_{4, 0} {}
    /// collection of hits accumulated to produce appropriately-binned
    /// histograms
    std::vector<int> adcs_;
    /// Histogram, if used
    TH1* hist_;
    /// Sum of values
    uint64_t sum_;
    /// Sum of values squared
    double sum_sq_;
    /// Number of entries
    int entries_;
    /// counts of various rejections
    std::vector<int> rejects_;
  };

  std::map<ldmx::HcalDigiID, Channel> pedestal_data_;

  void createAndFill(Channel& chan, ldmx::HcalDigiID detid);

 public:
  HcalPedestalAnalyzer(const std::string& n, framework::Process& p)
      : framework::Analyzer(n, p) {}
  virtual ~HcalPedestalAnalyzer() = default;

  void configure(framework::config::Parameters& ps) override {
    input_name_ = ps.getParameter<std::string>("input_name");
    input_pass_ = ps.getParameter<std::string>("input_pass");
    output_file_ = ps.getParameter<std::string>("output_file");
    comments_ = ps.getParameter<std::string>("comments");

    make_histos_ = ps.getParameter<bool>("make_histos", false);

    filter_no_tot_ = ps.getParameter<bool>("filter_noTOT", true);
    filter_no_toa_ = ps.getParameter<bool>("filter_noTOA", true);
    low_cutoff_ = ps.getParameter<int>("low_cutoff", 10);
    high_cutoff_ = ps.getParameter<int>("high_cutoff", 512);
  }

  void analyze(const framework::Event& event) override;
  void onProcessEnd() override;
};

}  // namespace hcal

#endif /* HCALPEDESTALANALYZER_H */
