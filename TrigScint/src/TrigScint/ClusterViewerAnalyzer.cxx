//ANALYZER TO VIEW CLUSTERS

#include "Framework/EventProcessor.h"
#include "Framework/Event.h"
#include "TrigScint/Event/TrigScintCluster.h"
#include <iostream>
#include <fstream>


class ClusterViewerAnalyzer : public framework::Analyzer {
    public:
    
    std::ofstream outfile;
        
        void configure(framework::config::Parameters& ps) override {
        pass_name_ = ps.get<std::string>("pass_name");
        output_file_ = ps.get<std::string>("output_file");
        
        outfile.open(output_file_);
        }
        
        std::string pass_name_;
        std::string output_file_;
        
        ClusterViewerAnalyzer(const std::string& name, framework::Process& p)
          : framework::Analyzer(name, p) {
          

        }
                
        void analyze(const framework::Event& event) override {
        
        const auto& clusters1 = 
        event.getCollection<ldmx::TrigScintCluster>("TriggerPad1Clusters", pass_name_);
        
        const auto& clusters2 = 
        event.getCollection<ldmx::TrigScintCluster>("TriggerPad2Clusters", pass_name_);
                
        const auto& clusters3 = 
        event.getCollection<ldmx::TrigScintCluster>("TriggerPad3Clusters", pass_name_);
        
        size_t Clusters = std::min({clusters1.size(), clusters2.size(), clusters3.size()});

        for (size_t i = 0; i < Clusters; i++) {

            int eventNumber = event.getEventNumber();
            float c1 = clusters1[i].getCentroid();
            float c2 = clusters2[i].getCentroid();
            float c3 = clusters3[i].getCentroid();

            outfile << eventNumber << " "
                    << c1 << " "
                    << c2 << " "
                    << c3 << "\n";
        }
        	
    	
        for (size_t i = 1; i < clusters1.size(); i++){
            ldmx_log(trace) << "In event " << event.getEventNumber() << ", extra cluster in pad 1: " 
            << clusters1[i].getCentroid() << "\n"; 
        }	
        
        for (size_t i = 1; i < clusters2.size(); i++){
            ldmx_log(trace) << "In event " << event.getEventNumber() << ", extra cluster in pad 2: " 
            << clusters2[i].getCentroid() << "\n"; 
        }	
        
        for (size_t i = 1; i < clusters3.size(); i++){
            ldmx_log(trace) << "In event " << event.getEventNumber() << ", extra cluster in pad 3: " 
            <<  clusters3[i].getCentroid() << "\n"; 
        }	
        
        }
        void onProcessEnd() override {outfile.close();}
    };

DECLARE_ANALYZER(ClusterViewerAnalyzer)
