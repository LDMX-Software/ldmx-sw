#include <TH1.h>
#include <TF1.h>
#include <TH2.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TGraphErrors.h>


#include <string>
#include <vector>
#include <iostream>
#include <fstream>


// Quadratic background function
Double_t background(Double_t *x, Double_t *par) {
  return par[0] + par[1]*x[0] + par[2]*x[0]*x[0];
}

// Lorentzian Peak function
Double_t lorentzianPeak(Double_t *x, Double_t *par) {
  return (0.5*par[0]*par[1]/TMath::Pi()) / TMath::Max(1.e-10,
													  (x[0]-par[2])*(x[0]-par[2])+ .25*par[1]*par[1]);
}


//some function with a broader low side
Double_t asymPeak(Double_t *x, Double_t *par) {
  Double_t expo=TMath::Exp((x[0]-par[1])/par[0]);
  par[2]=sqrt(fabs(par[1]));
  return 1/par[0]*expo*TMath::Exp( -expo );
  //f(x)=1βe−x−μβe−e−x−μβ
}

// Gaussian Peak function                                                                                                                                    
Double_t gaussianPeak(Double_t *x, Double_t *par) {
  return par[0]*TMath::Exp(  -0.5*((x[0]-par[1])/par[2])*((x[0]-par[1])/par[2])  );
}


