/**
 * @file Histogram1DBuilder.h
 * @brief Builder used to construct 1D histograms.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _HISTOGRAM_1D_BUILDER_H_
#define _HISTOGRAM_1D_BUILDER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <string>

namespace ldmx { 

    
    template <class T>
    class Histogram1DBuilder {

        public: 

            /** Constructor */
            Histogram1DBuilder(const std::string& name, const int& bins, 
                               const double& xLow, const double& xHigh);

            /** Set the histogram title. */
            void title(const std::string& title) { title_ = title; };
            
            /** Set the x Label. */
            void xLabel(const std::string& xLabel) { xLabel_ = xLabel; }; 

            /** @return An instance of a histogram of type T (TH1F, etc.) */
            T* build();

        private: 

            /** Name of the histogram. */
            static std::string name_;

            /** The histogram title. */
            static std::string title_; 

            /** The title of the x-axis. */
            static std::string xLabel_; 

            /** The number of bins. */
            static int bins_; 

            /** The low edge of the histogram. */
            static double xLow_; 

            /** The high edge of the histogram. */
            static double xHigh_; 

    }; // HistogramBuilder

} // ldmx

#endif // _HISTOGRAM_1D_BUILDER_H_
