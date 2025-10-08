#ifndef __FRAMEWORK_HISTOGRAM_POOL_H__
#define __FRAMEWORK_HISTOGRAM_POOL_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <functional>
#include <unordered_map>

//----------//
//   ROOT   //
//----------//
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TH2F.h"

namespace framework {

/**
 * @class HistogramPool
 *
 * Class for holding an EventProcessor's histogram pointers
 * and making sure that they all end up in the same directory
 * in the output histogram file.
 *
 * @note In v4.5.1 of ldmx-sw and earlier, all of the processors shared
 * the same pool of histograms, so the naming was a little funky. If
 * a processor's name was `"myProc"` and a histogram's name was `"myHist"`,
 * the histogram would be written to `"myProc/myProc_myHist"` in the output
 * histogram file. After v4.5.2, this central pool was abandoned and the
 * output histograms are written to the more obvious location `"myProc/myHist"`
 * in the output histogram file.
 *
 * # Usage
 * Each of the EventProcessors have a `histograms_` member variable that
 * developers can use within their producers and analyzers.
 * You need to create the histograms before filling them.
 * There are three different ways to specify the bins of a histogram:
 * - uniform bins: provide the number of bins, the minimum, and the maximum
 * - variable bins: provide the full list of bin edges (as a std::vector)
 * - categories: provide a list of named categories (as a
 * std::vector<std::string>)
 *
 * With these three different ways to specify bins, there are three different
 * ways to create a 1D histogram and nine different ways to create a 2D
 * histogram. (Future developer note: if we want to increase the number of ways
 * to specify the bins
 * -- "axis type" -- * or increase the number of dimensions again, we probably
 * just want to use a builder pattern like Boost.Histogram or just use
 * Boost.Histogram under-the-hood and copy the resulting histograms into ROOT
 * histograms at end of run time.)
 *
 * In `onProcessStart()` or in the Python configuration,
 * you create the histograms that you will want to fill.
 * For example,
 * ```cpp
 * histograms_.create("my_variable", "Label for Axis", 10, 0.0, 1.0);
 * ```
 * or
 * ```python
 * analyzer.build1DHistogram("my_variable", "Label for Axis", 10, 0.0, 1.0)
 * ```
 *
 * And then in `analyze` or `produce` you fill the histograms.
 * ```cpp
 * histograms_.fill("my_variable", the_value);
 * ```
 *
 * Attempting to `create` histograms without the Python configuration
 * providing an output histogram file will fail because then there
 * is no place for the histograms to be saved.
 *
 * The `fill` function uses the current setting of the weight in the
 * `histograms_`object. This is helpful if, for example, you are putting one
 * entry in the histogram for each event and you want the histogram to use
 * the event weights.
 * In order to do this event weighting for histograms, you would add
 * the following line and **all** of the histograms that are `fill`ed _after_
 * this line will use the event weight.
 * ```cpp
 * histograms_.setWeight(event.getEventWeight());
 * ```
 * If you don't want to use special weighting, that's fine. The default
 * weight is 1 and you may even have separate histograms in your processor
 * that use different weights for example.
 * ```cpp
 * histograms_setWeight(1);
 *
 * histograms_.fill("h_without_event_weight", value);
 *
 * histograms_.setWeight(event.getEventWeight());
 *
 * histograms_.fill("h_with_event_weight", value);
 * ```
 * Additionally, if you want to use individual weights for the different
 * histograms, you can use the `fillw` function to provide a weight when
 * calling `fill`.
 *
 * @note By default, the histograms do not store the sum of the square weights
 * in order to save space. If you are filling histograms with weights that are
 * not 1 and you care about correct error estimates using those histograms,
 * make sure to `create` the histogram with the `weighted` argument set to
 * `true`.
 */
class HistogramPool {
 private:
  /// The weight to fill histograms with
  double the_weight_{1.};
  /// the pool of histogram pointers
  std::unordered_map<std::string, TH1*> histograms_;
  /**
   * the callback to get the directory these histograms should go in
   *
   * This needs to be dynamic so that the directory is only created
   * upon request.
   *
   * @note the returned TDirectory pointer is immediately de-referenced,
   * so we will get a segmentation fault if the function stored here returns
   * an invalid address (such as nullptr).
   */
  std::function<TDirectory*()> get_directory_;

