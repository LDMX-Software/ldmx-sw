//WRITES LUT BASED ON FREQUENCY OF TRACK CANDIDATE PROPAGATION COMBINATIONS
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
    
    void configure(framework::config::Parameters& ps) override {
        input_file_ = ps.get<std::string>("input_file");
        output_file_ = ps.get<std::string>("output_file");
        lut_threshold_ = ps.get<double>("lut_threshold"); 
        //The LUT threshold is the minimum percentage of times that a track 
        //must appear in a pool of events to be written to the LUT. 
        //Example --- straight tracks whose pad 1-2 and pad 2-3 delta values 
        //            are both 0 make up ~85% of tracks out of 10.000 events,
        //            while a track with deltas (+22,-22), meaning pad 1 cluster in 
        //            e.g. bar 4, pad 2 cluster in bar 26, and pad 3 cluster in bar 4
        //            only appears once in 10.000 events (0.01%). The straight 
        //            tracks are written to the LUT and the single "anomaly" is not
        
    }

    std::ifstream infile;
    std::ofstream outfile; 
    std::string input_file_;
    std::string output_file_;
    double lut_threshold_;
    
    std::map<std::pair<float,float>, std::vector<Line>> groups;
    
    int totalLines = 0;
    
    LUTAnalyzer(const std::string& name, framework::Process& p)
     : framework::Analyzer(name,p) {
    }
        void onProcessStart() override {
        infile.open(input_file_);
        outfile.open(output_file_);
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
            std::cout << "LUT Threshold: " << lut_threshold_ * 100 << "%\n";
            
            for (auto& g : groups) {
            
                float p12 = g.first.first;
                float p23 = g.first.second;
                int count = g.second.size();
                
                double frac = (double)count / totalLines;
                
                std::cout << "(" << p12 << "," << p23 << ") appears " 
                << count << " times, (" << frac * 100 << " %)" << "\n";
            }
            
            for (auto& g : groups) { 
                
                int count = g.second.size();
                double frac = (double)count / totalLines; 
                
                if (frac > lut_threshold_) {
                    
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
