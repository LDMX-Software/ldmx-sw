/**
 * @file QIEAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_QIEANALYZER_H
#define TRIGSCINT_QIEANALYZER_H

//LDMX Framework                               
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "TrigScint/Event/EventReadout.h"
#include "TH1.h"
#include "TH2.h"

namespace trigscint {

  /**
   * @class QIEAnalyzer
   * @brief
   */
  class QIEAnalyzer : public framework::Analyzer {
  public:

	QIEAnalyzer(const std::string& name, framework::Process& process); // : framework::Analyzer(name, process) {}
	virtual ~QIEAnalyzer();
    virtual void configure(framework::config::Parameters &parameters);

    virtual void analyze(const framework::Event &event) final override;

	//
	virtual void onFileOpen();

	//
	virtual void onFileClose();

    virtual void onProcessStart() final override;

    virtual void onProcessEnd() final override;

    float convertToID( float yVal ) { return (yVal+yOffset_)*yToIDfactor_; }

  private:

	std::vector <std::vector <TH1F*> > vChargeVsTime;
	
    // 
    // TH1* hId;
    // TH1* hNtracksEvnb;
    // TH1* hNtracks;
    // TH1* hFindableTracks;
    // TH2* hTrackMatrix;
    // TH1* hNhits;
    // TH2 *hSimYEvnb;
    // TH2 *hSimIDEvnb;
    // TH2 *   hIdEvnb;
	
    std::string inputCol_;
    std::string inputPassName_{""};
	std::vector<double> peds_;

	int evNb;
    //    const 
    int nEv{200};
    int nChannels{16};
    int nTrkMax{100};

	//match nev, nchan above
	TH1F* hOut[200][16];;
	TH1F* hPE[16];
	TH2F* hPEvsT[16];
	
    double yOffset_{35.};
    double yToIDfactor_{50./80.};

  };
}

#endif /* TRIGSCINT_QIEANALYZER_H */