  /**
   * insert a histogram into this pool by name
   *
   * @note The `fill` method assumes we are creating TH1F or TH2F histograms
   * which is what is implemented in the create methods.
   *
   * @param[in] name name to store histogram under in the pool, the `create`
   * methods below ensure that the name in the pool and the name in the output
   * file are the same
   * @param[in] factory the function that creates the histogram
   * which is called after we go into the appropriate output directory,
   * the returned pointer is then put into the pool after checking that
   * it doesn't exist yet.
   * @param[in] weighted true if histogram is weighted (we then call Sumw2) or
   * false otherwise
   * @throws framework::Exception if the passed name already exists
   */
  void insert(const std::string& name, std::function<TH1*()> factory,
              bool weighted);

 public:
  /// define how we can get the directory we need
  HistogramPool(std::function<TDirectory*()> get_directory)
      : get_directory_{get_directory} {}

  /**
   * Set the weight for filling the histograms
   */
  void setWeight(double w) { the_weight_ = w; }

  /**
   * get a histogram from this pool by name
   */
  TH1* get(const std::string& name);

  /**
   * Create a histogram from the input configuration parameters
   */
  void create(const config::Parameters& p);

  /**
   * Create a ROOT 1D histogram with uniform binning
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param bins Total number of histogram bins.
   * @param xmin The lower histogram limit.
   * @param xmax The upper histogram limit.
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const int& bins, const double& xmin, const double& xmax,
              bool weighted = false);

  /**
   * Create a ROOT 1D histogram with variable binning
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param bins vector of bin edges
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& bins, bool weighted = false);

  /**
   * Create a ROOT 1D histogram with categorical binning
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param categories list of string categories to name the bins
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<std::string>& categories,
              bool weighted = false);

  /**
   * Create a ROOT 2D histogram with both axes having uniform binning
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xbins Total number of histogram bins in x.
   * @param xmin The lower histogram limit in x.
   * @param xmax The upper histogram limit in x.
   * @param y_label Title of the x axis.
   * @param ybins Total number of histogram bins in y.
   * @param ymin The lower histogram limit in y.
   * @param ymax The upper histogram limit in y.
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const int& xbins, const double& xmin, const double& xmax,
              const std::string& y_label, const int& ybins, const double& ymin,
              const double& ymax, bool weighted = false);

  /**
   * Create a ROOT 2D histogram with variable bins on the x axis
   * and uniform bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xbins vector of bin edges
   * @param y_label Title of the x axis.
   * @param ybins Total number of histogram bins in y.
   * @param ymin The lower histogram limit in y.
   * @param ymax The upper histogram limit in y.
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& xbins, const std::string& y_label,
              const int& ybins, const double& ymin, const double& ymax,
              bool weighted = false);

  /**
   * Create a ROOT 2D histogram with category bins on the x axis
   * and uniform bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xcategories vector of string categories
   * @param y_label Title of the x axis.
   * @param ybins Total number of histogram bins in y.
   * @param ymin The lower histogram limit in y.
   * @param ymax The upper histogram limit in y.
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<std::string>& xcategories,
              const std::string& y_label, const int& ybins, const double& ymin,
              const double& ymax, bool weighted = false);

  /**
   * Create a ROOT 2D histogram with uniform bins on the x axis
   * and variable bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xbins Total number of histogram bins in x.
   * @param xmin The lower histogram limit in x.
   * @param xmax The upper histogram limit in x.
   * @param y_label Title of the x axis.
   * @param ybins vector of bin edges
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const int& xbins, const double& xmin, const double& xmax,
              const std::string& y_label, const std::vector<double>& ybins,
              bool weighted = false);

  /**
   * Create a ROOT 2D histogram with variable bins on the x axis
   * and variable bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xbins vector of bin edges
   * @param y_label Title of the x axis.
   * @param ybins vector of bin edges
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& xbins, const std::string& y_label,
              const std::vector<double>& ybins, bool weighted = false);

  /**
   * Create a ROOT 2D histogram with category bins on the x axis
   * and variable bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xcategories vector of string categories
   * @param y_label Title of the x axis.
   * @param ybins vector of bin edges
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<std::string>& xcategories,
              const std::string& y_label, const std::vector<double>& ybins,
              bool weighted = false);

  /**
   * Create a ROOT 2D histogram with uniform bins on the x axis
   * and category bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xbins Total number of histogram bins in x.
   * @param xmin The lower histogram limit in x.
   * @param xmax The upper histogram limit in x.
   * @param y_label Title of the x axis.
   * @param ycategories vector of string categories
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const int& xbins, const double& xmin, const double& xmax,
              const std::string& y_label,
              const std::vector<std::string>& ycategories,
              bool weighted = false);

  /**
   * Create a ROOT 2D histogram with variable bins on the x axis
   * and category bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xbins vector of bin edges
   * @param y_label Title of the x axis.
   * @param ycategories vector of string categories
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& xbins, const std::string& y_label,
              const std::vector<std::string>& ycategories,
              bool weighted = false);

  /**
   * Create a ROOT 2D histogram with category bins on the x axis
   * and category bins on the y axis
   *
   * @param name Name of the histogram.
   * @param x_label Title of the x axis.
   * @param xcategories vector of string categories
   * @param y_label Title of the x axis.
   * @param ycategories vector of string categories
   * @param weighted whether to track the sum of squared weights separately
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<std::string>& xcategories,
              const std::string& y_label,
              const std::vector<std::string>& ycategories,
              bool weighted = false);

  /**
   * Fill a 1D histogram
   *
   * Uses the current setting of the weight.
   *
   * @param name name of the histogram to fill
   * @param val value to fill
   */
  template <typename T>
  void fill(const std::string& name, const T& val) {
    auto hist = dynamic_cast<TH1F*>(this->get(name));
    if (hist) {
      hist->Fill(val, the_weight_);
    } else {
      // the `get` method handles checking if the histogram exists
      // the only way that hist would be null at this point is if its not a TH1F
      EXCEPTION_RAISE(
          "BadHistSize",
          "Attempting to 1D fill a histogram that is not actually 1D.");
    }
  }

