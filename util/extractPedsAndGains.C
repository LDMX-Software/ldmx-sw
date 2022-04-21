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


TGraphErrors * findAndFitPeaks( TH1F * hIn, float width, bool verbose );
void extractPedsAndGains(string inFile, int deadChannel=-1, string decodePassName = "conv", int nSamples = 30, int verbosity=2)
{
  // macro extracting the per-channel gain and pedestal based on total Q histograms, and single PE peak fitting

  bool verbosePrint = (verbosity == 0 ); //adhere to ldmx printout lavel numbering
  
  TFile * fIn = TFile::Open( inFile.c_str(), "READ");
  TTree * tree = (TTree*)fIn->Get("LDMX_Events");
  fIn->ls();
  int exampleEvNb = 1;

  string digiName="QIEsamplesUp";
  int nChannels = 12;
  
  vector<string> vars = {"avgQ"}; // this variable is totQ/nSamp
  vector <int> maxVals = {2000};  // don't need to go high, focus on the single-few PE peaks 
  vector <int> minVals = {-500};  // should cover most negative pedestals 
  vector <float> binFactor = {0.05}; // histogram binning 

  TCanvas * c1 = new TCanvas("c1", "plot canvas", 600, 500);

  vector<string> cuts;
  vector<TH1F*> v_hOut;
  
  for (int iC = 0 ; iC < nChannels ; iC++)
	cuts.push_back( Form("%s_%s.chanID_ == %i", digiName.c_str(),decodePassName.c_str(), iC));

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
	  tree->Draw( (nSamp+"*("+digiName+"_"+decodePassName+"."+vars[iV]+"_) >> h"+bin+"("+bins+")").c_str(), cut.c_str() );
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


  // ok so now we have all the histograms. we just need to fit them...
  // it is reasonable to assume that we will have a hunch how wide each PE peak will be.
  // we can pass that param to the fitting function. 

  float fitRangeWidth=125.; // take this window to either side of mean to catch peak 
  vector <TGraphErrors*> v_g;
  TGraphErrors* gGains = new TGraphErrors(nChannels);
  TGraphErrors* gPeds = new TGraphErrors(nChannels);
  float scaleFac=6250.; //conversion from fC to e
  for (unsigned int iH = 0;  iH < v_hOut.size(); iH++) {
	if (iH == deadChannel)
	  continue;
	//do the actual peak fitting 
	std::cout<< "Fitting gain for channel " << iH << std::endl;
	TGraphErrors * g= findAndFitPeaks( v_hOut.at(iH), fitRangeWidth, verbosePrint); 
	g->Draw("ap");
	g->SetTitle(";Peak nb;Total charge [fC]");
	g->SetName(Form("g_gainChan%i", iH));
	TFitResultPtr fResult = g->Fit("pol1", "QS");
	std::cout << "For channel " << iH << ", got slope \t" << fResult->Parameter(1) << " fC/PE and intercept " << fResult->Parameter(0) << std::endl;
	std::cout << "Pedestal from intercept would be " << fResult->Parameter(0)/nSamples << std::endl;
	gGains->SetPoint( iH, iH, fResult->Parameter(1)*scaleFac);
	gGains->SetPointError( iH, 0, fResult->ParError(1)*scaleFac);
	double x, ped;
	g->GetPoint(0, x, ped);
	ped/=nSamples;
	float pedErr=g->GetErrorY( 0 );
	gPeds->SetPoint( iH, iH, ped ); 
	gPeds->SetPointError( iH, 0, pedErr); 
	std::cout << "Setting gain " << fResult->Parameter(1)*scaleFac << " and pedestal from first peak " << ped << " fC." << std::endl;

	v_g.push_back( g );
  }

  //store this to a root file for later plotting
  TString outFile = inFile;
  outFile=outFile.ReplaceAll(".root", "_gain.root");
  std::cout << "using outfile name" "" << outFile << std::endl;

  TFile * fOut = TFile::Open( outFile.Data(), "RECREATE");
  fOut->cd();
  fOut->ls();
  std::cout << "Got " << v_hOut.size() << " histograms" << std::endl;
  int nChanToWrite = deadChannel > -1 ? nChannels - 1 : nChannels; //adjust for if we have a dead channel in this run
   for (unsigned int iH = 0;  iH < v_hOut.size(); iH++) {
	v_hOut[iH]->Write();
	if (iH < nChanToWrite) 
	  v_g[iH]->Write();
  }
  if (deadChannel > -1 ) { //if there in fact is one (TODO: allow for list?)
	gGains->RemovePoint(deadChannel);
	gPeds->RemovePoint(deadChannel);
  }
  gGains->SetTitle(";channel ID;Gain");
  gGains->Write("gGainVsChanID");
  gPeds->SetTitle(";channel ID;Pedestal from fit [fC]");
  gPeds->Write("gPedestalVsChanID");
  fOut->Close();
  fIn->Close();
  
  //write gains to a .txt file for easy extraction of calibration in the hit reconstruction step 
  TString gainFile = inFile;
  gainFile=gainFile.ReplaceAll(".root", "_gains.txt");
  ofstream fGains;// gainFile.Data());
  fGains.open( gainFile.Data());
  for (int iP = 0; iP < gGains->GetN(); iP++) {
	double x, gain;
    gGains->GetPoint(iP, x, gain);
	fGains << x << "," << gain << "\n";
  }
  fGains.close();
  
  //finally write peds to a .txt file
  TString pedFile = inFile;
  pedFile=pedFile.ReplaceAll(".root", "_peds.txt");
  ofstream fPeds;
  fPeds.open( pedFile.Data());
  for (int iP = 0; iP < gPeds->GetN(); iP++) {
	double x, ped;
    gPeds->GetPoint(iP, x, ped);
	fPeds << x << "," << ped << "\n";
  }
  fPeds.close();
  //Done.
}


