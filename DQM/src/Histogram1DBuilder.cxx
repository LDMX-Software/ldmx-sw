/**
 * @file Histogram1DBuilder.cxx
 * @brief Builder used to construct 1D histograms.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "DQM/Histogram1DBuilder.h"

//----------//
//   ROOT   //
//----------//
#include "TH1F.h"
#include "TStyle.h"

namespace ldmx { 
  
    template <class T> 
        std::string Histogram1DBuilder<T>::name_{""};
    
    template <class T> 
        std::string Histogram1DBuilder<T>::title_{""};

    template <class T> 
        std::string Histogram1DBuilder<T>::xLabel_{""};

    template <class T>
        int Histogram1DBuilder<T>::bins_{-1}; 

    template <class T>
        double Histogram1DBuilder<T>::xLow_{-1}; 

    template <class T>
        double Histogram1DBuilder<T>::xHigh_{-1}; 

    template <class T> 
    Histogram1DBuilder<T>::Histogram1DBuilder(const std::string& name, 
            const int& bins, const double& xLow, const double& xHigh) {
       
        name_  = name;
        bins_  = bins; 
        xLow_  = xLow; 
        xHigh_ = xHigh;
        title_ = "";
        xLabel_ = "";

        gStyle->SetOptStat(0);
        gStyle->SetGridColor(17);
        gStyle->SetCanvasDefW(1100);
        gStyle->SetCanvasDefH(800);
        gStyle->SetCanvasColor(0);
        gStyle->SetCanvasBorderMode(0);
        gStyle->SetCanvasBorderSize(0);
        gStyle->SetPadBottomMargin(0.1);
        gStyle->SetPadTopMargin(0.01);
        gStyle->SetPadLeftMargin(0.1);
        gStyle->SetPadRightMargin(0.09);
        gStyle->SetPadGridX(1);
        gStyle->SetPadGridY(1);
        gStyle->SetPadTickX(1);
        gStyle->SetPadTickY(1);
        gStyle->SetFrameBorderMode(0);
        gStyle->SetTitleOffset(1.2, "yx");
        gStyle->SetOptLogy(1);
        gStyle->SetTitleFontSize(25);
 
    }

    template <class T>
    T* Histogram1DBuilder<T>::build() {
        T* hist = new T(name_.c_str(), name_.c_str(), bins_, xLow_, xHigh_);
        hist->SetTitle(title_.c_str()); 
        hist->GetXaxis()->SetTitle(xLabel_.c_str());
        hist->GetXaxis()->CenterTitle();
        hist->SetLineWidth(2); 
        return hist;
    }

    template class Histogram1DBuilder<TH1F>;
}