  /**
   * Fill a 1D histogram
   *
   * Using the input weight.
   *
   * @param name name of the histogram to fill
   * @param val value to fill
   * @param w weight to fill with
   */
  template <typename T>
  void fillw(const std::string& name, const T& val, double w) {
    auto hist = dynamic_cast<TH1F*>(this->get(name));
    if (hist) {
      hist->Fill(val, w);
    } else {
      // the `get` method handles checking if the histogram exists
      // the only way that hist would be null at this point is if its not a TH1F
      EXCEPTION_RAISE(
          "BadHistSize",
          "Attempting to 1D fill a histogram that is not actually 1D.");
    }
  }

  /**
   * Fill a 2D histogram
   *
   * Uses the current setting of the weight.
   *
   * @param name name of the histogram to fill
   * @param valx x value to fill
   * @param valy y value to fill
   */
  template <typename Tx, typename Ty>
  void fill(const std::string& name, const Tx& valx, const Ty& valy) {
    auto hist = dynamic_cast<TH2F*>(this->get(name));
    if (hist) {
      hist->Fill(valx, valy, the_weight_);
    } else {
      // the `get` method handles checking if the histogram exists
      // the only way that hist would be null at this point is if its not a TH2F
      EXCEPTION_RAISE(
          "BadHistSize",
          "Attempting to 2D fill a histogram that is not actually 2D.");
    }
  }

  /**
   * Fill a 2D histogram
   *
   * Using the input weight.
   *
   * @param name name of the histogram to fill
   * @param valx x value to fill
   * @param valy y value to fill
   * @param w weight to fill with
   */
  template <typename Tx, typename Ty>
  void fillw(const std::string& name, const Tx& valx, const Ty& valy,
             double w) {
    auto hist = dynamic_cast<TH2F*>(this->get(name));
    if (hist) {
      hist->Fill(valx, valy, w);
    } else {
      // the `get` method handles checking if the histogram exists
      // the only way that hist would be null at this point is if its not a TH2F
      EXCEPTION_RAISE(
          "BadHistSize",
          "Attempting to 2D fill a histogram that is not actually 2D.");
    }
  }
};
}  // namespace framework

#endif  // __FRAMEWORK_HISTOGRAM_POOL_H__
