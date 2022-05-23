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


TGraphErrors * isolateAndFitPeaks( TH1F * hIn, float width, bool verbose, bool isSim);
TGraphErrors * findAndFitPeaks( TH1F * hIn, float width, bool verbose, bool isSim);


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

void extractPedsAndGains(string inFile, int deadChannel=-1, string decodePassName = "conv", int nSamples = 30, int verbosity=2, bool isSim=false, bool doClean=false)
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

  string cleanStr="";
  if (doClean)
	cleanStr=Form("(%s_%s.flag_ == 0 || %s_%s.flag_ == 4) &&", digiName.c_str(),decodePassName.c_str(), digiName.c_str(),decodePassName.c_str());
  
  for (int iC = 0 ; iC < nChannels ; iC++)
	cuts.push_back( Form("%s %s_%s.chanID_ == %i", cleanStr.c_str(), digiName.c_str(),decodePassName.c_str(), iC));

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
	std::cout<< "----> Fitting gain for channel " << iH << std::endl;
	//	TGraphErrors * g= findAndFitPeaks( v_hOut.at(iH), fitRangeWidth, verbosePrint, isSim); 
	TGraphErrors * g= isolateAndFitPeaks( v_hOut.at(iH), fitRangeWidth, verbosePrint, isSim); 
	g->Draw("ap");
	g->SetTitle(";Peak nb;Total charge [fC]");
	g->SetName(Form("g_gainChan%i", iH));
	TFitResultPtr fResult = g->Fit("pol1", "QS");
	std::cout << "For channel " << iH << ", got slope \t" << fResult->Parameter(1) << " fC/PE and intercept " << fResult->Parameter(0) << std::endl;
	std::cout << "Pedestal from intercept would be " << fResult->Parameter(0)/nSamples << std::endl;
	gGains->SetPoint( iH, iH, fResult->Parameter(1)*scaleFac);
	gGains->SetPointError( iH, 0, fResult->ParError(1)*scaleFac);
	/*
	double x, ped;
	g->GetPoint(0, x, ped);
	*/
	float ped=fResult->Parameter(0);
	ped/=nSamples;
	float pedErr=fResult->ParError(0)/nSamples;//g->GetErrorY( 0 );
	gPeds->SetPoint( iH, iH, ped ); 
	gPeds->SetPointError( iH, 0, pedErr); 
	std::cout << "Setting gain " << fResult->Parameter(1)*scaleFac << " and pedestal from intercept " << ped << " fC." << std::endl;

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


