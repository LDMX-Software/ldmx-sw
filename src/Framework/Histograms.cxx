
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
  gStyle->SetOptStat(0);
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
  std::string fullName = name_ + "_" + name;

  // Create a histogram of type T
  auto hist = new TH1F(fullName.c_str(), fullName.c_str(), bins, xmin, xmax);

  // Set the title
  hist->SetTitle("");

  // Set the x-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(fullName, hist);
}

void HistogramHelper::create(const std::string& name, const std::string& xLabel,
                             const std::vector<double>& bins) {
  std::string fullName = name_ + "_" + name;

  // copy bin edges into a C98 form acceptable by ROOT
  int nbins = bins.size() - 1;
  double* binEdges = new double[bins.size()];
  for (unsigned int iBin = 0; iBin < bins.size(); iBin++)
    binEdges[iBin] = bins.at(iBin);

  auto hist = new TH1F(fullName.c_str(), fullName.c_str(), nbins, binEdges);

  delete[] binEdges;  // cleanup

  // Set the title
  hist->SetTitle("");

  // Set the x-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(fullName, hist);
}

void HistogramHelper::create(const std::string& name, const std::string& xLabel,
                             const double& xbins, const double& xmin,
                             const double& xmax, const std::string& yLabel,
                             const double& ybins, const double& ymin,
                             const double& ymax) {
  std::string fullName = name_ + "_" + name;

  // Create a histogram of type T
  auto hist = new TH2F(fullName.c_str(), fullName.c_str(), xbins, xmin, xmax,
                       ybins, ymin, ymax);

  // Set the title
  hist->SetTitle("");

  // Set the x-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Set the x-axis label
  hist->GetYaxis()->SetTitle(yLabel.c_str());
  hist->GetYaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(fullName, hist);
}

void HistogramHelper::create(const std::string& name, const std::string& xLabel,
                             const std::vector<double>& xbins,
                             const std::string& yLabel,
                             const std::vector<double>& ybins) {
  std::string fullName = name_ + "_" + name;

  // copy bin edges into a C98 form acceptable by ROOT
  int xNBins = xbins.size() - 1;
  double* xBinEdges = new double[xbins.size()];
  for (unsigned int iBin = 0; iBin < xbins.size(); iBin++)
    xBinEdges[iBin] = xbins.at(iBin);

  int yNBins = ybins.size() - 1;
  double* yBinEdges = new double[ybins.size()];
  for (unsigned int iBin = 0; iBin < ybins.size(); iBin++)
    yBinEdges[iBin] = ybins.at(iBin);

  auto hist = new TH2F(fullName.c_str(), fullName.c_str(), xNBins, xBinEdges,
                       yNBins, yBinEdges);

  delete[] xBinEdges;  // cleanup
  delete[] yBinEdges;  // cleanup

  // Set the title
  hist->SetTitle("");

  // Set the x-axis label
  hist->GetXaxis()->SetTitle(xLabel.c_str());
  hist->GetXaxis()->CenterTitle();

  // Set the x-axis label
  hist->GetYaxis()->SetTitle(yLabel.c_str());
  hist->GetYaxis()->CenterTitle();

  // Insert it into the pool of histograms for later use
  HistogramPool::getInstance().insert(fullName, hist);
}
}  // namespace framework
