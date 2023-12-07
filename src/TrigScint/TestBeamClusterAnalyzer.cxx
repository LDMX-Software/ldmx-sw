/**
 * @file TestBeamClusterAnalyzer.cxx
 * @brief An analyzer drawing the most basic quanities of Trigger Scintillator
 * bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/TestBeamClusterAnalyzer.h"

namespace trigscint {

TestBeamClusterAnalyzer::TestBeamClusterAnalyzer(const std::string& name,
                                         framework::Process& process)
    : Analyzer(name, process) {}

void TestBeamClusterAnalyzer::configure(framework::config::Parameters& parameters) {
  inputCol_ = parameters.getParameter<std::string>("inputCollection");
  inputPassName_ = parameters.getParameter<std::string>("inputPassName");
  //  wideInputCol_ = parameters.getParameter<std::string>("3hitInputCollection");
  //wideInputPassName_ = parameters.getParameter<std::string>("3hitInputPassName");

  std::cout << " [ TestBeamClusterAnalyzer ] In configure(), got parameters "
            << "\n\t inputCollection = " << inputCol_
            << "\n\t inputPassName = " << inputPassName_
			<< std::endl;
    //        << "\n\t 3hitInputCollection = " << wideInputCol_
    //        << "\n\t 3hitInputPassName = " << wideInputPassName_

  return;
}

void TestBeamClusterAnalyzer::analyze(const framework::Event& event) {
  if (!event.exists(inputCol_, inputPassName_) ) {
	ldmx_log(info) << "No cluster collection " << inputCol_ << "_" << inputPassName_ << " found. Skipping analysis of event";
    return;
  }
  
  const auto clusters{
      event.getCollection<ldmx::TrigScintCluster>(inputCol_, inputPassName_)};

  int n1hit=0;
  int n2hit=0;
  int n3hit=0;

  int nClusters=clusters.size();
  int idx=0;
  for (auto cluster : clusters) {
    int seed = cluster.getSeed();
    int nHits = cluster.getNHits();
    if ( nHits == 3 )
      n3hit++;
    else if ( nHits == 2 )
      n2hit++;
    else if ( nHits == 1 )
      n1hit++;
	
    float PE = cluster.getPE();

    hPEinClusters[seed]->Fill(PE);

	/* // this requires different implementation. use getHitIDs and use the indices in there
	   // in a loop over hits in the event to extract the PEs 
	   // -- later. 
	for (auto hits : cluster.getConstituents() )
	  hPEinHits[seed]->Fill(PE);
	*/
    // instead of always checking distance between the first two, instead, fill with distance from current to previous
    // this should give us a better idea about if we're dominated by close-by activity in secondaries 
    if (idx > 0) { //look back at the previous cluster when more than one 
      hDeltaCentroids->Fill( fabs(clusters[idx].getCentroid()-clusters[idx-1].getCentroid()) );
      hDeltaVsSeed->Fill(clusters[idx-1].getSeed(), fabs(clusters[idx].getCentroid()-clusters[idx-1].getCentroid()) );
    }
    idx++; // increment afterwards
  }  // over clusters

  /*
    
  if (n2hit)
	hN3N2->Fill((float)n3hit/n2hit);
  if (n1hit) {
	hN3N1->Fill((float)n3hit/n1hit);
	hN2N1->Fill((float)n2hit/n1hit);
  }
  */
  hN3N2->Fill(n2hit,n3hit);
  hN3N1->Fill(n1hit,n3hit);
  hN2N1->Fill(n1hit,n2hit);
  
  hNClusters->Fill(nClusters);
  //todo: get hit collection to fill Nhits later?
  hNHits->Fill(3*n3hit+2*n2hit+n1hit);
	

  return;
}

void TestBeamClusterAnalyzer::onProcessStart() {
  std::cout << "\n\n Process starts! My analyzer should do something -- like "
               "print this \n\n"
            << std::endl;

  getHistoDirectory();

  int PEmax = 600;
  int nPEbins =0.25*PEmax;
  // float Qmax = PEmax / (6250. / 4.e6);
  // float Qmin = -10;
  // int nQbins = (Qmax - Qmin) / 4;

  for (int iB = 0; iB < nChannels; iB++) {
    hPEinHits[iB] = new TH1F(Form("hPE_chan%i", iB), Form(";PE, chan%i", iB), nPEbins,
                       0, PEmax);
    hPEinClusters[iB] = new TH1F(Form("hPEinClusters_chan%i", iB),
                                 Form(";PE, chan%i", iB), nPEbins, 0, PEmax);
  }

  hDeltaVsSeed = new TH2F(
        "hDeltaVsSeed", ";BarID_{seed};#Delta_{centroid}", nChannels + 1,
        - 0.5,nChannels-0.5, 5*nChannels,0,nChannels);

  hDeltaCentroids = new TH1F("hDeltaCentroids",";#Delta_{centroid}",  5*nChannels,
							- 0.5, nChannels-0.5);

  hNHits = new TH1F("hNHits", "Number of hits in the event; N_{hits}; Events", 10, 0, 10);
  hNClusters = new TH1F("hNClusters", "Number of clusters in the event; N_{clusters}; Events", 10, 0, 10);

  /*
  hN3N2 = new TH1F("hN3N2", "Ratio of 3-hit to 2-hit clusters; N_{3-hit}/N_{2-hit}; Events", 10, 0, 4);
  hN3N1 = new TH1F("hN3N1", "Ratio of 3-hit to 1-hit clusters; N_{3-hit}/N_{1-hit}; Events", 10, 0, 4);
  hN2N1 = new TH1F("hN2N1", "Ratio of 2-hit to 1-hit clusters; N_{2-hit}/N_{1-hit}; Events", 10, 0, 4);
  */
  int nCl = 6;
  
    hN3N2 = new TH2F("hN3N2", "Number of 3-hit vs 2-hit clusters; N_{2-hit};N_{3-hit}; Events", nCl,-0.5,nCl-0.5, nCl,-0.5,nCl-0.5);
    hN3N1 = new TH2F("hN3N1", "Number of 3-hit vs 1-hit clusters; N_{1-hit};N_{3-hit}; Events", nCl,-0.5,nCl-0.5, nCl,-0.5,nCl-0.5);
    hN2N1 = new TH2F("hN2N1", "Number of 2-hit vs 1-hit clusters; N_{1-hit};N_{2-hit}; Events", nCl,-0.5,nCl-0.5, nCl,-0.5,nCl-0.5);
  
  return;
}

void TestBeamClusterAnalyzer::onProcessEnd() { return; }

}  // namespace trigscint

DECLARE_ANALYZER_NS(trigscint, TestBeamClusterAnalyzer)
