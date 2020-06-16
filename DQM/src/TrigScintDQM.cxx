/**
 * @file TrigScintDQM.cxx
 * @brief Analyzer used for TrigScint DQM. 
 * @author Omar Moreno, SLAC National Accelerator
 * @author Lene Kristian Bryngemark, Stanford University 
 */

#include "DQM/TrigScintDQM.h" 

namespace ldmx { 

    TrigScintDQM::TrigScintDQM(const std::string &name, Process &process) : 
        Analyzer(name, process) { }

    TrigScintDQM::~TrigScintDQM() {}

    void TrigScintDQM::onProcessStart() {

        std::cout << "Process starts!" << std::endl;

        getHistoDirectory();
    
        histograms_.create("id", "Channel ID of sim hit", 100, 0, 100);
        histograms_.create("total_energy", "Total energy deposition in the pad/event", 3000, 0, 3000);
        histograms_.create("n_hits", "TrigScint hit multiplicity in the pad/event", 100, 0, 100);
        histograms_.create("x", "Hit x position", 1000, -100, 100);
        histograms_.create("y", "Hit y position", 1000, -100, 100);
        histograms_.create("z", "Hit z position", 1000, -900, 100);
        
        histograms_.create("energy", "Energy deposition in a TrigScint bar", 1500, 0, 1500);
        histograms_.create("hit_time", "TrigScint hit time (ns)", 1600, -100, 1500);
        
        histograms_.create("max_pe:time", 
                           "Max Photoelectrons in a TrigScint bar", 1500, 0, 1500, 
                           "TrigScint max PE hit time (ns)", 1500, 0, 1500);
    
        histograms_.create("min_time_hit_above_thresh:pe", 
                           "Photoelectrons in a TrigScint bar", 1500, 0, 1500, 
                           "Earliest time of TrigScint hit above threshold (ns)", 1600, -100, 1500);

    }

    void TrigScintDQM::configure(Parameters& ps) {
        hitCollectionName_ = ps.getParameter< std::string >("hit_collection");
        padName_ = ps.getParameter< std::string >("pad");

        std::cout << "In TrigScintDQM::configure, got parameters " << hitCollectionName_ << " and " << padName_ << std::endl;

        detID_= std::make_unique<TrigScintID>();
    }

    void TrigScintDQM::analyze(const Event & event) { 

        // Check if the collection of digitized TrigScint hits exist. If it doesn't 
        // don't continue processing.
        if ( !event.exists(hitCollectionName_.c_str()) ) {
            std::cout << "No collection called " << hitCollectionName_ << std::endl;
            return; 
        }
      
        const std::vector<SimCalorimeterHit> TrigScintHits = event.getCollection<SimCalorimeterHit>( hitCollectionName_);
      
        // Get the total hit count
        int hitCount = TrigScintHits.size();  
        histograms_.fill("n_hits",hitCount);
      
        double totalEnergy{0};  
        for (const SimCalorimeterHit &hit : TrigScintHits ) {
    
            int detIDRaw{hit.getID()};
            detID_->setRawValue( hit.getID() ); 
            detID_->unpack();
            int bar = detID_->getFieldValue("bar");
        
            histograms_.fill("energy",hit.getEdep()); 
            histograms_.fill("hit_time",hit.getTime());
            histograms_.fill("id", bar );
            
            std::vector<float> posvec = hit.getPosition();
            histograms_.fill("x", posvec.at(0) );
            histograms_.fill("y", posvec.at(1) );
            histograms_.fill("z", posvec.at(2) );
                
            totalEnergy += hit.getEdep(); 
        }
      
        histograms_.fill("total_energy",totalEnergy);
      
    }
  
} // ldmx

DECLARE_ANALYZER_NS(ldmx, TrigScintDQM)
