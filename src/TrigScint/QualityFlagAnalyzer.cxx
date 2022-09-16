/**
 * @file QualityFlagAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/QualityFlagAnalyzer.h"

namespace trigscint {

  QualityFlagAnalyzer::QualityFlagAnalyzer(const std::string& name,
						   framework::Process& process)
    : Analyzer(name, process) {}
  QualityFlagAnalyzer::~QualityFlagAnalyzer() {}
  
  void QualityFlagAnalyzer::configure(framework::config::Parameters &parameters){

    inputEventCol_  = parameters.getParameter< std::string >("inputEventCollection");
    inputEventPassName_  = parameters.getParameter< std::string >("inputEventPassName");
    inputHitCol_  = parameters.getParameter< std::string >("inputHitCollection");
    inputHitPassName_  = parameters.getParameter< std::string >("inputHitPassName");
    peds_  = parameters.getParameter< std::vector<double> >("pedestals");
    gain_  = parameters.getParameter< std::vector<double> >("gain");
    startSample_  = parameters.getParameter< int >("startSample");

    std::cout << " [ QualityFlagAnalyzer ] In configure(), got parameters " 
	      << "\n\t inputEventCollection = " << inputEventCol_
	      << "\n\t inputEventPassName = " << inputEventPassName_
	      << "\n\t inputHitCollection = " << inputHitCol_
	      << "\n\t inputHitPassName = " << inputHitPassName_
	      << "\n\t startSample = " << startSample_
	      << "\n\t pedestals[0] = " << peds_[0]
	      << "\n\t gain[0] = " << gain_[0]
	      << "\t." << std::endl;

    return;
  }

  void QualityFlagAnalyzer::analyze(const framework::Event &event) {

    const auto channels{event.getCollection<trigscint::EventReadout >(inputEventCol_, inputEventPassName_)};
    const auto hits{event.getCollection<trigscint::TestBeamHit >(inputHitCol_, inputHitPassName_)};

	int evNb = event.getEventNumber();
	//	while (evNb < 0 ) {
	//  ldmx_log(debug) << "event number = " << evNb << " < 0; incrementing event number ";
	//  evNb++;
	//}
	int nChan = channels.size();
	ldmx_log(debug) << "in event " << evNb << "; nChannels = " << nChan;

	bool existsIntermediatePE = false;
	float hitPEs[nChannels] = {0.}; 
	
	//ok. get each channel, and find the associated hit, using bar nb. 

	for (auto chan : channels) {
	  std::vector<float> q = chan.getQ();
	  std::vector<float> qErr = chan.getQError();
	  std::vector<int> tdc = chan.getTDC();
	  int nTimeSamp = q.size();
	  int bar = chan.getChanID();

	  int flag = chan.getQualityFlag();
	  
	   //check if this is messing up flag 
	  for (auto hit : hits) { //we will be ok even if there is no match
		if (hit.getBarID() == bar ) { //
		  flag = hit.getQualityFlag();
		  hitPEs[bar]= hit.getPE();
		  if ( flag==0 && bar < 12 && 15 < hit.getPE() && hit.getPE() < 40)
			existsIntermediatePE=true;
		}
	  }

	  ldmx_log(debug) << "Got event flag " << flag;

	  //if flag = 0, fill for clean versions of the usual event displays
	  if ( flag==0 && evNb < nEv && bar < nChannels ) { //stick within the predefined histogram array
		for (int iT = 0; iT < q.size() ; iT++) {
		  ldmx_log(debug) << "in event " << evNb << "; channel " << bar << ", got charge[" << iT << "] = " << q.at(iT);
		  hOut[evNb][bar]->SetBinContent(iT+startSample_, q.at(iT));
		  hOut[evNb][bar]->SetBinError(iT+startSample_, fabs(qErr.at(iT)) );
		}//if within the number of events to plot individually
	  }//over time samples

	  // now select on flags
	  //special case: flag = 0 (this will happen to all eventually, so catch it now
	  if (flag == 0 && nEvDrawn[nFlags-1] < nEv ) { //then draw if we haven't collected enough                      
		int fillNb=nEvDrawn[nFlags-1];
		for (int iT = 0; iT < q.size() ; iT++) { //fill this plot with all q
			hOutFlag[nFlags-1][fillNb][bar]->SetBinContent(iT+startSample_, q.at(iT));
            hOutFlag[nFlags-1][fillNb][bar]->SetBinError(iT+startSample_, fabs(qErr.at(iT)) );
          }
		  //keep track of actual event number 
		  hOutFlag[nFlags-1][fillNb][bar]->GetYaxis()->SetTitle(Form("Q, flag 0, chan %i, ev %i [fC]", bar,evNb));
		  nEvDrawn[nFlags-1]++;  //update filled event counter for this flag (0)
	  }//if nothing was flagged 
	  else { //hit was flagged somehow
		for (int iF=0;iF<nFlags-1;iF++) { //do all but the last
		  int fillNb=nEvDrawn[iF];
		  ldmx_log(debug) << "Checking flag " << flags[iF];
		  //we're starting from the high numbers and iteratively subtracting
		  if (flag >= flags[iF] ) {
			ldmx_log(debug) << "Checking flag " << flags[iF];
			if (fillNb < nEv ) { //then 1. this flag must be raised 2. draw if we haven't collected enough
			  for (int iT = 0; iT < q.size() ; iT++) { //fill this plot with all q
				hOutFlag[iF][fillNb][bar]->SetBinContent(iT+startSample_, q.at(iT));
				hOutFlag[iF][fillNb][bar]->SetBinError(iT+startSample_, fabs(qErr.at(iT)) );
			  }//over time samples 
			  hOutFlag[iF][fillNb][bar]->GetYaxis()->SetTitle(Form("Q, flag %i, chan %i, ev %i [fC]", flags[iF],bar,evNb));
			nEvDrawn[iF]++;  //update filled event counter for this flag
			}
			flag-=flags[iF]; //subtract that flag from sum
		  }//if this flag 
		}//over flags
	  }//if any non-zero flag
	}//over channels	
		
	
	// select 15 < PE < 40 events 	
	if ( existsIntermediatePE && peFillNb < nEv ) { //then 1. this flag must be raised 2. draw if we haven't collected enough
	  ldmx_log(debug) << "Got at least one intermediate PE channel";
	  for (auto chan : channels) {
		std::vector<float> q = chan.getQ();
		std::vector<float> qErr = chan.getQError();
		std::vector<int> tdc = chan.getTDC();
		int nTimeSamp = q.size();
		int bar = chan.getChanID();
		for (int iT = 0; iT < q.size() ; iT++) { //fill this plot with all q
		  hOutPE[peFillNb][bar]->SetBinContent(iT+startSample_, q.at(iT));
		  hOutPE[peFillNb][bar]->SetBinError(iT+startSample_, fabs(qErr.at(iT)) );
		}//over time samples 
		hOutPE[peFillNb][bar]->GetYaxis()->SetTitle(Form("Q, chan %i, ev %i, PE %.2f", bar,evNb,hitPEs[bar]));
	  }//over channels	
	  peFillNb++;  //update filled event counter for this flag
	} //if fill 
	
	
    return;
  }

  void QualityFlagAnalyzer::onFileOpen() {
    std::cout << "\n\n File is opening! My analyzer should do something -- like print this \n\n" << std::endl;

    return;
  }

  void QualityFlagAnalyzer::onFileClose() {

    return;
  }
  
  void QualityFlagAnalyzer::onProcessStart() {
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
	}

	for (int iE = 0; iE<nEv; iE++) {
	  for (int iB = 0; iB<nChannels; iB++) { 
		hOut[iE][iB] = new TH1F(Form("hCharge_chan%i_ev%i", iB, iE), Form(";time sample; Q, channel %i, event %i [fC]", iB, iE), nTimeSamp,-0.5,nTimeSamp-0.5);
		hOutPE[iE][iB] = new TH1F(Form("hCharge_PEcut_chan%i_nb%i", iB, iE), Form(";time sample; Q, channel %i, event %i [fC]", iB, iE), nTimeSamp,-0.5,nTimeSamp-0.5);
		for (int iF=0; iF<nFlags; iF++) { 
		  hOutFlag[iF][iE][iB] = new TH1F(Form("hCharge_flag%i_chan%i_nb%i", flags[iF],iB, iE), Form(";time sample; Q, flag %i, chan %i, ev %i [fC]", iF, iB, iE), nTimeSamp,-0.5,nTimeSamp-0.5); //less confusing to name them upon actual use 
		  //hOutFlag[iF][iE][iB] = new TH1F("hCharge_flag",  "", nTimeSamp,-0.5,nTimeSamp-0.5);
		}
	  }
	}


	hTDCfireChanvsEvent = new TH2F("hTDCfireChanvsEvent", ";channel with TDC < 63;event number", nChannels,-0.5,nChannels-0.5, nEv, 0, nEv);
	
	evNb = 0;
	peFillNb=0;
	ldmx_log(debug) << "done setting up histograms" ;

    return;
  }
  

  void QualityFlagAnalyzer::onProcessEnd() {

    return;
  }


}

DECLARE_ANALYZER_NS(trigscint, QualityFlagAnalyzer)