// Sum of background and peak function
Double_t fitFunction(Double_t *x, Double_t *par) {
  //  return background(x,par) + lorentzianPeak(x,&par[3]);
  return gaussianPeak(x,par) + background(x,&par[3]);
}
void extractResponse(string inFile, int deadChannel=-1, string passName = "TBreco", int nSamples = 30, int verbosity=2, bool isSim=false, bool doClean=false)
{
  // macro extracting the per-channel MIP response based on calibrated hits
  //

  // we need hits, that are cleaned and calibrated
  // the average PE response, fit it --> this is what we calibrate to
  // then for each channel, fit and get mean
  // store the per-channel scale factor so we can apply it in the MIP regime 
  // done
  

  bool verbosePrint = (verbosity == 0 ); //adhere to ldmx printout lavel numbering
  
  TFile * fIn = TFile::Open( inFile.c_str(), "READ");
  TTree * tree = (TTree*)fIn->Get("LDMX_Events");
  fIn->ls();
  int exampleEvNb = 1;

  string digiName="testBeamHitsUp";
  int nChannels = 12;
  
  vector<string> vars = {"pe"}; // this variable is totQ/nSamp
  vector <int> maxVals = {200};  // don't need to go high, focus on the single-few PE peaks 
  vector <int> minVals = {20};  // should cover most negative pedestals 
  vector <float> binFactor = {0.15}; // histogram binning: bins per integer 

  TCanvas * c1 = new TCanvas("c1", "plot canvas", 600, 500);

  vector<string> cuts;
  vector<TH1F*> v_hOut;

  string cleanStr="";
  if (doClean)
	cleanStr=Form("(%s_%s.flag_ == 0 || %s_%s.flag_ == 4) &&", digiName.c_str(),passName.c_str(), digiName.c_str(),passName.c_str());
  
  for (int iC = 0 ; iC < nChannels ; iC++)
	cuts.push_back( Form("%s %s_%s.barID_ == %i", cleanStr.c_str(), digiName.c_str(),passName.c_str(), iC));

  c1->SetRightMargin( 1.5*c1->GetRightMargin());

  for (unsigned int iV = 0; iV < vars.size(); iV++) {
	for (unsigned int iC = 0; iC < cuts.size(); iC++) {
	  string cut =cuts[iC];
	  //set up binning
	  string bins = Form("%i, %i, %i", (int)(binFactor[iV]*(maxVals[iV]-minVals[iV])), minVals[iV], maxVals[iV]);
	  if (iC == 0)
		std::cout << bins << std::endl;
	  //draw from the tree
	  string bin = Form("%i", iC);
	  string nSamp = Form("%i", nSamples);
	  std::cout<<"At channel " << bin << std::endl;
	  std::cout<<"Using cut " << cut << std::endl;
	  tree->Draw( (digiName+"_"+passName+"."+vars[iV]+"_ >> h"+bin+"("+bins+")").c_str(), cut.c_str() );
	  //get them each and keep for later
	  TH1F *hOut = (TH1F*)gDirectory->Get(Form("h%i", iC)); 
	  if (!hOut || hOut->IsZombie()) {
		std::cout << "No histogram for channel " << iC << ", skipping" << std::endl;
		continue;
	  }
	  //set up axes 	  
	  string title = ";"+vars[iV]+" "+cut+";entries";
	  hOut->SetTitle( title.c_str() );
	  hOut->GetYaxis()->SetRangeUser(0.001, 1.5*hOut->GetMaximum()) ;
	  //draw 
	  hOut->Draw();
	  hOut->SetName(Form("h%s_chanID%i", vars[iV].c_str(), iC));
	  v_hOut.push_back( (TH1F*)hOut->Clone() );
	}//over cuts
  }//over variables 


  //make a histogram with all channels

  TH1F * hAll = (TH1F*)v_hOut[0]->Clone("hPE_all");
  for (unsigned int iC = 1; iC < nChannels; iC++) {
	if (iC == deadChannel)
	  continue;
	hAll->Add(v_hOut[iC]);
  }

  TFitResultPtr fPtr = hAll->Fit("landau", "S");
  float avgResponse = fPtr->Parameter(1);
  std::cout << "Got MPV for all channels combined: " << avgResponse << std::endl;
	
  // ok so now we have all the histograms. we just need to fit them...

  float fitRangeWidth=125.; // take this window to either side of mean to catch peak 
  TGraphErrors* gMPVs = new TGraphErrors(nChannels);
  for (unsigned int iH = 0;  iH < v_hOut.size(); iH++) {
	if (iH == deadChannel)
	  continue;
	//do the actual peak fitting 
	std::cout<< "----> Fitting response for channel " << iH << std::endl;
	TFitResultPtr fResult = v_hOut.at(iH)->Fit("landau", "QS");
	std::cout << "For channel " << iH << ", got MPV \t" << fResult->Parameter(1) << std::endl;
	gMPVs->SetPoint( iH, iH, fResult->Parameter(1));
	gMPVs->SetPointError( iH, 0, fResult->ParError(1));
  }

  
  //store this to a root file for later plotting
  TString outFile = inFile;
  outFile=outFile.ReplaceAll(".root", "_response.root");
  std::cout << "using outfile name" "" << outFile << std::endl;

  TFile * fOut = TFile::Open( outFile.Data(), "RECREATE");
  fOut->cd();
  fOut->ls();
  std::cout << "Got " << v_hOut.size() << " histograms" << std::endl;
  int nChanToWrite = deadChannel > -1 ? nChannels - 1 : nChannels; //adjust for if we have a dead channel in this run
   for (unsigned int iH = 0;  iH < v_hOut.size(); iH++) {
	v_hOut[iH]->Write();
  }
  if (deadChannel > -1 ) { //if there in fact is one (TODO: allow for list?)
	gMPVs->RemovePoint(deadChannel);
  }
  gMPVs->SetTitle(";channel ID;MIP peak fit MPV");
  gMPVs->Write("gMIPResponseVsChanID");
  fOut->Close();
  fIn->Close();
  
  //write responses to a .txt file for easy extraction of calibration in the hit reconstruction step 
  TString responseFile = inFile;
  responseFile=responseFile.ReplaceAll(".root", "_response.txt");
  ofstream fResponse;// responseFile.Data());
  fResponse.open( responseFile.Data());
  for (int iP = 0; iP < gMPVs->GetN(); iP++) {
	double x, MPV;
    gMPVs->GetPoint(iP, x, MPV);
	fResponse << x << "," << avgResponse/MPV << "\n";
  }
  fResponse.close();
    //Done.
}