TGraphErrors* isolateAndFitPeaks( TH1F * hIn, float width, bool verbose, bool isSim )
{
  float mean = hIn->GetBinCenter( hIn->GetMaximumBin() );
//make a clone where we can iteratively remove what is to the left of peak of interest 
  TH1F * hToFit = (TH1F*)hIn->Clone();
  float oldMean = hIn->GetXaxis()->GetXmin()+5;
  float maxVal = hIn->GetXaxis()->GetXmax();
  float minVal = oldMean;
  minVal = TMath::Max(mean-width,minVal);
  float max=TMath::Min(maxVal, mean+width); //start a bit more narrow, we don't know distance between peaks yet                                              
  TF1 * fBkg = new TF1("fBkg", background, -50, 5000,3);
  hIn->Fit(fBkg, "RS", "", mean+10, maxVal); // get the background fit by fitting from first peak to end
  std::cout << "Starting with fit interval (" << minVal << ", " << max << ")" << std::endl;
  TF1 * fGaus = new TF1("fGaus", "gaus", hIn->GetXaxis()->GetXmin(), hIn->GetXaxis()->GetXmax());
  //  TF1 * fGaus = new TF1("fGaus", lorentzianPeak, hIn->GetXaxis()->GetXmin(), hIn->GetXaxis()->GetXmax(), 3);
  //TF1 * fGaus = new TF1("fGaus", asymPeak, hIn->GetXaxis()->GetXmin(), hIn->GetXaxis()->GetXmax(), 3);
  vector<TFitResultPtr> vPtrs;
  int iPstart = 0;
  if (isSim)
    iPstart=1; //no real pedestal peak in MC
  
  int iP=iPstart;
  int norm = 0;
  float sigma=0;                                                                                                        
  bool hasAdjustedWidth=false;

  while ( max <= maxVal ) {
    if (verbose) {
      std::cout<<"Fitting for peak " << iP << ": around mean=" << mean
               << " between " << minVal << " and " << max
               <<  std::endl;
    }
    TFitResultPtr ptr = hToFit->Fit(fGaus, "QRS", "same", minVal, max);//mean-width, mean+width);     
	if (ptr || ptr->Parameter(1) < oldMean || ptr->Parameter(2) < 0.2*sigma) {//ptr is not 0 --> not converged, try again with narrower range, given that we probably nailed the peak already                                                    
      minVal=mean-0.4*width;
      max=mean+0.4*width;
      if (verbose) {
        std::cout<<"\tFitting again for peak " << iP << ": around mean=" << mean
                 << " between " << minVal << " and " << max
                 <<  std::endl;
      }
      ptr = hToFit->Fit(fGaus, "RS", "same",  minVal, max);// mean-0.8*width, mean+0.8*width);                                                                    
    }
   
   if (iP > iPstart && !hasAdjustedWidth) {
	 width=(ptr->Parameter(1)-oldMean)/2.; //total width is half distance between the two peaks                                                             
	 hasAdjustedWidth=true;
	 if (verbose) {
	   std::cout<<"\tUpdating width to " << width
				<<  std::endl;
	 }
   }

   mean=ptr->Parameter(1);
   sigma=ptr->Parameter(2);
   fGaus->DrawCopy("same");
   //could keep track of last bin which was reset, for some speed gain
   for (int iB=1; iB<hToFit->FindBin( mean+TMath::Min(width,float(7*sigma)) ); iB++)
	 hToFit->SetBinContent(iB, 0);
      
   oldMean=mean;
   mean=hToFit->GetBinCenter( hToFit->GetMaximumBin() );

   minVal=TMath::Max(minVal, float(mean-1.25*width));
   max=mean+1.25*width;     								
   norm+=ptr->Parameter(0);   //keep track of how much stats we have left to work with
   if (verbose) {
	 std::cout<<"\tUpdating sum of peak heights to " << norm
			  <<  std::endl;
   }
   // should probably assess fit quality somewhere here too                                                                                                   
   vPtrs.push_back( ptr );
   if (verbose) {
	 std::cout<<"\t\tFor peak " << iP << ", got mean=" << ptr->Parameter(1) <<
	   "\tand sigma=" << ptr->Parameter(2) <<  std::endl;
	 std::cout<<"\t\tUpdated mean to: " <<  mean << std::endl;
   }
   iP++;
   if ( iP > 10 ) //more than enough, and avoid eternal while loop                                                                                            
	 break;
   float bkgLevel=0;
   if (!isSim)
	 bkgLevel=fBkg->Eval(ptr->Parameter(1));
   //   if (hIn->GetEntries()-norm < 0.05*hIn->GetEntries() || ptr->Parameter(0)-bkgLevel< 8) { //don't fit peaks with just a few entries                          
   if (hToFit->Integral() < 0.01*hIn->Integral() || ptr->Parameter(0)-bkgLevel< 8 || hToFit->Integral() < 10) { //don't fit peaks with just a few entries                          
	 std::cout<<"\tHitting stats break factor at peak integral " << ptr->Parameter(0)
			  << " and (in data) fitted background level " << bkgLevel
			  << " and histogram entries " << hToFit->Integral()
			  << " out of total from start " << hIn->Integral()
			  <<  std::endl;
	 break;
   }
   
  } 
  const int nPoints = vPtrs.size();
  double x[(const int) nPoints];
  double ex[(const int) nPoints];
  double y[(const int) nPoints];
  double ey[(const int) nPoints];
  for ( int iP=0; iP<nPoints; iP++) {
	x[iP] = iPstart+iP;
	ex[iP] = 0;
	y[iP] = vPtrs.at(iP)->Parameter(1);;
	ey[iP] = vPtrs.at(iP)->Parameter(2);;
  }
  
  TGraphErrors * g= new TGraphErrors( nPoints, x, y, ex, ey);
  
  return g;
  
}


