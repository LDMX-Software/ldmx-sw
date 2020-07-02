/**
 * @file DummyAnalyzer.cxx
 * @brief Class that defines a dummy Analyzer implementation that just prints some messages
 * @author Jeremy Mans, University of Minnesota
 */


/*~~~~~~~~~~~~*/
/*   StdLib   */
/*~~~~~~~~~~~~*/
#include <any>
#include <iostream>
#include <variant>

/*~~~~~~~~~~*/
/*   ROOT   */ 
/*~~~~~~~~~~*/
#include "TH1.h"

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Event/CalorimeterHit.h"

/*~~~~~~~~~~~~~~*/
/*   Framework  */
/*~~~~~~~~~~~~~~*/
#include "Framework/Event.h"
#include "Framework/EventFile.h" 
#include "Framework/EventProcessor.h"
#include "Framework/NtupleManager.h"
#include "Framework/Process.h" 

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
            void configure(Parameters& parameters) final override { 
                caloCol_ = parameters.getParameter< std::string >("caloHitCollection");  
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

                //All processors have access to the logger using
                //the 'ldmx_log' macro.
                ldmx_log(debug) << "[ DummyAnalyzer]: Analyzing an event!";
                
                // Get the collection of calorimeter hits from the event.
                auto tca = event.getCollection<CalorimeterHit>(caloCol_);  
                
                // Loop through the collection and fill the histogram
                float maxEnergyHit = -1.;

                for (const CalorimeterHit &hit : tca) {  
                    
                    // Fill the histogram
                    hEnergy->Fill(hit.getEnergy());

                    // Check if this hits energy is bigger
                    if ( hit.getEnergy() > maxEnergyHit ) maxEnergyHit = hit.getEnergy();
                }
                
                // set the ntuple variable
                //  the ntuple is filled at the end of each event
                ntuple_.setVar<float>("energy", maxEnergyHit);

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
            void onFileOpen(EventFile& eventFile) final override {
                std::cout << "[ DummyAnalyzer ]: Opening " 
                          << eventFile.getFileName() << "!" << std::endl;
            }

            /**
             *  Callback for the analyzer to take any action once the file 
             *  has been closed. 
             */
            void onFileClose(EventFile& eventFile) final override {
                std::cout << "[ DummyAnalyzer ]: Closing " 
                          << eventFile.getFileName() << "!" << std::endl;
            }

            /**
             * Callback for the analyzer to take any action before the 
             * processing of events begins. 
             */
            void onProcessStart() final override {
                std::cout << "[ DummyAnalyzer ]: Starting processing!" << std::endl;
               
                // Create a tree.  Note that the openHistoFile method needs to 
                // be called first.  By doing so, the histogram file is opened
                // if it hasn't been by another process and the current 
                // directory is changed to the top level directory.
                process_.openHistoFile(); 

                // Create a tree and add variable to it 
                //    the ntuple_ member variable is inherited from EventProcessor
                ntuple_.create("Dummy");
                ntuple_.addVar<float>("Dummy", /* tree name */ 
                                      "energy"); 

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

DECLARE_ANALYZER_NS(ldmx, DummyAnalyzer)
