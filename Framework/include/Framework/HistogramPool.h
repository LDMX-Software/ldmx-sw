/**
 * @file HistogramPool.h
 * @brief Singleton class used to create and pool histograms.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __FRAMEWORK_HISTOGRAM_POOL_H__
#define __FRAMEWORK_HISTOGRAM_POOL_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory>
#include <unordered_map>
#include <iostream>

// Forward declarations
class TH1; 

namespace ldmx { 

    class HistogramPool {

        private: 

            /** Container for all histograms. */
            std::unordered_map< std::string, TH1* > histograms_;

            /** HistogramPool singlenton. */
            static HistogramPool* instance;

            /** Private constructor to prevent instantiation */
            HistogramPool(); 

        public: 

            static HistogramPool* getInstance();  

            /**
             * Create a ROOT 1D histogram of type T and pool it for later use.
             *
             * @param name Name of the histogram. This will also be used as a 
             *             title.
             * @param xLabel Title of the x axis.
             * @param bins Total number of histogram bins.
             * @param xmin The lower histogram limit.
             * @param xmax The upper histogram limit.
             */
            template <typename T>
            void create(const std::string& name, const std::string& xLabel, 
                        const int& bins, const int& xmin, const int& xmax) {
                
                // Create a histogram of type T
                T* hist = new T(name.c_str(), name.c_str(), bins, xmin, xmax);

                // Set the title
                hist->SetTitle(""); 

                // Set the x-axis label
                hist->GetXaxis()->SetTitle(xLabel.c_str()); 
                hist->GetXaxis()->CenterTitle(); 

                // Insert it into the pool of histograms for later use
                histograms_[name] = hist; 
            }

            /**
             * Create a ROOT 2D histogram of type T and pool it for later use.
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
            template <typename T>
            void create(const std::string& name, const std::string& xLabel, 
                        const int& xbins, const int& xmin, const int& xmax,
                        const std::string& yLabel, 
                        const int& ybins, const int& ymin, const int& ymax) {
                
                // Create a histogram of type T
                T* hist = new T(name.c_str(), name.c_str(), xbins, xmin, xmax, ybins, ymin, ymax);

                // Set the title
                hist->SetTitle(""); 

                // Set the x-axis label
                hist->GetXaxis()->SetTitle(xLabel.c_str()); 
                hist->GetXaxis()->CenterTitle(); 

                // Set the x-axis label
                hist->GetYaxis()->SetTitle(yLabel.c_str()); 
                hist->GetYaxis()->CenterTitle(); 

                // Insert it into the pool of histograms for later use
                histograms_[name] = hist; 
            }

            /** 
             * @return Retrieve the histogram named "name" from the pool. 
             */
            TH1* get(const std::string& name);

    }; // HistogramPool

} // ldmx

#endif // __FRAMEWORK_HISTOGRAM_POOL_H__
