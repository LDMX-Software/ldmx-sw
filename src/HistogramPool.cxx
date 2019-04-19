/**
 * @file HistogramPool.h
 * @brief Singleton class used to create and pool histograms.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

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

namespace ldmx { 

    HistogramPool* HistogramPool::instance = nullptr;

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

    HistogramPool* HistogramPool::getInstance() { 
        
        // Create an instance of HistogramPool if needed
        if (!instance) instance = new HistogramPool; 

        return instance; 
    }

    TH1* HistogramPool::get(const std::string& name) { 
        auto histo = histograms_.find(name); 
        if (histo == histograms_.end()) { 
            throw std::invalid_argument("Histogram " + name + " not found.");  
        }  
        
        return histograms_[name];
    }
}