TGraphErrors* findAndFitPeaks( TH1F * hIn, float width, bool verbose, bool isSim )
{

  float mean = hIn->GetBinCenter( hIn->GetMaximumBin() );
  float maxVal = hIn->GetXaxis()->GetXmax();
  std::cout << "Limit on maxVal = " << maxVal << std::endl;
  float oldMean = hIn->GetXaxis()->GetXmin()+5;
  float minVal = oldMean;
  minVal = TMath::Max(mean-width,minVal);
	float max=TMath::Min(maxVal, mean+width); //start a bit more narrow, we don't know distance between peaks yet
  std::cout << "Starting with fit interval (" << minVal << ", " << max << ")" << std::endl;
  vector<TFitResultPtr> vPtrs;
  int iPstart = 0;
  if (isSim)
	iPstart=1; //no real pedestal peak in MC 
  int iP=iPstart;
  int norm = 0;
  TF1 * fGaus = new TF1("fGaus", "gaus", -50, 5000);
  TF1 * fBkg = new TF1("fBkg", background, -50, 5000,3);
  hIn->Fit(fBkg, "RS", "", mean+10, maxVal); // get the background fit by fitting from first peak to end
  //
  /*
  TF1 * fSum= new TF1("fSum", "fBkg+gaus(3)", mean, 5000);
  //*/
  TF1 *fSum = new TF1("fSum",fitFunction,mean, 5000, 6);
  fSum->FixParameter(3, 0);//fBkg->GetParameter(0));
  fSum->FixParameter(4, 0);//fBkg->GetParameter(1));
  fSum->FixParameter(5, 0);//fBkg->GetParameter(2));
  bool hasAdjustedWidth=false;
  while ( max <= maxVal ) {
	if (verbose) {
	  std::cout<<"Fitting for peak " << iP << ": around mean=" << mean
			   << " between " << minVal << " and " << max 
			   <<  std::endl;
	}
	TFitResultPtr ptr = hIn->Fit(fGaus, "QRS", "same", minVal, max);//mean-width, mean+width);
	//potentially adjust width based on fit result?
	//parameter order: norm, mean, sigma
	if (ptr || ptr->Parameter(1) < oldMean) {//ptr is not 0 --> not converged, try again with narrower range
	  minVal=mean-1.2*width;
	  max=mean+0.8*width;
	  if (verbose) {
		std::cout<<"\tFitting again for peak " << iP << ": around mean=" << mean
				 << " between " << minVal << " and " << max
				 <<  std::endl;
	  }
	  ptr = hIn->Fit(fGaus, "RS", "same",  minVal, max);// mean-0.8*width, mean+0.8*width);
	}
	mean=ptr->Parameter(1);
	if (ptr->Parameter(1) < oldMean+width and iP > 2) //we got some hits already, mean is decreasing so this is getting wonky, interrupt
	  break;
	if ( oldMean < 0) //need to do something different in the start, where we don't know the 1PE interval between peaks
	  mean+= 1.5*width; //width is not yet adjusted based on peak distance 
	else {
	  if (verbose) {
		std::cout<<"\tUpdated next mean estimate from fit result " << mean << " to: " <<  mean+ptr->Parameter(1)-oldMean << " using old mean " << oldMean<< std::endl;
	  }
	  mean+=ptr->Parameter(1)-oldMean;
	  if (!hasAdjustedWidth) {
		width=(ptr->Parameter(1)-oldMean)/4.; //total width is half distance between the two peaks 
		hasAdjustedWidth=true;
		if (verbose) {
		  std::cout<<"\tUpdating width to " << width
				   <<  std::endl;
		}
	  }
	  fGaus->DrawCopy("same");
	}
	oldMean=ptr->Parameter(1);
	minVal=TMath::Max(minVal, float(mean-1.25*width));
	max=TMath::Min(maxVal, float(mean+1.25*width)); 

	norm+=ptr->Parameter(0);//ptr->Parameter(0); //TMath::Sqrt(ptr->Parameter(2)*TMath::Pi())*ptr->Parameter(0); //keep track of how much stats we have left to work with
    if (verbose) {
	  std::cout<<"\tUpdating sum of peak heights to " << norm 
			   <<  std::endl;
	}
	// should probably assess fit quality somewhere here too 
	vPtrs.push_back( ptr );
    if (verbose) {
	  std::cout<<"\t\tFor peak " << iP << ", got mean=" << ptr->Parameter(1) <<
		"\tand sigma=" << ptr->Parameter(2) <<  std::endl;
	  std::cout<<"\t\tUpdated mean to: " <<  mean << std::endl;
	}
	iP++;
	if ( iP > 10 ) //more than enough, and avoid eternal while loop
	  break;
	float bkgLevel=0;
	if (!isSim)
	  bkgLevel=fBkg->Eval(ptr->Parameter(1));  
	if (hIn->GetEntries()-norm < 0.05*hIn->GetEntries() || ptr->Parameter(0)-bkgLevel< 8) { //don't fit peaks with just a few entries 
	  std::cout<<"\tHitting stats break factor at peak integral " << ptr->Parameter(0)
			   << " and (in data) fitted background level " << bkgLevel
             << " and histogram entries " << hIn->GetEntries()
             <<  std::endl;
	  break;
	}
  }	

    const int nPoints = vPtrs.size();
    double x[(const int) nPoints];
    double ex[(const int) nPoints];
    double y[(const int) nPoints];
    double ey[(const int) nPoints];
    for ( int iP=0; iP<nPoints; iP++) {
      x[iP] = iPstart+iP;
      ex[iP] = 0;
      y[iP] = vPtrs.at(iP)->Parameter(1);;
      ey[iP] = vPtrs.at(iP)->Parameter(2);;
    }

    TGraphErrors * g= new TGraphErrors( nPoints, x, y, ex, ey);

	return g;
}
