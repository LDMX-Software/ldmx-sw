/**
 * @file DummyAnalyzer.cxx
 * @brief Class that defines a dummy Analyzer implementation that just prints some messages
 * @author Jeremy Mans, University of Minnesota
 */

#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"
#include <iostream>
#include "TH1.h"
#include "Event/Event.h"
#include "Event/CalorimeterHit.h"
#include "TClonesArray.h"

namespace ldmx {

/**
 * @class DummyAnalyzer
 * @brief A dummy Analyzer implementation that just prints some messages and makes a simple histogram of calorimeter energy
 */
    class DummyAnalyzer : public ldmx::Analyzer {
	public:
	DummyAnalyzer(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name,process) { }

	virtual void configure(const ldmx::ParameterSet& ps) {
	    caloCol_=ps.getString("caloHitCollection");
	}
 
	virtual void analyze(const ldmx::Event& event) {
	    std::cout << "DummyAnalyzer: Analyzing an event!" << std::endl;
	    const TClonesArray* tca=event.getCollection(caloCol_);
	    for (size_t i=0; i<tca->GetEntriesFast(); i++) {
		const ldmx::CalorimeterHit* chit=(const ldmx::CalorimeterHit*)(tca->At(i));
		h_energy->Fill(chit->getEnergy());
	    }
	}
	virtual void onFileOpen() {
	    std::cout << "DummyAnalyzer: Opening a file!" << std::endl;
	}
	virtual void onFileClose() {
	    std::cout << "DummyAnalyzer: Closing a file!" << std::endl;
	}
	virtual void onProcessStart() {
	    std::cout << "DummyAnalyzer: Starting processing!" << std::endl;
	    getHistoDirectory();
	    h_energy=new TH1F("Energy","Energy",500,0.0,1.0);
	}
	virtual void onProcessEnd() {
	    std::cout << "DummyAnalyzer: Finishing processing!" << std::endl;
	}
	private:
	TH1* h_energy;
	std::string caloCol_;
    };
}

DECLARE_ANALYZER_NS(ldmx, DummyAnalyzer);
