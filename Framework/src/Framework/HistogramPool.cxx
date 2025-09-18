
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
    EXCEPTION_RAISE("InvalidArg", "Histogram " + name + " not found in pool.");
  }

  return histograms_[name];
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

void HistogramPool::fill(const std::string& name, const double& val) {
  auto hist = dynamic_cast<TH1F*>(this->get(name));
  if (hist) {
    hist->Fill(val, the_weight_);
  } else {
    EXCEPTION_RAISE("BadHistSize", "Attempting to 1D fill a histogram that is not actually a TH1F");
  }
}

void HistogramPool::fill(const std::string& name, const double& valx, const double& valy) {
  auto hist = dynamic_cast<TH2F*>(this->get(name));
  if (hist) {
    hist->Fill(valx, valy, the_weight_);
  } else {
    EXCEPTION_RAISE("BadHistSize", "Attempting to 2D fill a histogram that is not actually a TH2F");
  }
}

}  // namespace framework
