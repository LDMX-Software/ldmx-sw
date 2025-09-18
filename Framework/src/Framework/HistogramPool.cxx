
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

void HistogramPool::create(const std::string& name, const std::string& xLabel,
                             const double& bins, const double& xmin,
                             const double& xmax) {
  // Create a histogram of type T
  auto hist = new TH1F(name.c_str(), name.c_str(), bins, xmin, xmax);

  // Set the title
  hist->SetTitle("");

  // Set the x_-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  histograms_[name] = hist;
}

void HistogramPool::create(const std::string& name, const std::string& xLabel,
                             const std::vector<double>& bins) {
  auto hist = new TH1F(name.c_str(), name.c_str(), bins.size()-1, bins.data());

  // Set the title
  hist->SetTitle("");

  // Set the x_-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  histograms_[name] = hist;
}

void HistogramPool::create(const std::string& name, const std::string& xLabel,
                             const double& xbins, const double& xmin,
                             const double& xmax, const std::string& yLabel,
                             const double& ybins, const double& ymin,
                             const double& ymax) {
  // Create a histogram of type T
  auto hist = new TH2F(name.c_str(), name.c_str(), xbins, xmin, xmax,
                       ybins, ymin, ymax);

  // Set the title
  hist->SetTitle("");

  // Set the x-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Set the y-axis label
  hist->GetYaxis()->SetTitle(yLabel.c_str());
  hist->GetYaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  histograms_[name] = hist;
}

void HistogramPool::create(const std::string& name, const std::string& xLabel,
                             const std::vector<double>& xbins,
                             const std::string& yLabel,
                             const std::vector<double>& ybins) {
  auto hist = new TH2F(name.c_str(), name.c_str(),
                       xbins.size()-1, xbins.data(),
                       ybins.size()-1, ybins.data());

  // Set the title
  hist->SetTitle("");

  // Set the x-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Set the y-axis label
  hist->GetYaxis()->SetTitle(yLabel.c_str());
  hist->GetYaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  histograms_[name] = hist;
}
}  // namespace framework
