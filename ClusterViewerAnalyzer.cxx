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
        
        std::cout << "Pad1 clusters: " << clusters1.size() << std::endl;
        std::cout << "Pad2 clusters: " << clusters2.size() << std::endl;
        std::cout << "Pad3 clusters: " << clusters3.size() << std::endl;
        
        size_t Clusters = std::min({clusters1.size(), clusters2.size(), clusters3.size()});
              
        for (size_t i=0; i < Clusters; i++) {

            float c1 = clusters1[i].getCentroid();
            float c2 = clusters2[i].getCentroid();
            float c3 = clusters3[i].getCentroid();
            std::cout << "printed " << c1 << std::endl;
            std::cout << "printed " << c2 << std::endl;
            std::cout << "printed " << c3 << std::endl;
        	    outfile << c1 << " "
            		    << c2 << " "
            		    << c3 << "\n";
        	}
        }
        void onProcessEnd() override {outfile.close();}
    };

DECLARE_ANALYZER(ClusterViewerAnalyzer)
