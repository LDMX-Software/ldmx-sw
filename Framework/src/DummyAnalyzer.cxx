/**
 * @file DummyAnalyzer.cxx
 * @brief Class that defines a dummy Analyzer implementation that just prints some messages
 * @author Jeremy Mans, University of Minnesota
 */

#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"
#include <iostream>
#include "TH1.h"
#include "Framework/Event.h"
#include "Event/CalorimeterHit.h"

namespace ldmx {

    /**
     * @class DummyAnalyzer
     * @brief A dummy Analyzer implementation that just prints some messages and makes a simple histogram of calorimeter energy
     */
    class DummyAnalyzer : public ldmx::Analyzer {
        public:

      DummyAnalyzer(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name, process) { ievt=0; }

            virtual void configure(const ldmx::ParameterSet& ps) {
                caloCol_=ps.getString("caloHitCollection");
            }

            virtual void analyze(const ldmx::Event& event) {
                std::cout << "DummyAnalyzer: Analyzing an event!" << std::endl;
                const std::vector<CalorimeterHit> getCaloHits = event.getCollection< CalorimeterHit >( caloCol_ );
                for ( const CalorimeterHit &chit : getCaloHits ) {
                    h_energy->Fill(chit.getEnergy());
                }
        		if (ievt==0) {
        		  std::vector<ProductTag> pts=event.getProducts();
        		  std::cout << "\nDemonstration of printing out all the event contents\n";
        		  for (auto j: pts) {
        		    std::cout << "  " << j << std::endl;
        		  }
        		  std::cout << std::endl;
        		}
        		if (ievt==1) {
        		  std::vector<ProductTag> pts=event.searchProducts("","sim","");
        		  std::cout << "\nDemonstration of searching for all products with pass name 'sim'\n";
        		  for (auto j: pts) {
        		    std::cout << "   " << j << std::endl;
        		  }
        		  std::cout << std::endl;
        		}
        		if (ievt==2) {
        		  std::vector<ProductTag> pts=event.searchProducts(".*cal.*","","");
        		  std::cout << "\nDemonstration of searching for all products with 'cal' anywhere in the product name\n";
        		  for (auto j: pts) {
        		    std::cout << "   " << j << std::endl;
        		  }
        		  std::cout << std::endl;
        		}
        		ievt++;

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
            int ievt;
    };
}

DECLARE_ANALYZER_NS(ldmx, DummyAnalyzer);
