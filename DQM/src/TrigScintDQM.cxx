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
       
        // Get an instance of the histogram pool  
        histograms_ = HistogramPool::getInstance();    


        // Move into the TrigScint directory
	getHistoDirectory();

	// would like to avoid individual naming of histograms -- they are in different histogram dirs so i would have thought that 
	// they could be written individually. however i think this would require passing a name to getInstance() or similar? 
	// without that it looks like i always end up writing from all collections to the latest set of histograms created. 

	// so this is what i would like to, but can't do, for now:

	/*
	histDir_ = getHistoDirectory();

	histograms_->create<TH1F>("id", "Channel ID of sim hit", 100, 0, 100);
	histograms_->create<TH1F>("total_energy", "Total energy deposition in the pad/event", 3000, 0, 3000);
	histograms_->create<TH1F>("n_hits", "TrigScint hit multiplicity in the pad/event", 100, 0, 100);
	histograms_->create<TH1F>("x", "Hit x position", 1000, -100, 100);
	histograms_->create<TH1F>("y", "Hit y position", 1000, -100, 100);
	histograms_->create<TH1F>("z", "Hit z position", 1000, -900, 100);
	
	histograms_->create<TH1F>("energy", "Energy deposition in a TrigScint bar", 1500, 0, 1500);
	histograms_->create<TH1F>("hit_time", "TrigScint hit time (ns)", 1600, -100, 1500);
	
        histograms_->create<TH2F>("max_pe:time", 
                                  "Max Photoelectrons in a TrigScint bar", 1500, 0, 1500, 
                                  "TrigScint max PE hit time (ns)", 1500, 0, 1500);

        histograms_->create<TH2F>("min_time_hit_above_thresh:pe", 
                                  "Photoelectrons in a TrigScint bar", 1500, 0, 1500, 
                                  "Earliest time of TrigScint hit above threshold (ns)", 1600, -100, 1500);
	*/



	// instead do this 

	histograms_->create<TH1F>("id_"+padName_, "Channel ID of sim hit", 100, 0, 100);
        histograms_->create<TH1F>("total_energy_"+padName_, "Total energy deposition in the pad/event", 3000, 0, 3000);
        histograms_->create<TH1F>("n_hits_"+padName_, "TrigScint hit multiplicity in the pad/event", 100, 0, 100);
        histograms_->create<TH1F>("x_"+padName_, "Hit x position", 1000, -100, 100);
        histograms_->create<TH1F>("y_"+padName_, "Hit y position", 1000, -100, 100);
        histograms_->create<TH1F>("z_"+padName_, "Hit z position", 5000, -900, 100);

        histograms_->create<TH1F>("energy_"+padName_, "Energy deposition in a TrigScint bar", 1500, 0, 1500);
        histograms_->create<TH1F>("hit_time_"+padName_, "TrigScint hit time (ns)", 1600, -100, 1500);

        histograms_->create<TH2F>("max_pe:time_"+padName_,
                                  "Max Photoelectrons in a TrigScint bar", 1500, 0, 1500,
                                  "TrigScint max PE hit time (ns)", 1500, 0, 1500);

        histograms_->create<TH2F>("min_time_hit_above_thresh:pe_"+padName_,
                                  "Photoelectrons in a TrigScint bar", 1500, 0, 1500,
                                  "Earliest time of TrigScint hit above threshold (ns)", 1600, -100, 1500);



	// TODO: implement getting a list of the constructed histograms, to iterate through and set overflow boolean. 


    }

    void TrigScintDQM::configure(Parameters& ps) {
      hitCollectionName_ = ps.getParameter< std::string >("hit_collection");
      padName_ = ps.getParameter< std::string >("pad");

      std::cout << "In TrigScintDQM::configure, got parameters " << hitCollectionName_ << " and " << padName_ << std::endl;

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
      histograms_->get("n_hits_"+padName_)->Fill(hitCount); 
      
      double totalEnergy{0};  

      for (const SimCalorimeterHit &hit : TrigScintHits ) {
	
	histograms_->get("energy_"+padName_)->Fill(hit.getEdep()); 
	histograms_->get("hit_time_"+padName_)->Fill(hit.getTime());
	histograms_->get("id_"+padName_)->Fill(hit.getID()>>4 );
	
	std::vector<float> posvec = hit.getPosition();
	histograms_->get("x_"+padName_)->Fill( posvec.at(0) );
	histograms_->get("y_"+padName_)->Fill( posvec.at(1) );
	histograms_->get("z_"+padName_)->Fill( posvec.at(2) );
        
	totalEnergy += hit.getEdep(); 
      }
      
      histograms_->get("total_energy_"+padName_)->Fill(totalEnergy); 
      
    }
  
} // ldmx

DECLARE_ANALYZER_NS(ldmx, TrigScintDQM)
