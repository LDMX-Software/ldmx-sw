
#include "Framework/Histograms.h"

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

HistogramPool::HistogramPool() {
  gStyle->SetOptStat(1);
  gStyle->SetGridColor(17);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetTitleOffset(1.2, "yx");
  gStyle->SetTitleFontSize(25);

  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.09);
  gStyle->SetPadGridX(1);
  gStyle->SetPadGridY(1);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);

  gStyle->SetHistLineWidth(2);
}

HistogramPool& HistogramPool::getInstance() {
  // Create an instance of HistogramPool if needed
  //  Guarnteed to be destroyed, instantiaed on first use
  static HistogramPool instance;

  return instance;
}

TH1* HistogramPool::get(const std::string& name) {
  auto histo = histograms_.find(name);
  if (histo == histograms_.end()) {
    EXCEPTION_RAISE("InvalidArg", "Histogram " + name + " not found in pool.");
  }

  return histograms_[name];
}

void HistogramHelper::create(const std::string& name, const std::string& xLabel,
                             const double& bins, const double& xmin,
                             const double& xmax) {
  std::string full_name = name_ + "_" + name;

  // Create a histogram of type T
  auto hist = new TH1F(full_name.c_str(), full_name.c_str(), bins, xmin, xmax);

  // Set the title
  hist->SetTitle("");

  // Set the x_-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(full_name, hist);
}

void HistogramHelper::create(const std::string& name, const std::string& xLabel,
                             const std::vector<double>& bins) {
  std::string full_name = name_ + "_" + name;

  // copy bin edges into a C98 form acceptable by ROOT
  int nbins = bins.size() - 1;
  double* bin_edges = new double[bins.size()];
  for (unsigned int i_bin = 0; i_bin < bins.size(); i_bin++)
    bin_edges[i_bin] = bins.at(i_bin);

  auto hist = new TH1F(full_name.c_str(), full_name.c_str(), nbins, bin_edges);

  delete[] bin_edges;  // cleanup

  // Set the title
  hist->SetTitle("");

  // Set the x_-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(full_name, hist);
}

void HistogramHelper::create(const std::string& name, const std::string& xLabel,
                             const double& xbins, const double& xmin,
                             const double& xmax, const std::string& yLabel,
                             const double& ybins, const double& ymin,
                             const double& ymax) {
  std::string full_name = name_ + "_" + name;

  // Create a histogram of type T
  auto hist = new TH2F(full_name.c_str(), full_name.c_str(), xbins, xmin, xmax,
                       ybins, ymin, ymax);

  // Set the title
  hist->SetTitle("");

  // Set the x_-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Set the x_-axis label
  hist->GetYaxis()->SetTitle(yLabel.c_str());
  hist->GetYaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(full_name, hist);
}

void HistogramHelper::create(const std::string& name, const std::string& xLabel,
                             const std::vector<double>& xbins,
                             const std::string& yLabel,
                             const std::vector<double>& ybins) {
  std::string full_name = name_ + "_" + name;

  // copy bin edges into a C98 form acceptable by ROOT
  int x_n_bins = xbins.size() - 1;
  double* x_bin_edges = new double[xbins.size()];
  for (unsigned int i_bin = 0; i_bin < xbins.size(); i_bin++)
    x_bin_edges[i_bin] = xbins.at(i_bin);

  int y_n_bins = ybins.size() - 1;
  double* y_bin_edges = new double[ybins.size()];
  for (unsigned int i_bin = 0; i_bin < ybins.size(); i_bin++)
    y_bin_edges[i_bin] = ybins.at(i_bin);

  auto hist = new TH2F(full_name.c_str(), full_name.c_str(), x_n_bins,
                       x_bin_edges, y_n_bins, y_bin_edges);

  delete[] x_bin_edges;  // cleanup
  delete[] y_bin_edges;  // cleanup

  // Set the title
  hist->SetTitle("");

  // Set the x_-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Set the x_-axis label
  hist->GetYaxis()->SetTitle(yLabel.c_str());
  hist->GetYaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(full_name, hist);
}
}  // namespace framework
