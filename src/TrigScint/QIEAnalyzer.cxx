/**
 * @file QIEAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/QIEAnalyzer.h"

namespace trigscint {

  QIEAnalyzer::QIEAnalyzer(const std::string& name,
						   framework::Process& process)
    : Analyzer(name, process) {}
  QIEAnalyzer::~QIEAnalyzer() {}
  
  void QIEAnalyzer::configure(framework::config::Parameters &parameters){

    inputCol_  = parameters.getParameter< std::string >("inputCollection");
    inputPassName_  = parameters.getParameter< std::string >("inputPassName");
    peds_  = parameters.getParameter< std::vector<double> >("pedestals");
    gain_  = parameters.getParameter< std::vector<double> >("gain");
    startSample_  = parameters.getParameter< int >("startSample");

    std::cout << " [ QIEAnalyzer ] In configure(), got parameters " 
	      << "\n\t inputCollection = " << inputCol_
	      << "\n\t inputPassName = " << inputPassName_
	      << "\n\t startSample = " << startSample_
	      << "\n\t pedestals[0] = " << peds_[0]
	      << "\n\t gain[0] = " << gain_[0]
	      << "\t." << std::endl;

    return;
  }

  void QIEAnalyzer::analyze(const framework::Event &event) {

    const auto channels{event.getCollection<trigscint::EventReadout >(inputCol_, inputPassName_)};

	int evNb = event.getEventNumber();
	//	while (evNb < 0 ) {
	//  ldmx_log(debug) << "event number = " << evNb << " < 0; incrementing event number ";
	//  evNb++;
	//}
	int nChan = channels.size();
	ldmx_log(debug) << "in event " << evNb << "; nChannels = " << nChan;
	
	for (auto chan : channels) {
	  std::vector<float> q = chan.getQ();
	  std::vector<float> qErr = chan.getQError();
	  std::vector<int> tdc = chan.getTDC();
	  int nTimeSamp = q.size();
	  int bar = chan.getChanID();
	  float qTot = 0;
	  float qPedSubtractedAvg = 0;
	  int firstT = startSample_-1;
	  int nSampAbove = 0;
	  int nSampAboveEventPed = 0;
	  float subtrPE = 0;
	  float subtrQ = 0;
	  float ped = chan.getPedestal() ;
	  for (int iT = 0; iT < q.size() ; iT++) {
		ldmx_log(debug) << "in event " << evNb << "; channel " << bar << ", got charge[" << iT << "] = " << q.at(iT);
		if ( evNb < nEv && bar < nChannels ) { //stick within the predefined histogram array
		  //		  hOut[evNb][bar]->Fill(iT+startSample_, q.at(iT));
		  hOut[evNb][bar]->SetBinContent(iT+startSample_, q.at(iT));
		  hOut[evNb][bar]->SetBinError(iT+startSample_, fabs(qErr.at(iT)) );
		  if ( tdc.at(iT) < 63 ) {
			ldmx_log(info) << "Found fired TDC = " << tdc.at(iT) << " at time sample " << iT << " in channel " << bar << " and event " << evNb ;
			hOut[evNb][bar]->SetLineColor(kRed+1); //for some reason, the style settings are washed out later...
			hOut[evNb][bar]->SetMarkerColor(hOut[evNb][bar]->GetLineColor());
			hOut[evNb][bar]->SetMarkerSize(0.2);

			if (iT+startSample_  > 0)
			  hTDCfireChanvsEvent->Fill( bar, evNb, iT+startSample_);
			else 
			  hTDCfireChanvsEvent->Fill( bar, evNb);
		  }
		}//if within the number of events to plot individually 
		if ( q.at(iT) > 2*fabs(peds_[ bar ]) ) 	{ //integrate all charge well above ped to convert to a PE count
		  qTot+=q.at(iT);
		  qPedSubtractedAvg+=q.at(iT)-chan.getPedestal(); //peds_[ bar ];
		  nSampAbove++;
		  ldmx_log(debug) << " above channel overall pedestal: " << q.at(iT) << " > " << 2*fabs(peds_[bar]) ;
   
		  if (firstT = startSample_-1) //keep track of first time sample above threshold
			firstT=startSample_+iT;
		}//if above threshold
		if (q.at(iT) > ped ) { 
		  subtrQ+=q.at(iT)-peds_[bar] ; 
		  nSampAboveEventPed++;
		  ldmx_log(debug) << " above channel event pedestal: " << q.at(iT) << " > " << ped;
		}//if above channel event pedestal
	  }//over time samples
	  float PE = qTot*6250./gain_[bar];
	  subtrPE=subtrQ*6250./gain_[bar];
	  hTotQvsPed[ bar ]->Fill( ped, qTot );
	  hPE[ bar ]->Fill( PE );
	  hPEvsT[ bar ]->Fill( firstT, PE );
	  if (nSampAbove > 0) {
		qPedSubtractedAvg/=nSampAbove;
		hPedSubtractedAvgQvsT[ bar ]->Fill( firstT, qPedSubtractedAvg );
		hAvgQvsT[ bar ]->Fill( firstT, qTot/nSampAbove );
	  }
	  //if (chan.getPedestal() < 40. ) {
	  //subtrQ = subtrPE/(6250./4.e6); //undo conversion
	  ldmx_log(debug) << "filling qTot histograms" ;
	  hPedSubtractedTotQvsPed[ bar ]->Fill( ped, subtrQ );
	  if (ped < 40 ) //avoid case where we have saturation and a plateau as much as possible
		hPedSubtractedTotQvsN[ bar ]->Fill( nSampAboveEventPed, subtrQ );
	  hPedSubtractedPEvsN[ bar ]->Fill(nSampAboveEventPed, subtrPE );
	  hPedSubtractedPEvsT[ bar ]->Fill(firstT, subtrPE );
	  ldmx_log(debug) << " done filling qTot histograms" ;
		// }
	}//over channels



	
	
    return;
  }

  void QIEAnalyzer::onFileOpen() {
    std::cout << "\n\n File is opening! My analyzer should do something -- like print this \n\n" << std::endl;

    return;
  }

  void QIEAnalyzer::onFileClose() {

    return;
  }
  
  void QIEAnalyzer::onProcessStart() {
    std::cout << "\n\n Process starts! My analyzer should do something -- like print this \n\n" << std::endl;
    getHistoDirectory();
	
	int nTimeSamp=40;
	int PEmax=100;
	int nPEbins=5*PEmax;
	float Qmax=PEmax/(6250./4.e6);
	float Qmin=-10;
	int nQbins=(Qmax-Qmin)/4;

	ldmx_log(debug) << "Setting up histograms... " ;

	for (int iB = 0; iB<nChannels; iB++) {
	  hPE[iB]=new TH1F(Form("hPE_chan%i", iB), Form(";PE, chan%i", iB),nPEbins,0,PEmax);
	  hPEvsT[iB]=new TH2F(Form("hPEvsT_chan%i", iB), Form(";First time sample above summing threshold;PE, chan%i", iB),nTimeSamp+1,-1.5,nTimeSamp-0.5, nPEbins,0,PEmax);
	  hPedSubtractedAvgQvsT[iB]=new TH2F(Form("hPedSubtrAvgQvsT_chan%i", iB), Form(";First time sample above threshold;Pedestal subtracted average Q, chan%i [fC]", iB),nTimeSamp+1,-1.5,nTimeSamp-0.5, nQbins/10,Qmin,Qmax/10.);
	  hPedSubtractedTotQvsPed[iB]=new TH2F(Form("hPedSubtrTotQvsPed_chan%i", iB), Form(";Channel event pedestal [fC];Event pedestal subtracted total Q, chan%i [fC]", iB), 1010,Qmin,1000, 10010, -10, 10000); //nQbins/2,Qmin,Qmax/5., nQbins,Qmin,2*Qmax);
	  	  hPedSubtractedTotQvsN[iB]=new TH2F(Form("hPedSubtrTotQvsN_chan%i", iB), Form(";Number of time samples added; Event pedestal subtracted total Q, chan%i [fC]", iB), nTimeSamp+1,-1.5,nTimeSamp-0.5, 10010, -10, 10000);
	  hTotQvsPed[iB]=new TH2F(Form("hTotQvsPed_chan%i", iB), Form(";Channel event pedestal [fC];Event total Q, chan%i [fC]", iB), 1010,Qmin,1000, 10010, -10, 10000); //nQbins/2,Qmin,Qmax/5., nQbins,Qmin,2*Qmax);
	  hPedSubtractedPEvsN[iB]=new TH2F(Form("hPedSubtrPEvsN_chan%i", iB), Form(";Number of time samples above threshold;Pedestal subtracted PE, chan%i [fC]", iB),nTimeSamp+1,-1.5,nTimeSamp-0.5, nPEbins,0,PEmax);
	  hPedSubtractedPEvsT[iB]=new TH2F(Form("hPedSubtrPEvsT_chan%i", iB), Form(";First time sample above threshold;Pedestal subtracted PE, chan%i [fC]", iB),nTimeSamp+1,-1.5,nTimeSamp-0.5, nPEbins,0,PEmax);
	  hAvgQvsT[iB]=new TH2F(Form("hAvgQvsT_chan%i", iB), Form(";First time sample above threshold;Average Q, chan%i [fC]", iB),nTimeSamp+1,-1.5,nTimeSamp-0.5, nQbins/10,Qmin,Qmax/10);
	}

	for (int iE = 0; iE<nEv; iE++) {
	  for (int iB = 0; iB<nChannels; iB++) { 
		hOut[iE][iB] = new TH1F(Form("hCharge_chan%i_ev%i", iB, iE), Form(";time sample; Q, channel %i, event %i [fC]", iB, iE), nTimeSamp,-0.5,nTimeSamp-0.5);
	  }
	}


	hTDCfireChanvsEvent = new TH2F("hTDCfireChanvsEvent", ";channel with TDC < 63;event number", nChannels,-0.5,nChannels-0.5, nEv, 0, nEv);
	
	evNb = 0;

	ldmx_log(debug) << "done setting up histograms" ;

    return;
  }
  

  void QIEAnalyzer::onProcessEnd() {

    return;
  }


}

DECLARE_ANALYZER_NS(trigscint, QIEAnalyzer)
