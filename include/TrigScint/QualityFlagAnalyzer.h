/**
 * @file QualityFlagAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_QUALITYFLAGANALYZER_H
#define TRIGSCINT_QUALITYFLAGANALYZER_H

//LDMX Framework                               
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "TrigScint/Event/EventReadout.h"
#include "TrigScint/Event/TestBeamHit.h"
#include "TH1.h"
#include "TH2.h"

namespace trigscint {

  /**
   * @class QualityFlagAnalyzer
   * @brief
   */
  class QualityFlagAnalyzer : public framework::Analyzer {
  public:

	QualityFlagAnalyzer(const std::string& name, framework::Process& process); // : framework::Analyzer(name, process) {}
	virtual ~QualityFlagAnalyzer();
    virtual void configure(framework::config::Parameters &parameters);

    virtual void analyze( const framework::Event &event) final override;

	//
	virtual void onFileOpen();

	//
	virtual void onFileClose();

    virtual void onProcessStart() final override;

    virtual void onProcessEnd() final override;

	
  private:

	std::vector <std::vector <TH1F*> > vChargeVsTime;
	

	//configurable parameters
    std::string inputEventCol_;   //full event stream input 
    std::string inputEventPassName_{""};
    std::string inputHitCol_;     // hit collection
    std::string inputHitPassName_{""};
	std::vector<double> peds_;
	std::vector<double> gain_;
	int startSample_{0};

	//plotting stuff 
	int evNb;
    const int nEv{200};
    const int nChannels{16};
    const int nFlags{6};
	int peFillNb{0};
	
	//make sure to match constants above 
	const int flags[6] = {16,8,4,2,1,0};  //this order just makes looping easier
    int nEvDrawn[6]={0}; //keep a counter for each flag type to get good stats
	TH1F* hOut[200][16];
	TH1F* hOutPE[200][16];
	TH1F* hOutFlag[6][200][16]; //for 4 quality flags and 0 (no flag)
	TH1F* hPE[16];
	
	TH2F* hTDCfireChanvsEvent;

  };
}

#endif /* TRIGSCINT_QUALITYFLAGANALYZER_H */
