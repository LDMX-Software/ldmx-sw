#ifndef __FRAMEWORK_HISTOGRAM_POOL_H__
#define __FRAMEWORK_HISTOGRAM_POOL_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory>
#include <unordered_map>
#include <iostream>

//----------//
//   ROOT   //
//----------//
#include "TH2F.h"
#include "TH1F.h"

namespace ldmx { 


    /**
     * @class HistogramPool
     *
     * Singleton class used to create and pool histograms.
     * 
     * Helpful for managing all those TH1 pointers by name instead of using variables.
     */
    class HistogramPool {

        private: 

            /** Container for all histograms. */
            std::unordered_map< std::string, TH1* > histograms_;

            /** 
             * Private constructor to prevent instantiation 
             *
             * Sets some style options as well.
             */
            HistogramPool();

        public: 

            /// Hide copy constructor
            HistogramPool(HistogramPool const&) = delete;

            /// Hide assignment operator
            void operator=(HistogramPool const&) = delete;

            /// Access the single instance of HistogramPool by reference
            static HistogramPool& getInstance();  

            /**
             * Insert a histogram into the pool
             *
             * @note Does not check for any doubling of names!
             */
            void insert(const std::string& name, TH1* hist) {
                histograms_[name] = hist;
            }

            /** 
             * Get a histogram using its name.
             *
             * Checks if histogram exists.
             *
             * @return Retrieve the histogram named "name" from the pool. 
             */
            TH1* get(const std::string& name);

    }; // HistogramPool

    /**
     * @class HistogramHelper
     *
     * Interface class between an EventProcessor and the HistogramPool
     */
    class HistogramHelper {

        private:

            /// The weight to fill histograms with
            double theWeight_{1.};

            /// The name of the processor that this helper is assigned to
            std::string name_;

        public:

            /**
             * Constructor
             *
             * Sets the name
             */
            HistogramHelper(const std::string& name) : name_(name) { }

            /**
             * Set the weight for filling the histograms
             */
            void setWeight(double w) { theWeight_ = w; }

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
            void create(const std::string& name, const std::string& xLabel, const std::vector<double>& bins );

            /**
             * Create a ROOT 2D histogram of type TH2F and pool it for later use.
             *
             * @note Does not check if another histogram of the same name is in use.
             *
             * @param name Name of the histogram. This will also be used as a 
             *             title.
             * @param xLabel Title of the x axis.
             * @param xbins Total number of histogram bins in x.
             * @param xmin The lower histogram limit in x.
             * @param xmax The upper histogram limit in x.
             * @param yLabel Title of the x axis.
             * @param ybins Total number of histogram bins in y.
             * @param ymin The lower histogram limit in y.
             * @param ymax The upper histogram limit in y.
             */
            void create(const std::string& name, const std::string& xLabel, 
                        const double& xbins, const double& xmin, const double& xmax,
                        const std::string& yLabel, 
                        const double& ybins, const double& ymin, const double& ymax);

            /**
             * Fill a 1D histogram
             *
             * Uses the current setting of theWeight_.
             *
             * @param name name of the histogram to fill
             * @param val value to fill
             */
            void fill(const std::string& name, const double& val) {
                dynamic_cast<TH1F*>(this->get(name))->Fill( val , theWeight_ );
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
            void fill(const std::string& name, const double& valx , const double& valy) {
                dynamic_cast<TH2F*>(this->get(name))->Fill( valx , valy , theWeight_ );
            }

            /**
             * Get a pointer to a histogram by name
             *
             * @param name name of the histogram to get
             */
            TH1* get(const std::string& name) {
                return HistogramPool::getInstance().get( name_+"_"+name );
            }
    };
} // ldmx

#endif // __FRAMEWORK_HISTOGRAM_POOL_H__
