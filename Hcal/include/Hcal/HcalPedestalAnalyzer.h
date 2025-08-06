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
  bool filter_noTOT;
  bool filter_noTOA;
  int low_cutoff_, high_cutoff_;

  struct Channel {
    Channel() : hist{0}, sum{0}, sum_sq{0}, entries{0}, rejects{4, 0} {}
    /// collection of hits accumulated to produce appropriately-binned
    /// histograms
    std::vector<int> adcs;
    /// Histogram, if used
    TH1* hist;
    /// Sum of values
    uint64_t sum;
    /// Sum of values squared
    double sum_sq;
    /// Number of entries
    int entries;
    /// counts of various rejections
    std::vector<int> rejects;
  };

  std::map<ldmx::HcalDigiID, Channel> pedestal_data_;

  void create_and_fill(Channel& chan, ldmx::HcalDigiID detid);

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

    filter_noTOT = ps.getParameter<bool>("filter_noTOT", true);
    filter_noTOA = ps.getParameter<bool>("filter_noTOA", true);
    low_cutoff_ = ps.getParameter<int>("low_cutoff", 10);
    high_cutoff_ = ps.getParameter<int>("high_cutoff", 512);
  }

  void analyze(const framework::Event& event) override;
  void onProcessEnd() override;
};

}  // namespace hcal

#endif /* HCALPEDESTALANALYZER_H */
