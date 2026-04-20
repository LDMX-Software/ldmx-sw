
#include "Framework/EventProcessor.h"
#include "Framework/Event.h"
#include "TrigScint/Event/TrigScintTrack.h"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

class LUTAnalyzer : public framework::Analyzer {
    public:

    struct Line {int event; float p1, p2, p3, p3alt;
    };
    
    std::ifstream infile;
    std::ofstream outfile;

    std::map<std::pair<float,float>, std::vector<Line>> groups;
    
    int totalLines = 0;
    
    LUTAnalyzer(const std::string& name, framework::Process& p)
     : framework::Analyzer(name,p) {

        infile.open("clusters.txt");
        outfile.open("LUT.txt");
    }

        void analyze(const framework::Event& event) override {
            if (totalLines > 0) return;
            
            int ev;
            float p1, p2, p3, p3alt;
            
            while (infile >> ev >> p1 >> p2 >> p3 >> p3alt) {
            
                float p12 = p2-p1;
                float p23 = p3-p2;
                
                groups[{p12,p23}].push_back({ev,p1,p2,p3,p3alt});
                totalLines++;
            }
        }
        
        void onProcessEnd() override {
            
            int combs = 0;
            int tracks = 0;
            
            std::cout << "Total number of track candidates: " << totalLines << "\n";
            std::cout << "Number of track candidate types: " << groups.size() << "\n";
            
            double threshold = 1.0/1000.0;
            std::cout << "Threshold: " << threshold << "\n";
            
            for (auto& g : groups) {
            
                float p12 = g.first.first;
                float p23 = g.first.second;
                int count = g.second.size();
                
                double frac = (double)count / totalLines;
                
                std::cout << "(" << p12 << "," << p23 << ") appears " 
                << count << " times, (" << frac << " %)" << "\n";
            }
            
            for (auto& g : groups) { 
                
                int count = g.second.size();
                double frac = (double)count / totalLines; 
                
                if (frac > threshold) {
                    
                    combs++;
                    
                    for (auto& line : g.second) {
                        
                        tracks++;
                        
                        outfile << line.p1 << " "
                                << line.p2 << " "
                                << line.p3 << "\n";
                    }
                }
            }
        std::cout << "\nLUT textfile written.";
        std::cout << "\n" << combs << " combinations (" << tracks << " tracks) written to LUT."  << "\n";
        }
};

DECLARE_ANALYZER(LUTAnalyzer)

	






