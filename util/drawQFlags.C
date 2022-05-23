#include <TH1.h>
#include <TF1.h>
#include <TH2.h>
#include <TLine.h>
#include <TLatex.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TSystem.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>


void drawQFlags(string inFile, int flag, int maxEvents = 200, int deadChannel =8, int nChannels=16)
{
  // plotting macro for histograms of channel readout with a given quality flag set

   int nSiPMs=12; //some channels not connected to SiPMs

   gErrorIgnoreLevel = kWarning; //suppress file writing output 
   //these are derived as the mean of gaussian fits to total Q in the event. they an vary some but these are ok on average 
   std::vector <float> pedestals={2.96771, -0.781398, -1.21252, 3.85539, 2.70707, -2.12383, 2.84691, -1.97643, -0.610725, -0.9, 2.3167, 2.02866, 3.3, -0.3, 1.3, 1.3};

   TString outDir(inFile);
   outDir.ReplaceAll("data/", "");
   outDir.ReplaceAll("unpacked_", "");
   outDir.ReplaceAll("reformat_", "");
   outDir.ReplaceAll("reco", "");
   outDir.ReplaceAll("_linearize", "");
   outDir.ReplaceAll("linearize", "");
   outDir.ReplaceAll("hists", "");
   outDir.ReplaceAll("_plots", "");
   outDir.ReplaceAll("plots", "");
   outDir.ReplaceAll(".root", "");
   outDir.ReplaceAll("__", "_");

   outDir.Prepend("plots/");
   std::cout << "Using output subdir " << outDir <<  std::endl;

   if ( gSystem->mkdir(outDir.Data(), true) ) {
	 std::cout<<"Couldn't create directory " << outDir.Data() << std::endl;
   }

   TCanvas * c1 = new TCanvas("c1", "canvas", 600, 500);
   
   TFile * fIn = TFile::Open( inFile.c_str(), "READ");

   if (!fIn)
	 std::cout << "Couldn't open " << inFile << std::endl;
   
   fIn->cd("plotMaker"); 
   //  fIn->ls();
  
  TLatex * latex = new TLatex();
  latex->SetNDC();
  latex->SetTextSize(0.04);

  TF1 * fPed = new TF1("fPed", "[0]", -5, 50);
  fPed->SetLineStyle(2);
  fPed->SetLineColor(kBlue+2);

  float leftMarg, rightMarg, topMarg, bottomMarg;
  float leftMarg2, rightMarg2, topMarg2, bottomMarg2;
  bool hasDrawnCanvas=false;
  for (int iE = 1; iE < maxEvents; iE++) { //event counts start from event number 1
	vector <TH1F*> vH(nChannels);
	float maxQ=0.;
	float minQ=0.;
	float ped=0;
	
	for (int iC = 0 ; iC < nChannels ; iC++) {
	  //for (int iC = 0 ; iC < nSiPMs ; iC++) { //consider this to skip uninstrumented channels, but for now, they are a good reference point 
	  bool drawTDC = false;
	  ped = 0;
	  TH1F * h =(TH1F*)gDirectory->Get(Form("hCharge_flag%i_chan%i_nb%i", flag,iC,iE));
	  if (!h ) {
		std::cout << "Couldn't find histogram " << Form("hCharge_flag%i_chan%i_nb%i", flag,iC,iE) << std::endl;
		continue;
	  }
	  if (h->GetEntries() == 0 )
		continue;
	  if (h->GetMaximum() > maxQ )
		maxQ = h->GetMaximum();
	  if (h->GetMinimum() < minQ )
		minQ = h->GetMinimum();
	  h->DrawCopy("he"); 

	  //here: calculate the actual event pedestal 
	  vector<float> vals;
	  for (int iB = 1; iB <= h->GetNbinsX(); iB++ )
		vals.push_back( h->GetBinContent(iB) );
	  std::sort(vals.begin(), vals.end());
	  int quartLength=(int)vals.size()/4;
	  for (int i = quartLength; i < 3*quartLength ; i++) {
		ped+=vals[i];
	  }
	  ped/=2*quartLength;
	  fPed->SetParameter(0, ped);
	  fPed->DrawCopy("same");

	  TString titleStr(h->GetTitle());
	  titleStr.ReplaceAll(Form("Q, flag %i, ",flag), "");
	  titleStr.ReplaceAll(" [fC]", "");
	  latex->DrawLatex(0.67, 0.75, titleStr);

	  c1->SaveAs(Form("%s/hCharge_flag%i_ch%i_nb%i.pdf", outDir.Data(),flag,iC,iE));

	}
  }

  
  //Done.
}
