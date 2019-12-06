/**
 * @file DummyAnalyzer.cxx
 * @brief Class that defines a dummy Analyzer implementation that just prints some messages
 * @author Jeremy Mans, University of Minnesota
 */


/*~~~~~~~~~~~~*/
/*   StdLib   */
/*~~~~~~~~~~~~*/
#include <iostream>

/*~~~~~~~~~~*/
/*   ROOT   */ 
/*~~~~~~~~~~*/
#include "TClonesArray.h"
#include "TH1.h"

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Event/CalorimeterHit.h"
#include "Event/Event.h"

/*~~~~~~~~~~~~~~*/
/*   Framework  */
/*~~~~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

namespace ldmx {

    /**
     * @class DummyAnalyzer
     * @brief A dummy Analyzer implementation that just prints some messages 
     *        and makes a simple histogram of calorimeter energy
     */
    class DummyAnalyzer : public Analyzer {
        
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
            }

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param pSet Set of parameters used to configure this processor.
             */
            void configure(const ldmx::ParameterSet& ps) final override {
                caloCol_ = ps.getString("caloHitCollection");
            }

            /// Destructor 
            ~DummyAnalyzer() {}; 

            /**
             * Process the event, print some messages, and make a simple 
             * histogram and ntuple out of the calorimeter energy. 
             *
             * @param event The event to analyze. 
             */
            void analyze(const ldmx::Event& event) final override {

                std::cout << "[ DummyAnalyzer]: Analyzing an event!" << std::endl;

                // Get the collection of calorimeter hits from the event. 
                const TClonesArray* tca = event.getCollection(caloCol_);
                
                // Loop through the collection and fill both the histogram and 
                // ntuple. 
                for (size_t i{0}; i < tca->GetEntriesFast(); ++i) {
                    
                    // Get the ith hit from the collection.
                    auto iHit{static_cast<const CalorimeterHit*>(tca->At(i))}; 

                    // Fill the histogram
                    hEnergy->Fill(iHit->getEnergy());
                }

                // Print out all of the product tags in the event.
                if (ievt == 0) {
                    
                    std::cout << "[ DummyAnalyzer ]: Demonstration of printing out all the event contents." << std::endl;
                    
                    auto pts = event.getProducts();
                    for (const auto &j : pts) std::cout << "\t" << j << std::endl;
                }

                // Search for all of the products with the pass name 'sim' 
                if (ievt == 1) {
                    
                    std::cout << "[ DummyAnalyzer ]: Demonstration of searching for all products with pass name 'sim'." << std::endl;
                    
                    auto pts = event.searchProducts("","sim","");
                    for (const auto &j : pts) std::cout << "\t" << j << std::endl;
                }

                //  Search for all of the products with the 'cal' in the product
                //  name. 
                if (ievt == 2) {
                    
                    std::cout << "[ DummyAnalyzer ]: Demonstration of searching for all products with 'cal' anywhere in the product name"
                              << std::endl;
                    
                    auto pts = event.searchProducts(".*cal.*","","");
                    for (const auto &j : pts) std::cout << "\t" << j << std::endl;
                }
                
                ++ievt;
            }

            /** 
             *  Callback for the analyer to take any action once a file has been
             *  opened. 
             */
            void onFileOpen(const std::string& filename) final override {
                std::cout << "[ DummyAnalyzer ]: Opening " 
                          << filename << "!" << std::endl;
            }

            /**
             *  Callback for the analyzer to take any action once the file 
             *  has been closed. 
             */
            void onFileClose(const std::string& filename) final override {
                std::cout << "[ DummyAnalyzer ]: Closing " 
                          << filename << "!" << std::endl;
            }

            /**
             * Callback for the analyzer to take any action before the 
             * processing of events begins. 
             */
            void onProcessStart() final override {
                std::cout << "[ DummyAnalyzer ]: Starting processing!" << std::endl;
               
                // Create all histograms needed by the analyzer  
                getHistoDirectory();
                hEnergy = new TH1F("Energy","Energy",500,0.0,1.0);
            }

            /** 
             * Callback for the analyzer to take any action after the processing
             * of all events has ended. 
             */
            void onProcessEnd() final override {
                std::cout << "[ DummyAnalyzer ]: Finishing processing!" << std::endl;
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
            int ievt{0};

    }; // DummyAnalyzer 

} // ldmx

DECLARE_ANALYZER_NS(ldmx, DummyAnalyzer);
