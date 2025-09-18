#ifndef __FRAMEWORK_HISTOGRAM_POOL_H__
#define __FRAMEWORK_HISTOGRAM_POOL_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <memory>
#include <unordered_map>

//----------//
//   ROOT   //
//----------//
#include "TH1F.h"
#include "TH2F.h"

namespace framework {

/**
 * @class HistogramPool
 *
 * Class for holding an EventProcessor's histogram pointers
 * and making sure that they all end up in the same directory
 * in the output histogram file.
 */
class HistogramPool {
 private:
  /// The weight to fill histograms with
  double the_weight_{1.};

  /// The name of the processor that this helper is assigned to
  std::string name_;

  /// The directory all histograms should go into
  TDirectory* file_{nullptr};

  /**
   * the sub-directory that these histograms should go into
   *
   * We only create this directory if a histogram is created
   * so that we avoid creating empty directories in the output
   * histogram file.
   */
  TDirectory* directory_{nullptr};

  /**
   * Generic creation of a new histogram
   *
   * The input function is the one that calls `new` with the specific options.
   * I want to do this generic function so that we have one place that
   * has the directory creation and cd code and we can't just pass in the
   * `TH1` pointer because then the `new` is not called when we are in the
   * correct directory.
   *
   * @param[in] name name of histogram
   * @param[in] factory function that creates the TH1
   */
  void create(const std::string& name, std::function<(TH1*)()> factory);

 public:
  /**
   * Constructor
   *
   * Sets the name and output file along with updating
   * some of the style options for the histograms.
   */
  HistogramPool(TDirectory* file, const std::string& name);

  /**
   * Set the weight for filling the histograms
   */
  void setWeight(double w) { the_weight_ = w; }

  /**
   * get a histogram from this pool by name
   */
  TH1* get(const std::string& name);

  /**
   * Create a ROOT 1D histogram of type TH1F and pool it for later use.
   *
   * @note Does not check if another histogram of the same name is in use.
   *
   * @param name Name of the histogram. This will also be used as a
   *             title.
   * @param xLabel Title of the x axis.
   * @param bins Total number of histogram bins.
   * @param xmin The lower histogram limit.
   * @param xmax The upper histogram limit.
   */
  void create(const std::string& name, const std::string& xLabel,
              const double& bins, const double& xmin, const double& xmax);

  /**
   * Create a ROOT 1D histogram of type TH1F and pool it for later use.
   *
   * @note Does not check if another histogram of the same name is in use.
   *
   * @param name Name of the histogram. This will also be used as a
   *             title.
   * @param xLabel Title of the x axis.
   * @param bins vector of bin edges
   */
  void create(const std::string& name, const std::string& xLabel,
              const std::vector<double>& bins);

  /**
   * Create a ROOT 2D histogram of type TH2F and pool it for later use.
   *
   * @note Does not check if another histogram of the same name is in use.
   *
   * @param name Name of the histogram. This will also be used as a
   *             title.
   * @param xLabel Title of the x axis.
   * @param xbins Total number of histogram bins in x_.
   * @param xmin The lower histogram limit in x_.
   * @param xmax The upper histogram limit in x_.
   * @param yLabel Title of the x axis.
   * @param ybins Total number of histogram bins in y_.
   * @param ymin The lower histogram limit in y_.
   * @param ymax The upper histogram limit in y_.
   */
  void create(const std::string& name, const std::string& xLabel,
              const double& xbins, const double& xmin, const double& xmax,
              const std::string& yLabel, const double& ybins,
              const double& ymin, const double& ymax);

  /**
   * Create a ROOT 2D histogram of type TH2F and pool it for later use.
   *
   * @note Does not check if another histogram of the same name is in use.
   *
   * @param name Name of the histogram. This will also be used as a
   *             title.
   * @param xLabel Title of the x axis.
   * @param xbins Bin edges on x axis
   * @param yLabel Title of the y axis.
   * @param ybins Bin edges on y axis
   */
  void create(const std::string& name, const std::string& xLabel,
              const std::vector<double>& xbins, const std::string& yLabel,
              const std::vector<double>& ybins);

  /**
   * Fill a 1D histogram
   *
   * Uses the current setting of theWeight_.
   *
   * @param name name of the histogram to fill
   * @param val value to fill
   */
  void fill(const std::string& name, const double& val) {
    auto hist = dynamic_cast<TH1F*>(this->get(name));
    if (hist) {
      hist->Fill(val, the_weight_);
    }
  }

  /**
   * Fill a 2D histogram
   *
   * Uses the current setting of theWeight_.
   *
   * @param name name of the histogram to fill
   * @param valx x value to fill
   * @param valy y value to fill
   */
  void fill(const std::string& name, const double& valx, const double& valy) {
    auto hist = dynamic_cast<TH2F*>(this->get(name));
    if (hist) {
      hist->Fill(valx, valy, the_weight_);
    }
  }

};
}  // namespace framework

#endif  // __FRAMEWORK_HISTOGRAM_POOL_H__
