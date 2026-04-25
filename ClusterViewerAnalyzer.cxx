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
        
        outfile.open("clusters.txt"); //Cluster combinations will be saved in txt file
        }
        
        void analyze(const framework::Event& event) override {
        
            const auto& clusters1 = 
            event.getCollection<ldmx::TrigScintCluster>("TriggerPad1Clusters", "uptoclustering");
            
            const auto& clusters2 = 
            event.getCollection<ldmx::TrigScintCluster>("TriggerPad2Clusters", "uptoclustering");
                    
            const auto& clusters3 = 
            event.getCollection<ldmx::TrigScintCluster>("TriggerPad3Clusters", "uptoclustering");
            
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
    	
            	//extra clusters should only very rarely (1 in 10.000) occur if the 
            	//clustering_threshold is set to >3 when using TruthHitProducer.cxx.  
            	//Without THP, extra clusters will still exist, but TrigScintTrackProducer  
            	//seems to be able to choose the "correct" cluster for track-making anyways. 
                	
            for (size_t i = 1; i < clusters1.size(); i++){
                std::cout << "In event "
                          << event.getEventNumber() 
                          << ", extra cluster in pad 1: " 
                          << clusters1[i].getCentroid() 
                          << "\n"; 
            }	
        
            for (size_t i = 1; i < clusters2.size(); i++){
                std::cout << "In event " 
                          << event.getEventNumber() 
                          << ", extra cluster in pad 2: " 
                          << clusters2[i].getCentroid() 
                          << "\n"; 
            }	
            
            for (size_t i = 1; i < clusters3.size(); i++){
                std::cout << "In event " 
                          << event.getEventNumber() 
                          << ", extra cluster in pad 3: " 
                          <<  clusters3[i].getCentroid() 
                          << "\n"; 
            }	
        
        }
        
        void onProcessEnd() override {outfile.close();}
    };

DECLARE_ANALYZER(ClusterViewerAnalyzer)
