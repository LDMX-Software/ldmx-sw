
#include "Framework/HistogramPool.h"

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
    EXCEPTION_RAISE(
        "InvalidArg",
        "Histogram " + name +
            " not found in pool."
            "\nMake sure to `histograms_.create` in onProcessStart for any "
            "histogram you want to `histograms_.fill`.");
  }

  return histograms_[name];
}

void HistogramPool::insert(const std::string& name,
                           std::function<TH1*()> factory,
                           bool weighted) {
  if (histograms_.find(name) != histograms_.end()) {
    EXCEPTION_RAISE(
        "RepeatName",
        "Histogram " + name +
            " already exists in histogram pool."
            "\nMake sure to use distinct names for your different histograms!"
            "\nYou can use `histograms_.get` to retrieve a pointer to a "
            "specific histogram "
            "if you want to do some other customizations besides filling.");
  }

  get_directory_()->cd();

  auto h = factory();
  if (weighted) h->Sumw2();
  histograms_[name] = h;
}

std::tuple<std::size_t, double, double> category_bins(
    const std::vector<std::string>& categories, int offset = 0) {
  std::size_t n_categories = categories.size();
  double min = offset - 0.5;
  double max = offset + n_categories + 0.5;
  return std::make_tuple(n_categories, min, max);
}

void label_axis(TAxis* axis, const std::vector<std::string>& categories) {
  for (std::size_t ibin{1}; ibin <= categories.size(); ibin++) {
    axis->SetBinLabel(ibin, categories[ibin - 1].c_str());
  }
}

void HistogramPool::create(const std::string& name,
                           const std::vector<std::string>& categories, bool weighted) {
  insert(name, [&]() {
    auto [nbins, xmin, xmax] = category_bins(categories);
    auto hist = new TH1F(name.c_str(), "", nbins, xmin, xmax);
    label_axis(hist->GetXaxis(), categories);
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
                           const int& bins, const double& xmin,
                           const double& xmax, bool weighted) {
  insert(name, [&]() {
    auto hist = new TH1F(name.c_str(), "", bins, xmin, xmax);
    hist->GetXaxis()->SetTitle(x_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
                           const std::vector<double>& bins, bool weighted) {
  insert(name, [&]() {
    auto hist = new TH1F(name.c_str(), "", bins.size() - 1, bins.data());
    hist->GetXaxis()->SetTitle(x_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const int& xbins, const double& xmin, const double& xmax,
            const std::string& y_label, const int& ybins, const double& ymin,
            const double& ymax, bool weighted) {
  insert(name, [&]() {
    auto hist =
        new TH2F(name.c_str(), "", xbins, xmin, xmax, ybins, ymin, ymax);
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const std::vector<double>& xbins,
            const std::string& y_label, const int& ybins, const double& ymin,
            const double& ymax, bool weighted) {
  insert(name, [&]() {
    auto hist =
        new TH2F(name.c_str(), "", xbins.size() - 1, xbins.data(), ybins, ymin, ymax);
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const std::vector<std::string>& xcategories,
            const std::string& y_label, const int& ybins, const double& ymin,
            const double& ymax, bool weighted) {
  insert(name, [&]() {
    auto [nxbins, xmin, xmax] = category_bins(xcategories);
    auto hist =
        new TH2F(name.c_str(), "", nxbins, xmin, xmax, ybins, ymin, ymax);
    label_axis(hist->GetXaxis(), xcategories);
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const int& xbins, const double& xmin, const double& xmax,
            const std::string& y_label, const std::vector<double>& ybins, bool weighted) {
  insert(name, [&]() {
    auto hist =
        new TH2F(name.c_str(), "", xbins, xmin, xmax, ybins.size() - 1, ybins.data());
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const int& xbins, const double& xmin, const double& xmax,
            const std::string& y_label, const std::vector<std::string>& ycategories, bool weighted) {
  insert(name, [&]() {
    auto [nybins, ymin, ymax] = category_bins(ycategories);
    auto hist =
        new TH2F(name.c_str(), "", xbins, xmin, xmax, nybins, ymin, ymax);
    label_axis(hist->GetYaxis(), ycategories);
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const std::vector<double>& xbins,
            const std::string& y_label, const std::vector<double>& ybins, bool weighted) {
  insert(name, [&]() {
    auto hist = new TH2F(name.c_str(), "", xbins.size() - 1, xbins.data(),
                         ybins.size() - 1, ybins.data());
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const std::vector<double>& xbins,
            const std::string& y_label, const std::vector<std::string>& ycategories, bool weighted) {
  insert(name, [&]() {
    auto [nybins, ymin, ymax] = category_bins(ycategories);
    auto hist =
        new TH2F(name.c_str(), "", xbins.size() - 1, xbins.data(), nybins, ymin, ymax);
    label_axis(hist->GetYaxis(), ycategories);
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

void HistogramPool::create(const std::string& name, const std::string& x_label,
            const std::vector<std::string>& xcategories,
            const std::string& y_label, const std::vector<std::string>& ycategories, bool weighted) {
  insert(name, [&]() {
    auto [nybins, ymin, ymax] = category_bins(ycategories);
    auto [nxbins, xmin, xmax] = category_bins(xcategories);
    auto hist =
        new TH2F(name.c_str(), "", nxbins, xmin, xmax, nybins, ymin, ymax);
    label_axis(hist->GetYaxis(), ycategories);
    label_axis(hist->GetXaxis(), xcategories);
    hist->GetXaxis()->SetTitle(x_label.c_str());
    hist->GetYaxis()->SetTitle(y_label.c_str());
    return hist;
  }, weighted);
}

}  // namespace framework
