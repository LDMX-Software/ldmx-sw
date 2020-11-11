
#include "EventProc/ElectronCounter.h" 

namespace ldmx { 

    ElectronCounter::ElectronCounter(const std::string &name, Process &process) : 
        Producer(name, process) { 
    }

    ElectronCounter::~ElectronCounter() { 
    }

    void ElectronCounter::configure(Parameters& parameters) { 

        /**
         * You access configuration parameters set in the python
         * by asking for the parameter with the same name as the
         * python member variable.
         */

        inputColl_ = parameters.getParameter< std::string >("input_collection");
        inputPassName_ = parameters.getParameter< std::string >("input_pass_name");
        outputColl_ = parameters.getParameter< std::string >("output_collection");
        nElectronsSim_ = parameters.getParameter< int >("simulated_electron_number");
        useSimElectronCount_ = parameters.getParameter< bool >("use_simulated_electron_number");

		/*  // can rehash this for cluster vs track counting
		if (mode_ == 0) {
		  algoName_ = "LayerSumTrig";
        } else if (mode_ == 1) {
		  algoName_ = "CenterTower";
        }
		*/
		
        ldmx_log(debug) << "ElectronCounter is using parameters: "
						<< " \n\tinput_collection = "  << inputColl_ 
						<< " \n\tinput_pass_name = "  << inputPassName_ 
						<< " \n\toutput_collection = " << outputColl_ 
						<< " \n\tsimulated_electron_number = " << nElectronsSim_
						<< " \n\tuse_simulated_electron_number = " << useSimElectronCount_ ;
    }

    void ElectronCounter::produce(Event& event) { 

	  if (useSimElectronCount_ ) {

		if ( nElectronsSim_ < 0 ) {
		  ldmx_log(fatal) << "Can't use unset number of simulated electrons as electron count! Set with 'simulated_electron_number' " ;
		  return;
		}

		//then we just set it equal to simulated number and we're done
		 event.setElectronCount( nElectronsSim_ );
		 return;
	  }
		
	  // Check if the collection of trig scint tracks exist.  If not, 
	  // don't bother processing the event. 

	  if (!event.exists( inputColl_, inputPassName_ )) {
		ldmx_log(fatal) << "Attemping to use non-existing input collection " << inputColl_ << "_" << inputPassName_ << " to count electrons! Exiting." ;
		return;
	  }

	  //TODO, if cluster counting is needed: have two functions, one with tracks, one with clusters, and just call one or the other.
	  
	  // Get the collection of digitized ECal hits from the event
	  const std::vector<TrigScintTrack> tracks = event.getCollection<TrigScintTrack>( inputColl_, inputPassName_ );

	  event.setElectronCount( tracks.size() );

	  
	  ldmx_log(debug) << "Found " << tracks.size() << " electrons (tracks) using input collection " << inputColl_ << "_" << inputPassName_ ;
	  
	  //TODO? possibly put some numbers in an output collection

	}
} // ldmx

DECLARE_PRODUCER_NS(ldmx, ElectronCounter) 
