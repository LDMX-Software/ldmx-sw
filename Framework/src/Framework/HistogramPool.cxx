
#include "Framework/HistogramPool.h"

#include "Framework/Exception/Exception.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <stdexcept>

//----------//
//   ROOT   //
//----------//
#include "TH1.h"
#include "TStyle.h"

namespace framework {

TH1* HistogramPool::get(const std::string& name) {
  auto histo = histograms_.find(name);
  if (histo == histograms_.end()) {
    EXCEPTION_RAISE("InvalidArg",
        "Histogram " + name + " not found in pool."
        "\nMake sure to `histograms_.create` in onProcessStart for any "
        "histogram you want to `histograms_.fill`.");
  }

  return histograms_[name];
}


std::tuple<std::size_t,double,double> category_bins(
    const std::vector<std::string>& categories,
    int offset = 0
    ) {
  std::size_t n_categories = categories.size();
  double min = offset - 0.5;
  double max = offset + n_categories + 1.5;
  return std::make_tuple(n_categories, min, max);
}

void label_axis(TAxis* axis, const std::vector<std::string>& categories) {
  for (std::size_t ibin{1}; ibin <= categories.size(); ibin++) {
    axis->SetBinLabel(ibin, categories[ibin-1].c_str());
  }
}

void HistogramPool::create(const std::string& name, const std::vector<std::string>& categories) {
  create(name, "", 0, 0, 0);
  auto h{get(name)};
  for (const auto& cat : categories) {
    h->Fill(cat.c_str(), 0.0);
  }
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
                             const int& bins, const double& xmin,
                             const double& xmax) {
  get_directory_()->cd();

  auto hist = new TH1F(name.c_str(), "", bins, xmin, xmax);
  hist->GetXaxis()->SetTitle(x_label.c_str());

  histograms_[name] = hist;
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
                             const std::vector<double>& bins) {
  get_directory_()->cd();

  auto hist = new TH1F(name.c_str(), "", bins.size()-1, bins.data());
  hist->GetXaxis()->SetTitle(x_label.c_str());

  histograms_[name] = hist;
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
                             const int& xbins, const double& xmin,
                             const double& xmax, const std::string& y_label,
                             const int& ybins, const double& ymin,
                             const double& ymax) {
  get_directory_()->cd();

  auto hist = new TH2F(name.c_str(), "", xbins, xmin, xmax,
                       ybins, ymin, ymax);
  hist->GetXaxis()->SetTitle(x_label.c_str());
  hist->GetYaxis()->SetTitle(y_label.c_str());

  histograms_[name] = hist;
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
                             const std::vector<double>& xbins,
                             const std::string& y_label,
                             const std::vector<double>& ybins) {
  get_directory_()->cd();
  auto hist = new TH2F(name.c_str(), "",
                       xbins.size()-1, xbins.data(),
                       ybins.size()-1, ybins.data());
  hist->GetXaxis()->SetTitle(x_label.c_str());
  hist->GetYaxis()->SetTitle(y_label.c_str());

  histograms_[name] = hist;
}

/*
void HistogramPool::fill(const std::string& name, const double& val) {
}
*/

void HistogramPool::fill(const std::string& name, const double& valx, const double& valy) {
  auto hist = dynamic_cast<TH2F*>(this->get(name));
  if (hist) {
    hist->Fill(valx, valy, the_weight_);
  } else {
    EXCEPTION_RAISE("BadHistSize", "Attempting to 2D fill a histogram that is not actually a TH2F");
  }
}

}  // namespace framework
