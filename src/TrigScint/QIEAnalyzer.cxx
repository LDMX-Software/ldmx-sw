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

    std::cout << " [ QIEAnalyzer ] In configure(), got parameters " 
	      << "\n\t inputCollection = " << inputCol_
	      << "\n\t inputPassName = " << inputPassName_
	      << "\n\t pedestals[0] = " << peds_[0]
	      << "\t." << std::endl;

    return;
  }

  void QIEAnalyzer::analyze(const framework::Event &event) {

    const auto channels{event.getCollection<trigscint::EventReadout >(inputCol_, inputPassName_)};

	int evNb = event.getEventNumber();
	int nChan = channels.size();

	for (auto chan : channels) {
	  std::vector<float> q = chan.getQ();
	  int nTimeSamp = q.size();
	  int bar = chan.getChanID();
	  float qTot = 0;
	  int firstT = -1;
	  for (int iT = 0; iT < q.size() ; iT++) {
		ldmx_log(debug) << "in event " << evNb << "; channel " << bar << ", got charge[" << iT << "] = " << q.at(iT);
		if ( evNb < nEv && bar < nChannels ) //stick within the predefined histogram array
		  hOut[evNb][bar]->Fill(iT, q.at(iT));
		if ( q.at(iT) > 2*fabs(peds_[ bar ]) ) 	{ //integrate all charge well above ped to convert to a PE count
		  qTot+=q.at(iT);
		  if (firstT =-1) //keep track of first time sample above threshold
			firstT=iT;
		}//if above threshold
	  }//over time samples
	  float PE = qTot*6250./4.e6;
	  hPE[ bar ]->Fill( PE );
	  hPEvsT[ bar ]->Fill( firstT, PE );

	}//over channels



	
	
	//vChargeVsTime.push_back( vH );
    return;
  }

  // /*
  void QIEAnalyzer::onFileOpen() {
    std::cout << "\n\n File is opening! My analyzer should do something -- like print this \n\n" << std::endl;

    return;
  }

  void QIEAnalyzer::onFileClose() {

    return;
  }
  //  */
  
  void QIEAnalyzer::onProcessStart() {
    std::cout << "\n\n Process starts! My analyzer should do something -- like print this \n\n" << std::endl;

    getHistoDirectory();

    /*
	  int yMax = 50;
    int yMin = -yMax;
    int nBinsY = (yMax-yMin)/1; //1 mm resolution
    //    
    hSimYEvnb = new TH2F("hSimYEvnb","Beam electron y for each event;Event number;y",nEv,0,nEv, nBinsY,yMin,yMax);
    hSimIDEvnb = new TH2F("hSimIDEvnb","Beam electron y converted to channel ID, for each event;Event number;ID",nEv,0,nEv, nBinsY,convertToID(yMin),convertToID(yMax));
    hIdEvnb = new TH2F("hIdEvnb","Id track vs event, filled with N_{PE};Event;Channel ID;N_{PE}", nEv,0,nEv, nChannels+2,0,nChannels+2);
    hId=new TH1F("hId","channel id;Channel ID", nChannels,0,nChannels);
    hNtracksEvnb=new TH1F("hNtracksEvnb","N_tracks for each event;Event;N_{tracks}", nEv,0,nEv);
    hNtracks=new TH1F("hNtracks","N_tracks per event;Event;N_{tracks}", nTrkMax,0,nTrkMax);
    hFindableTracks=new TH1F("hFindableTracks","N_findableTracks per event;Event;N_{tracks}^{findable}", nTrkMax,0,nTrkMax);
    hTrackMatrix=new TH2F("hTrackMatrix","Found vs findable tracks per event;N_{tracks}^{findable};N_{tracks}", nTrkMax,-0.5,nTrkMax-0.5, nTrkMax,-0.5,nTrkMax-0.5);
    hNhits=new TH1F("hNhits","Nhits in a track;Track width [channel Nb];N_{tracks}", 60, 0, 6);
*/
	int nTimeSamp=40;
	for (int iE = 0; iE<nEv; iE++) {
	  for (int iB = 0; iB<nChannels; iB++) { 
		//TH1F * hOut = new TH1F(Form("hCharge_chan%i_ev%i", iB, iE), Form(";time sample; Q, channel %i, event %i [fC]", iB, iE), nTimeSamp, -0.5, nTimeSamp-0.5);
		hOut[iE][iB] = new TH1F(Form("hCharge_chan%i_ev%i", iB, iE), Form(";time sample; Q, channel %i, event %i [fC]", iB, iE), nTimeSamp,-0.5,nTimeSamp-0.5);
	  }
	}
	int PEmax=500;
	for (int iB = 0; iB<nChannels; iB++) {
	  hPE[iB]=new TH1F(Form("hPE_chan%i", iB), Form(";PE, chan%i", iB),5*PEmax,0,PEmax);
	  hPEvsT[iB]=new TH2F(Form("hPEvsT_chan%i", iB), Form(";First time sample above summing threshold;PE, chan%i", iB),nTimeSamp+1,-1.5,nTimeSamp-0.5, 5*PEmax,0,PEmax);
	
	}
	evNb = 0;
	
    return;
  }
  

  void QIEAnalyzer::onProcessEnd() {

    return;
  }


}

DECLARE_ANALYZER_NS(trigscint, QIEAnalyzer)
