//ANALYZER TO VIEW CLUSTERS

#include "Framework/EventProcessor.h"
#include "Framework/Event.h"
#include "TrigScint/Event/TrigScintCluster.h"
#include <iostream>
#include <fstream>


class ClusterViewerAnalyzer : public framework::Analyzer {
    public:
    
    std::ofstream outfile;
    
        ClusterViewerAnalyzer(const std::string& name, framework::Process& p)
          : framework::Analyzer(name, p) {
          
        outfile.open("clusters.txt");
        }
                
        void analyze(const framework::Event& event) override {
        
        const auto& clusters1 = 
        event.getCollection<ldmx::TrigScintCluster>("TriggerPad1Clusters","truthisolater");
        
        const auto& clusters2 = 
        event.getCollection<ldmx::TrigScintCluster>("TriggerPad2Clusters","truthisolater");
                
        const auto& clusters3 = 
        event.getCollection<ldmx::TrigScintCluster>("TriggerPad3Clusters","truthisolater");
        
        size_t Clusters = std::min({clusters1.size(), clusters2.size(), clusters3.size()});

        for (size_t i = 0; i < Clusters; i++) {

            int eventNumber = event.getEventNumber();
            float c1 = clusters1[i].getCentroid();
            float c2 = clusters2[i].getCentroid();
            float c3 = clusters3[i].getCentroid();
            float c3_alt = c3;

            if (clusters3.size() > i + 1) {
                c3_alt = clusters3[i + 1].getCentroid();
            }

            outfile << eventNumber << " "
                    << c1 << " "
                    << c2 << " "
                    << c3 << " "
                    << c3_alt << "\n";
        }
        	
    	
        for (size_t i = 1; i < clusters1.size(); i++){
            std::cout << "In event " << event.getEventNumber() <<", extra cluster in pad 1: " 
            << clusters1[i].getCentroid() << "\n"; 
        }	
        
        for (size_t i = 1; i < clusters2.size(); i++){
            std::cout << "In event " << event.getEventNumber() <<", extra cluster in pad 2: " 
            << clusters2[i].getCentroid() << "\n"; 
        }	
        
        for (size_t i = 1; i < clusters3.size(); i++){
            std::cout << "In event " << event.getEventNumber() <<", extra cluster in pad 3: " 
            <<  clusters3[i].getCentroid() << "\n"; 
        }	
        
        }
        void onProcessEnd() override {outfile.close();}
    };

DECLARE_ANALYZER(ClusterViewerAnalyzer)
