/**
 * @file TestBeamHitAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/TestBeamHitAnalyzer.h"

namespace trigscint {

  TestBeamHitAnalyzer::TestBeamHitAnalyzer(const std::string& name,
						   framework::Process& process)
    : Analyzer(name, process) {}
  TestBeamHitAnalyzer::~TestBeamHitAnalyzer() {}
  
  void TestBeamHitAnalyzer::configure(framework::config::Parameters &parameters){

    inputCol_  = parameters.getParameter< std::string >("inputCollection");
    inputPassName_  = parameters.getParameter< std::string >("inputPassName");
    peds_  = parameters.getParameter< std::vector<double> >("pedestals");
    startSample_  = parameters.getParameter< int >("startSample");

    std::cout << " [ TestBeamHitAnalyzer ] In configure(), got parameters " 
	      << "\n\t inputCollection = " << inputCol_
	      << "\n\t inputPassName = " << inputPassName_
	      << "\n\t startSample = " << startSample_
	      << "\n\t pedestals[0] = " << peds_[0]
	      << "\t." << std::endl;

    return;
  }

  void TestBeamHitAnalyzer::analyze(const framework::Event &event) {

    const auto channels{event.getCollection<trigscint::TestBeamHit >(inputCol_, inputPassName_)};

	int evNb = event.getEventNumber();
	int nChan = channels.size();
	int leadBar=-1;
	int subleadBar=-1;
	float peLead=-1;
	float peSublead=-1;
	bool existsIntermediatePE=false;
	
	for (auto chan : channels) {
	  int bar = chan.getBarID();
	  float PE = chan.getPE();
	  if ( evNb < nEv && bar < nChannels ) { //stick within the predefined histogram array
		hEvDisp->Fill( evNb, bar, PE);
	  } // if within event display range
	  if ( PE > peLead ) {
		peSublead=peLead;
		subleadBar=leadBar;
		peLead=PE;
		leadBar=bar;
	  }
	  else if ( PE > peSublead ) { //need a specific check, bars not sorted in PE so leadPE might be found before or after
		peSublead=PE;
        subleadBar=bar;
	  }
	  hPE[bar]->Fill(PE);
	  if ( chan.getQualityFlag() ==0 && bar < 12 && 15 < PE && PE < 40 )
		existsIntermediatePE=true;
	}//over channels

	if ( existsIntermediatePE && fillNb < nEv ) {
	  for (auto chan : channels) {
		int bar = chan.getBarID();
		if (bar < 12 ) { //stick within the predefined histogram array
		  float PE = chan.getPE();
		  hEvDispPE->Fill( fillNb, bar, PE);
		}
	  } // if within event display range
	  fillNb++;
	}//if PE range triggers writing these event displays 
	
	if (subleadBar == -1) {
	  subleadBar = leadBar;
	  peSublead = peLead;
	}
	hPEVsDelta[leadBar]->Fill(leadBar-subleadBar, peLead);
	hDeltaPEVsDelta[leadBar]->Fill(leadBar-subleadBar, peLead-peSublead);

	//	if ( (subleadBar%2) == (leadBar%2) ) // in same layer (or even, hit). skip if we're not seeing a max across layers
	//  return;  -- on the other hand this is evident from the plot: delta is even 
	
	hPEmaxVsDelta->Fill(leadBar-subleadBar, peLead);

	return;
  }

  void TestBeamHitAnalyzer::onFileOpen() {
    std::cout << "\n\n File is opening! My analyzer should do something -- like print this \n\n" << std::endl;

    return;
  }

  void TestBeamHitAnalyzer::onFileClose() {

    return;
  }
  
  void TestBeamHitAnalyzer::onProcessStart() {
    std::cout << "\n\n Process starts! My analyzer should do something -- like print this \n\n" << std::endl;

    getHistoDirectory();

	
	int nTimeSamp=40;
	int PEmax=400;
	int nPEbins=2*PEmax;
	float Qmax=PEmax/(6250./4.e6);
	float Qmin=-10;
	int nQbins=(Qmax-Qmin)/4;
	
	for (int iB = 0; iB<nChannels; iB++) {
	  hPE[iB]=new TH1F(Form("hPE_chan%i", iB), Form(";PE, chan%i", iB),nPEbins,0,PEmax);
	  hPEVsDelta[iB]=new TH2F(Form("hPEVsDelta_chan%i", iB), Form(";#Delta_{barID};PE, chan%i has max PE", iB),nChannels+1,-nChannels/2-0.5,nChannels/2+0.5, nPEbins,0,PEmax);
	  hDeltaPEVsDelta[iB]=new TH2F(Form("hDeltaPEVsDelta_chan%i", iB), Form(";#Delta_{barID};#Delta_PE, chan%i has max PE", iB),nChannels+1,-nChannels/2-0.5,nChannels/2+0.5, nPEbins,0,PEmax);
	}

	//make event displays for events where there are channels with weird event hit PE counts 
	for (int iE = 0; iE<nEv; iE++) {
	  for (int iB = 0; iB<nChannels; iB++) { 
		hOut[iE][iB] = new TH1F(Form("hCharge_chan%i_nv%i", iB, iE), Form(";time sample; Q, chan %i, ev %i [fC]", iB, iE), nTimeSamp,-0.5,nTimeSamp-0.5);
	  }
	}
	
	hPEmaxVsDelta=new TH2F("hPEmaxVsDelta",";#Delta_{barID};PE, max hit",nChannels,-nChannels/2,nChannels/2, nPEbins,0,PEmax);
	hEvDisp = new TH2F(Form("hEvDisp_ev%i", nEv), ";Event number; Bar ID; PE", nEv,0.5,nEv+0.5, nChannels,-0.5,nChannels-0.5);
	hEvDispPE = new TH2F("hEvDispPEcut", ";Event number; Bar ID; PE", nEv,0.5,nEv+0.5, nChannels,-0.5,nChannels-0.5);
  

	fillNb = 0;
	evNb = 0;
	
    return;
  }
  

  void TestBeamHitAnalyzer::onProcessEnd() {

    return;
  }


}

DECLARE_ANALYZER_NS(trigscint, TestBeamHitAnalyzer)
