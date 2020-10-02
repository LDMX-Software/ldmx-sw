/**
 * @file OverlayProducer.cxx
 * @brief In-time pile-up producer  
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "EventProc/OverlayProducer.h"

namespace ldmx {

    void OverlayProducer::configure(Parameters& parameters) {

      // Configure this instance of the producer
	  std::cout << "[ OverlayProducer ]: configure " << std::endl;
	  
                            
      overlayFileName_     = parameters.getParameter< std::string >("overlayFileName");
      overlayTreeName_     = parameters.getParameter< std::string >("overlayTreeName");
      poissonMu_           = parameters.getParameter< double >("numberOverlaidInteractions");
      doPoisson_           = parameters.getParameter< int >("doPoisson");
      overlayProcessName_  = parameters.getParameter< std::string >("overlayProcessName");
      collections_         = parameters.getParameter< std::vector <std::string> >("overlayHitCollections");
      seed_                = parameters.getParameter< int >("randomSeed");
      passName_            = "overlay";
      verbosity_           = parameters.getParameter< int >("verbosity");


	  simParticleCollName_ = parameters.getParameter<std::string>( "collection_name" );
      simParticlePassName_ = parameters.getParameter<std::string>( "pass_name" );
      overlayPassName_     = parameters.getParameter<std::string>( "overlay_pass_name" );

      if (verbosity_) {
	std::cout << "[ OverlayProducer ]: Got parameters \n \t overlayFileName = " << overlayFileName_ 
		  << "\n\t overlayTreeName = " << overlayTreeName_
		  << "\n\t numberOverlaidInteractions = " << poissonMu_
		  << "\n\t doPoisson = " << doPoisson_
		  << "\n\t overlayProcessName = " << overlayProcessName_
		  << "\n\t overlayHitCollections = " ;
	for ( const std::string &coll : collections_ )
	  std::cout << coll << "; " ;
	std::cout << "\n\t randomSeed = " << seed_
		  << "\n\t passName hardcoded to " << passName_
		  << "\n\t verbosity = " << verbosity_
		  << std::endl;

	  //		  << "\n\t  = " << 
      }


     

        return;
    }

    void OverlayProducer::produce(Event& event) {
      //event is the incoming, simulated event/"hard" process
      //event_ is the overlay producer's own event. 
      if (verbosity_ > 2) {
	std::cout << "[ OverlayProducer ]: produce() starts on event " <<  event.getEventHeader().getEventNumber() << std::endl;
      }

      if (! overlayFile_->nextEvent() ) {
	std::cerr << "Couldn't read next event!" << std::endl;
	return;
      }

      Event * overlayEvent = overlayFile_->getEvent();
      if (! overlayEvent ) {
	std::cerr << "No overlay event!" << std::endl;
	return;
      }
      else 
	if (verbosity_ > 2) {
	  std::cout  << "Got an overlay event " << std::endl;
	}

      if (verbosity_ > 2) {
	std::cout << "[ OverlayProducer ]: starting from overlay event " <<  overlayEvent->getEventHeader().getEventNumber() << std::endl;
      }

      // sample a poisson distribution or use a deterministic number of overlay events : this is nEvsOverlay

      int nEvsOverlay = doPoisson_ ? (int) rndm_->Poisson( poissonMu_ ) : (int)poissonMu_ ;  //either sample or take the number at face value every time 

      // find if lastEvent_ + nEvsOverlay > nEvents in the tree as a whole. in that case, reset lastEvent to 0 or (some random small nb)
      if (lastEvent_ + nEvsOverlay >= nEventsTot_)
		lastEvent_ = 0;
      
      if (verbosity_ > 2) {
		std::cout << "[ OverlayProducer ]: will overlay " <<  nEvsOverlay << " events on the simulated one" << std::endl;
      }
      
      // then with each collection, loop over nEvs, and push back all the hits in the overlay collection to the input collection
      for (int iEv = 0 ; iEv < nEvsOverlay ; iEv++ ) {
		
		if (verbosity_ > 2) {
		  std::cout << "[ OverlayProducer ]: in loop: overlaying event " <<  iEv << std::endl;
		}
		
		
		// get the collections that we want to overlay, by looping over the list of collections passed to the producer : collections_
		for (uint iColl = 0; iColl < collections_.size(); iColl++) {
		  
		  std::vector<SimCalorimeterHit> simHits = event.getCollection<SimCalorimeterHit>( collections_[iColl] );
		  
		  std::vector<SimCalorimeterHit> overlayHits = overlayEvent->getCollection<SimCalorimeterHit>( collections_[iColl] );
		  for (const SimCalorimeterHit &simHit : simHits ) {
			
			simHit.Print();
			
		  } //over calo simhit collection
		  
		  for (const SimCalorimeterHit &overlayHit : overlayHits ) {
			
			overlayHit.Print();
			
		  } //over overlay calo simhit collection
		  
		  
		} // over collections 
		
		overlayEvent->nextEvent();
		
		// done. 
	    //finally, increment book keeper of which events we've used 
	    lastEvent_++;
		
      } // over events 
	  
	  
	  return;
    }
  
  void OverlayProducer::onFileOpen() {//EventFile& input) {
	if (verbosity_ > 2) {
	  std::cout << "[ OverlayProducer ]: onFileOpen() " << std::endl;
	}
	
	//      simFile_ = std::make_unique<EventFile*> ( input );
	//      simFile_=&input ;
	
	return;
  }
  
  void OverlayProducer::onFileClose(EventFile&) {
	
	return;
  }
  
  void OverlayProducer::onProcessStart() {
	if (verbosity_ > 2) {
	  std::cout << "[ OverlayProducer ]: onProcessStart() " << std::endl;
	}

      //based on the config parameters, set stuff up 
      // doing this here guarantess that the overlay file(s) gets set up regardless of whether we have an input file or not; onFileOpen gets called for both modes.

      rndm_  = std::make_unique<TRandom2>( seed_ );
      //      EventFile overlayFile_ (overlayFileName_);      


      overlayFile_ = std::make_unique<EventFile>( overlayFileName_ );
      overlayFile_->setupEvent( &overlayEvent_ );


      if (verbosity_ > 2) {
		std::cout << "[ OverlayProducer ]: onProcessStart () successful. Used input file: " << simFile_->getFileName() << std::endl;
		std::cout << "[ OverlayProducer ]: onProcessStart () successful. Got event info: " << std::endl;
		overlayFile_->getEvent()->Print( verbosity_ );
      }
	  
	  
	  return;
    }

    void OverlayProducer::onProcessEnd() {

        return;
    }

}

DECLARE_PRODUCER_NS(ldmx, OverlayProducer)
