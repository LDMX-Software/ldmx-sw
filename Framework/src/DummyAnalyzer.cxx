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
     * @brief A dummy Analyzer implementation that just prints some messages 
     *        and makes a simple histogram of calorimeter energy
     */
    class DummyAnalyzer : public ldmx::Analyzer {
        
        public:
            
            /**
             * Class constructor.
             *
             * @param name Name for this instance of the class.
             * @param process The Process used to run this analyzer, provided 
             *                by the framework. 
             */
            DummyAnalyzer(const std::string& name, ldmx::Process& process) 
                : ldmx::Analyzer(name, process) { 
                    ievt=0; 
            }

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param pSet Set of parameters used to configure this processor.
             */
            virtual void configure(const ldmx::ParameterSet& ps) {
                caloCol_=ps.getString("caloHitCollection");
            }

            /// Destructor 
            ~DummyAnalyzer() {}; 

            /**
             * Process the event, print some messages, and make a simple 
             * histogram and ntuple out of the calorimeter energy. 
             *
             * @param event The event to analyze. 
             */
            virtual void analyze(const ldmx::Event& event) {

                std::cout << "[ DummyAnalyzer]: Analyzing an event!" << std::endl;

                // Get the collection of calorimeter hits from the event. 
                const TClonesArray* tca=event.getCollection(caloCol_);
                
                // Loop through the collection and fill both the histogram and 
                // ntuple. 
                for (size_t i=0; i<tca->GetEntriesFast(); i++) {
                    const ldmx::CalorimeterHit* chit=(const ldmx::CalorimeterHit*)(tca->At(i));
                    hEnergy->Fill(chit->getEnergy());
                }

                // Print out all of the product tags in the event.
                if (ievt==0) {
                    std::vector<ProductTag> pts=event.getProducts();
                    std::cout << "\nDemonstration of printing out all the event contents\n";
                    for (auto j: pts) {
                        std::cout << "  " << j << std::endl;
                    }
                    std::cout << std::endl;
                }

                // Search for all of the products with the pass name 'sim' 
                if (ievt==1) {
                    std::vector<ProductTag> pts=event.searchProducts("","sim","");
                    std::cout << "\nDemonstration of searching for all products with pass name 'sim'\n";
                    for (auto j: pts) {
                        std::cout << "   " << j << std::endl;
                    }
                    std::cout << std::endl;
                }

                //  Search for all of the products with the 'cal' in the product
                //  name. 
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

            /** 
             *  Callback for the analyer to take any action once a file has been
             *  opened. 
             */
            virtual void onFileOpen() {
                std::cout << "DummyAnalyzer: Opening a file!" << std::endl;
            }

            /**
             *  Callback for the analyzer to take any action once the file 
             *  has been closed. 
             */
            virtual void onFileClose() {
                std::cout << "DummyAnalyzer: Closing a file!" << std::endl;
            }

            /**
             * Callback for the analyzer to take any action before the 
             * processing of events begins. 
             */
            virtual void onProcessStart() {
                std::cout << "DummyAnalyzer: Starting processing!" << std::endl;
                getHistoDirectory();
                hEnergy=new TH1F("Energy","Energy",500,0.0,1.0);
            }

            /** 
             * Callback for the analyzer to take any action after the processing
             * of all events has ended. 
             */
            virtual void onProcessEnd() {
                std::cout << "DummyAnalyzer: Finishing processing!" << std::endl;
            }

        private:

            /// Histogram of the calorimeter energy
            TH1* hEnergy{nullptr};

            /**
             * Name of the collection containing the name of the calorimeter
             * hits.
             */
            std::string caloCol_;

            /// Event counter
            int ievt;
    };
}

DECLARE_ANALYZER_NS(ldmx, DummyAnalyzer);
