#include <TH1.h>
#include <TF1.h>
#include <TH2.h>
#include <TLine.h>
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


void makeEventCanvas(string inFile, int maxEvents = 200, int deadChannel =8, int nChannels=16)
{
  // plotting macro for all pulse shapes in an event 
   int nSiPMs=12; //some channels not connected to SiPMs

   gErrorIgnoreLevel = kWarning; //suppress file writing message output 
   //these are derived as the mean of gaussian fits to the "event pedestal" (average over middle two quartiles) for each channel
   // TODO: pull these in from a pedestal file instead (either a default one or the one derived for the run at hand, although it doesn't move much)
   std::vector <float> pedestals={2.96771, -0.781398, -1.21252, 3.85539, 2.70707, -2.12383, 2.84691, -1.97643, -0.610725, -4.4, 2.3167, 2.02866, 3.3, -0.3, 1.3, 1.3};
								  /*-4.6, //0.6,
            -2.6, //4.4,
            -0.6, //-1.25,
            4.5,  //3.9,      // #3
            1.9,  //10000., // #4: (used to be) dead channel during test beam
            -2.2, //-2.1,   // #5
            0.9,  //2.9,    // #6
            -1.2, //-2,     // #7
            4.8,  //-0.4,   // #8
            -4.4, //-1.1,   // #9: dead channel in TTU teststand setup
            -0.1, //1.5,    // #10
            -1.7, //2.0,    // #11
            3.3,  //3.7,    // #12 -- uninstrumented
            -0.3, //2.8,    // #13 -- uninstrumented
            1.3,  //-1.5,   // #14 -- uninstrumented
            1.3   //1.6     // #15 -- uninstrumented
								  */

   // some path cleanup that could be done more methodically 
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

   TCanvas * c1 = new TCanvas("c1", "canvas", 800, 1400);
   c1->Divide(2, 8);  //16 in total                              
   TCanvas * c2 = new TCanvas("c2", "scaled canvas", 800, 1400);
   c2->Divide(2, nSiPMs/2);  //16 in total
   
   TFile * fIn = TFile::Open( inFile.c_str(), "READ");
   if (!fIn)
	 std::cout << "Couldn't open " << inFile << std::endl;
   fIn->cd("QIEplotMaker");
   fIn->ls();
   
   TLatex * latex = new TLatex();
   latex->SetNDC();
   latex->SetTextSize(0.2);

   TF1 * fPed = new TF1("fPed", "[0]", -5, 50);
   fPed->SetLineStyle(2);
   fPed->SetLineColor(kBlue+2);
   TF1 * fThr = new TF1("fThr", "[0]", -5, 50);
   fThr->SetLineStyle(3);
   fThr->SetLineColor(kRed+2);
   TLegend * leg = new TLegend(0.45, 0.2, 0.99, 0.85);
   leg->AddEntry(fPed, "Determined overall pedestal", "L");
   leg->AddEntry(fThr, "Event pedestal / plateau ", "L");

   TH2F * tdcFireMap = (TH2F*)gDirectory->Get("hTDCfireChanvsEvent");
   float leftMarg, rightMarg, topMarg, bottomMarg;
   float leftMarg2, rightMarg2, topMarg2, bottomMarg2;
   bool hasDrawnCanvas=false;
   for (int iE = 1; iE < maxEvents; iE++) { //start from event number 1
	 vector <TH1F*> vH(nChannels);
	 float maxQ=0.;
	 float minQ=0.;
	 float ped=0;
	 TLine *l = NULL;
	
	 for (int iC = 0 ; iC < nChannels ; iC++) {
	   //for (int iC = 0 ; iC < nSiPMs ; iC++) {
	   bool drawTDC = false;
	   ped = 0;
	   TH1F * h =(TH1F*)gDirectory->Get(Form("hCharge_chan%i_ev%i", iC,iE));
	   if (!h ) {
		 std::cout << "Couldn't find histogram " << Form("hCharge_chan%i_ev%i",iC,iE) << std::endl;
		 continue;
	   }	  //	  h->SetDirectory(0);
	   if (h->GetMaximum() > maxQ )
		 maxQ = h->GetMaximum();
	   if (h->GetMinimum() < minQ )
		 minQ = h->GetMinimum();
	   h->GetYaxis()->SetTitleSize(2.3*h->GetYaxis()->GetTitleSize());
	   h->GetXaxis()->SetTitleSize(2.3*h->GetXaxis()->GetTitleSize());
	   h->GetYaxis()->SetLabelSize(2.*h->GetYaxis()->GetLabelSize());
	   h->GetXaxis()->SetLabelSize(2.15*h->GetXaxis()->GetLabelSize());
	   h->GetYaxis()->SetTitleOffset(0.54); 
	   h->GetXaxis()->SetTitleOffset(0.45*h->GetXaxis()->GetTitleOffset());
	   h->SetMarkerSize(0.2);
	   h->GetYaxis()->SetMaxDigits(3);
	   //	  TGaxis::SetExponentOffset(0.1, 0.93, "y");
	   if ( tdcFireMap ) {
		 int fired = tdcFireMap->GetBinContent( iC+1, iE+1);
		 h->SetLineColor( fired/2 + 1); 
		 l = new TLine(fired,h->GetMinimum(),fired,h->GetMaximum());
		 l->SetLineColor( kGreen-7 );
		 l->SetLineWidth( 3 );
		 if ( fired > 0 )
		   drawTDC=true;
	   }
	   c1->cd(iC+1);
	   if (!hasDrawnCanvas) {
		 leftMarg=0.9*gPad->GetLeftMargin();
		 rightMarg=1.8*gPad->GetRightMargin();
		 topMarg=1.5*gPad->GetTopMargin();
		 bottomMarg=gPad->GetBottomMargin();
		 gPad->SetGridx();
	   }
	   gPad->SetTopMargin( topMarg );
	   gPad->SetBottomMargin( bottomMarg );
	   gPad->SetLeftMargin( leftMarg );
	   gPad->SetRightMargin( rightMarg );
	   // use DrawCopy here since histograms and function parameters are otherwise updated later on
	   h->DrawCopy("he"); 
	   if (drawTDC)
		 l->Draw("same");
	   //here: calculate the current event pedestal
	   vector<float> vals;
	   for (int iB = 1; iB <= h->GetNbinsX(); iB++ )
		 vals.push_back( h->GetBinContent(iB) );
	   std::sort(vals.begin(), vals.end());
	   int quartLength=(int)vals.size()/4;
	   for (int i = quartLength; i < 3*quartLength ; i++) {
		 ped+=vals[i];
	   }
	   ped/=2*quartLength;
	   fPed->SetParameter(0, pedestals.at(iC));
	   fPed->DrawCopy("same");
	   fThr->SetParameter(0, ped); 
	   fThr->DrawCopy("same");
	   if ( iC == deadChannel ) { //this channel is dead. use its space for some info!
		 latex->SetTextSize(0.2);
		 latex->DrawLatex(0.24, 0.75, Form("Event %i",iE) ); 
		 leg->Draw();
	   }
	 }
	 for (int iC = 0 ; iC < nSiPMs ; iC++) {
	   TH1F * h =(TH1F*)gDirectory->Get(Form("hCharge_chan%i_ev%i", iC,iE));
	   c2->cd(iC+1);
	   if (!hasDrawnCanvas) {
		 gPad->SetGridx();
		 //if ( iC < 2)
		 topMarg2=2.35*gPad->GetTopMargin();
		 //else   // actually, need room for exponent on top in all cases :( 
		 //		  gPad->SetTopMargin( 1.99*gPad->GetTopMargin());
		 bottomMarg2=1.75*gPad->GetBottomMargin();
		 leftMarg2=1.19*gPad->GetLeftMargin();
		 rightMarg2=0.5*gPad->GetRightMargin();
	   }
	   gPad->SetTopMargin( topMarg2 );
	   gPad->SetBottomMargin( bottomMarg2 );
	   gPad->SetLeftMargin( leftMarg2 );
	   gPad->SetRightMargin( rightMarg2 );

	   h->GetYaxis()->SetRangeUser(minQ, 1.2*maxQ);
	   h->SetTitle(";Time sample;Charge [fC]");
	   h->GetYaxis()->SetMaxDigits(3);
	   h->GetXaxis()->SetRangeUser(0, 30);
	   h->GetXaxis()->SetTitleOffset(0.91);
	   h->GetYaxis()->SetTitleOffset(0.81);
	   h->SetLineColor( kBlack ); //remove TDC colouring
	   h->DrawCopy("hist");
	   fPed->SetParameter(0, pedestals.at(iC));
	   //	  fPed->DrawCopy("same");
	   fThr->SetParameter(0, ped);
	   //fThr->DrawCopy("same");
	   /*
	   if ( iC == deadChannel ) { //this channel is dead. uncomment to use its space for some info
		 latex->DrawLatex(0.2, 0.49, Form("Event %i rescaled",iE) );
		 leg->Draw();
	   }
	   */
	   latex->SetTextSize(0.11);
	   if (iC < 2)
		 latex->DrawLatex(0.67, 0.75, Form("Channel %i",iC) );
	   else
		 latex->DrawLatex(0.67, 0.91, Form("Channel %i",iC) );
	 }
	 hasDrawnCanvas = true;
	 //	c2->cd();
	 //	latex->DrawLatex(0.4, 0.9, Form("Event %i",iE) ); //add event number
	 // save space by not saving the .png, uncomment if needed
	 // c1->SaveAs(Form("%s/hCharge_ev%i.png", outDir.Data(), iE));
	 c1->SaveAs(Form("%s/hCharge_ev%i.pdf", outDir.Data(), iE));
	 c2->SetTopMargin(0.2);
	 c2->cd(2);
	 //	LDMXLabelFixSize(0.6, 0.91, 0.13, "2022", 0.8);
	 // save space by not saving the .png, uncomment if needed
	 //c2->SaveAs(Form("%s/hCharge_ev%i_standardized.png", outDir.Data(), iE));
	 c2->SaveAs(Form("%s/hCharge_ev%i_standardized.pdf", outDir.Data(), iE));
   }
  
   //Done.
}