TGraphErrors* findAndFitPeaks( TH1F * hIn, float width, bool verbose )
{

  float mean = hIn->GetBinCenter( hIn->GetMaximumBin() );
  float maxVal = hIn->GetXaxis()->GetXmax();
  std::cout << "Limit on maxVal = " << maxVal << std::endl;
  float oldMean = hIn->GetXaxis()->GetXmin()+5;
  float minVal = oldMean;
  minVal = TMath::Max(mean-width,minVal);
  std::cout << "Starting with minVal = " << minVal << std::endl;
  vector<TFitResultPtr> vPtrs;
  int iP = 0;
  int norm = 0;
  TF1 * fGaus = new TF1("fGaus", "gaus", -50, 5000);

  while ( mean + 1.5*width < maxVal ) {
	float max=TMath::Min(maxVal, mean+width); //not really needed, given while condition
	if (verbose) {
	  std::cout<<"Fitting for peak " << iP << ": around mean=" << mean
			   << " between " << minVal << " and " << max 
			   <<  std::endl;
	}
	TFitResultPtr ptr = hIn->Fit("gaus", "QRS", "same", minVal, max);//mean-width, mean+width);
	//potentially adjust width based on fit result?
	//parameter order: norm, mean, sigma
	if (ptr || ptr->Parameter(1) < oldMean) {//ptr is not 0 --> not converged, try again with narrower range
	  minVal=mean-1.2*width;
	  max=mean+0.8*width;
    if (verbose) {
	  std::cout<<"Fitting again for peak " << iP << ": around mean=" << mean
			   << " between " << minVal << " and " << max
			   <<  std::endl;
	}
	//	  ptr = hIn->Fit("gaus", "RS", "same",  minVal, max);// mean-0.8*width, mean+0.8*width);
	ptr = hIn->Fit(fGaus, "RS", "same",  minVal, max);// mean-0.8*width, mean+0.8*width);
	}
	if (ptr->Parameter(1) < oldMean and iP > 2) //we got some hits already, this is getting wonky, interrupt
	  break;
	if ( oldMean < 0) //need to do something different in the start, where we don't know the 1PE interval between peaks
	  mean+= 2.5*width;
	else {
	  mean+=ptr->Parameter(1)-oldMean;
	  fGaus->DrawCopy("same");
	}
	oldMean=ptr->Parameter(1);
	minVal=TMath::Max(minVal, mean-width);
	norm+=ptr->Parameter(0);//ptr->Parameter(0); //TMath::Sqrt(ptr->Parameter(2)*TMath::Pi())*ptr->Parameter(0); //keep track of how much stats we have left to work with
    if (verbose) {
	  std::cout<<"Updating sum of peak heights to " << norm 
			   <<  std::endl;
	}
	if (hIn->GetEntries()-norm < 0.05*hIn->GetEntries()) {
	  std::cout<<"Hitting stats break factor at peak integral " << norm 
             << " and histogram entries " << hIn->GetEntries()
             <<  std::endl;
	  break;
	}
	// should probably assess fit quality somewhere here too 
	vPtrs.push_back( ptr );
    if (verbose) {
	  std::cout<<"For peak " << iP << ", got mean=" << ptr->Parameter(1) <<
		"\tand sigma=" << ptr->Parameter(2) <<  std::endl;
	  std::cout<<"Updated mean to: " <<  mean << std::endl;
	}
	iP++;
	if ( iP > 10 ) //more than enough, and avoid eternal while loop
	  break;
  }	

    const int nPoints = vPtrs.size();
    double x[(const int) nPoints];
    double ex[(const int) nPoints];
    double y[(const int) nPoints];
    double ey[(const int) nPoints];
    for ( int iP=0; iP<nPoints; iP++) {
      x[iP] = iP;
      ex[iP] = 0;
      y[iP] = vPtrs.at(iP)->Parameter(1);;
      ey[iP] = vPtrs.at(iP)->Parameter(2);;
    }

    TGraphErrors * g= new TGraphErrors( nPoints, x, y, ex, ey);

	return g;
}
