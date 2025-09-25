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
 * # Usage
 * Each of the EventProcessors have a `histograms_` member variable that
 * developers can use within their producers and analyzers.
 * You need to create the histograms before filling them.
 * There are three different ways to specify the bins of a histogram:
 * - uniform bins: provide the number of bins, the minimum, and the maximum
 * - variable bins: provide the full list of bin edges (as a std::vector)
 * - categories: provide a list of named categories (as a std::vector<std::string>)
 *
 * With these three different ways to specify bins, there are three different
 * ways to create a 1D histogram and eight different ways to create a 2D histogram.
 *
 * In `onProcessStart()`, you create the histograms that you will want to fill.
 * For example,
 * ```cpp
 * histograms_.create("my_variable", "Label for Axis", 10, 0.0, 1.0);
 * ```
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
   * @param[in] weighted true if histogram is weighted (we then call Sumw2) or false otherwise
   * @throws framework::Exception if the passed name already exists
   */
  void insert(const std::string& name, std::function<TH1*()> factory, bool weighted);

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
   * Create a ROOT 1D histogram of type TH1F and pool it for later use.
   *
   * @note Does not check if another histogram of the same name is in use.
   *
   * @param name Name of the histogram. This will also be used as a
   *             title.
   * @param x_label Title of the x axis.
   * @param bins Total number of histogram bins.
   * @param xmin The lower histogram limit.
   * @param xmax The upper histogram limit.
   */
  void create(const std::string& name, const std::string& x_label,
              const int& bins, const double& xmin, const double& xmax,
              bool weighted = false);

  /**
   * Create a ROOT 1D histogram of type TH1F and pool it for later use.
   *
   * @param name Name of the histogram. This will also be used as a
   *             title.
   * @param x_label Title of the x axis.
   * @param bins vector of bin edges
   */
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& bins, bool weighted = false);

  void create(const std::string& name,
              const std::vector<std::string>& categories,
              bool weighted = false);

  /**
   * Create a ROOT 2D histogram of type TH2F and pool it for later use.
   *
   * @param name Name of the histogram. This will also be used as a
   *             title.
   * @param x_label Title of the x axis.
   * @param xbins Total number of histogram bins in x_.
   * @param xmin The lower histogram limit in x_.
   * @param xmax The upper histogram limit in x_.
   * @param y_label Title of the x axis.
   * @param ybins Total number of histogram bins in y_.
   * @param ymin The lower histogram limit in y_.
   * @param ymax The upper histogram limit in y_.
   */
  void create(const std::string& name, const std::string& x_label,
              const int& xbins, const double& xmin, const double& xmax,
              const std::string& y_label, const int& ybins, const double& ymin,
              const double& ymax, bool weighted = false);
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& xbins,
              const std::string& y_label, const int& ybins, const double& ymin,
              const double& ymax, bool weighted = false);
  void create(const std::string& name, const std::string& x_label,
              const std::vector<std::string>& xcategories,
              const std::string& y_label, const int& ybins, const double& ymin,
              const double& ymax, bool weighted = false);
  void create(const std::string& name, const std::string& x_label,
              const int& xbins, const double& xmin, const double& xmax,
              const std::string& y_label, const std::vector<double>& ybins, bool weighted = false);
  void create(const std::string& name, const std::string& x_label,
              const int& xbins, const double& xmin, const double& xmax,
              const std::string& y_label, const std::vector<std::string>& ycategories, bool weighted = false);
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& xbins,
              const std::string& y_label, const std::vector<double>& ybins, bool weighted = false);
  void create(const std::string& name, const std::string& x_label,
              const std::vector<double>& xbins,
              const std::string& y_label, const std::vector<std::string>& ycategories, bool weighted = false);
  void create(const std::string& name, const std::string& x_label,
              const std::vector<std::string>& xcategories,
              const std::string& y_label, const std::vector<std::string>& ycategories, bool weighted = false);

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
  void fillw(const std::string& name, const Tx& valx, const Ty& valy, double w) {
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
