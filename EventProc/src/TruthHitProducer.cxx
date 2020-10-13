
#include "EventProc/TruthHitProducer.h" 

namespace ldmx { 

    TruthHitProducer::TruthHitProducer(const std::string &name, Process &process) : 
        Producer(name, process) { 
    }

    TruthHitProducer::~TruthHitProducer() { 
    }

    void TruthHitProducer::configure(Parameters& parameters) { 

        /**
         * You access configuration parameters set in the python
         * by asking for the parameter with the same name as the
         * python member variable.
         */

	    inputCollection_  = parameters.getParameter< std::string >("input_collection");
        inputPassName_    = parameters.getParameter< std::string >("input_pass_name" );
        outputCollection_ = parameters.getParameter< std::string >("output_collection");
        verbose_          = parameters.getParameter< bool >("verbose");


   if (verbose_)
        {
          ldmx_log(info) << "In TruthHitProducer: configure done!" ;
          ldmx_log(info)<< "Got parameters:  " 
                        << "\nInput collection:     " << inputCollection_
                        << "\nInput pass name:     " << inputPassName_
                        << "\nOutput collection:    " << outputCollection_
                        << "\nVerbose: " << verbose_ ;

        }

    }

    void TruthHitProducer::produce(Event& event) { 

        // Check if the collection of reconstructed ECal hits exist.  If not, 
        // don't bother processing the event. 
	  if ( !event.exists(inputCollection_) ) {
		ldmx_log(error) << "No input collection called " << inputCollection_ << " found; skipping!" ; 
		return; 

	  }
        // looper over sim hits and aggregate energy depositions for each detID
        const auto simHits{event.getCollection< SimCalorimeterHit >(inputCollection_,inputPassName_)};
        auto particleMap{event.getMap< int, SimParticle >("SimParticles")};


		std::vector < SimCalorimeterHit > truthBeamElectrons ;

		for (const auto& simHit : simHits) {          
		  bool keep = false;
			  // check if hits is from beam electron and, if so, add to output collection
		  for( int i = 0 ; i < simHit.getNumberOfContribs() ; i++){
			
			auto contrib = simHit.getContrib(i);
			if( verbose_ ){
			  ldmx_log(debug) << "contrib " << i << " trackID: " << contrib.trackID << " pdgID: " << contrib.pdgCode << " edep: " << contrib.edep ;
			  ldmx_log(debug) << "\t particle id: " << particleMap[contrib.trackID].getPdgID() << " particle status: " << particleMap[contrib.trackID].getGenStatus();
			}  //beam electron (PDGID = 11, genStatus == 1) 
			if( particleMap[contrib.trackID].getPdgID() == 11 && particleMap[contrib.trackID].getGenStatus() == 1 ){
			  keep = true;
			}
			if (keep)
			  truthBeamElectrons.push_back( simHit );
		  }
		}
		event.add(outputCollection_, truthBeamElectrons);
    }
} // ldmx

DECLARE_PRODUCER_NS(ldmx, TruthHitProducer) 
