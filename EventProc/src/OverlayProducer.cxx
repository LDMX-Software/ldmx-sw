/**
 * @file OverlayProducer.cxx
 * @brief In-time pile-up producer  
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "EventProc/OverlayProducer.h"
#include "Framework/RandomNumberSeedService.h"

namespace ldmx {

    void OverlayProducer::configure(Parameters& parameters) {

      // Configure this instance of the producer
	  ldmx_log(debug) <<  "Running configure() " ;
	  
	  // name of file containing events to be overlaid, and a list of collections to overlay
      overlayFileName_     = parameters.getParameter< std::string >("overlayFileName");
      collections_         = parameters.getParameter< std::vector <std::string> >("overlayHitCollections");
      simPassName_         = parameters.getParameter< std::string >( "passName" );
      overlayPassName_     = parameters.getParameter< std::string >( "overlayPassName" );

	  // overlay specifics: 
      poissonMu_           = parameters.getParameter< double >("totalNumberOfInteractions");
      doPoisson_           = parameters.getParameter< int >("doPoisson");
      timeSigma_           = parameters.getParameter< double >("timeSpread");
      timeMean_            = parameters.getParameter< double >("timeMean");
	  nBunchesToSample_    = parameters.getParameter< int >("nBunchesToSample");
	  bunchSpacing_        = parameters.getParameter< double >("bunchSpacing");
	  
      verbosity_           = parameters.getParameter< int >("verbosity");

	  /// Print the parameters actually set. Helpful in case of typos.
      if (verbosity_) {
		 ldmx_log(info) <<  "Got parameters \n \t overlayFileName = " << overlayFileName_ 
				  << "\n\t sim pass name = " << simPassName_
				  << "\n\t overlay pass name = " << overlayPassName_
				  << "\n\t overlayHitCollections = " ;
		for ( const std::string &coll : collections_ )
		   ldmx_log(info) <<  coll << "; " ;

		ldmx_log(info) 
				  << "\n\t numberOverlaidInteractions = " << poissonMu_
				  << "\n\t doPoisson = " << doPoisson_
				  << "\n\t timeSpread = " << timeSigma_
				  << "\n\t timeMean = " << timeMean_
				  << "\n\t verbosity = " << verbosity_ ;
		
      }
	  
	  return;
    }
  
  void OverlayProducer::produce(Event& event) {
	//event is the incoming, simulated event/"hard" process
	//overlayEvent_ is the overlay producer's own event. 

	if (verbosity_ > 1) {
	   ldmx_log(info) <<  "produce() starts on simulation event " <<  event.getEventHeader().getEventNumber() ;
	}

	/// set up random seeds
	if (rndm_.get() == nullptr) {
	  //not been seeded yet, get it from RNSS
	  const auto& rnss = getCondition<RandomNumberSeedService>(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
	  rndm_ = std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndm"));
	}
	if (rndmTime_.get() == nullptr) {
	  //not been seeded yet, get it from RNSS
	  const auto& rnss = getCondition<RandomNumberSeedService>(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
	  rndmTime_ = std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndmTime"));
	}

	
	// sample a poisson distribution, or use a deterministic number of overlay events
	int nEvsOverlay = doPoisson_ ? (int) rndm_->Poisson( poissonMu_ ) : (int)poissonMu_ ;
	// the poisson samples the total number of events, which is nOverlay + 1 (since it includes the sim event)
	nEvsOverlay -=1; // now subtract the sim event from the poisson mu

	
	if (verbosity_ > 2) {
	   ldmx_log(debug) <<  "will overlay " <<  nEvsOverlay << " events on the simulated one" ;
	}
	
 	
	//get event wherever nextEvent()  left us 
	if (! &overlayEvent_ ) {
	  ldmx_log(error) << "No overlay event!" ;
	  return;
	}
	
	if (verbosity_ > 2) {
	   ldmx_log(debug) <<  "starting from overlay event " <<  overlayEvent_.getEventHeader().getEventNumber() ;
	}

      
	// using nextEvent to loop, we need to loop over overlay events and in an inner loop, loop over collections, and store them.
	// after all pileup events have been added, the vector of collections is iterated over and added to the event bus.
	
	std::vector< std::vector<SimCalorimeterHit> >  v_outHits;
	std::map< std::string, int > collectionMap;

	std::vector<SimCalorimeterHit> simHits;
	std::map< int, SimCalorimeterHit > hitMap;

	
	for (int iEv = 0 ; iEv < nEvsOverlay ; iEv++ ) {
		
	  if (verbosity_ > 2) {
		 ldmx_log(debug) <<  "in loop: overlaying event " <<  iEv + 1  << " out of " << nEvsOverlay ;
	  }
		
	  // an overlay event wide time offset to be applied to all its hits.
	  // TODO -- figure out if we should also randomly shift the time of the sim event (likely only needed if time bias gets picked up by BDT or ML by way of pulse behaviour)
	  float timeOffset = rndmTime_->Gaus( timeMean_, timeSigma_ );
	  int bunchOffset = (int) rndmTime_->Uniform( - (nBunchesToSample_+1), nBunchesToSample_+1 );  	  // +1 to get inclusive interval
	  float bunchTimeOffset = bunchSpacing_ * bunchOffset;
	  timeOffset += bunchTimeOffset;
	  
	  if (verbosity_ > 2) {
         ldmx_log(debug) <<  "hit time offset in event " <<  iEv + 1  << " is  " << timeOffset << " ns";
         ldmx_log(debug) <<  "bunch position offset in event " <<  iEv + 1  << " is  " << bunchOffset << ", leading to an additional time offset of " << bunchTimeOffset << " ns";
      }



	  // TODO: get this list of collections by listing what's there in the overlay file
	  // could we use drop/keep rules to skip anything we don't want? 
	  //   -- or would that have to happen at overlay file production time, since Process doesn't own this file?
	  // we *could* also use a listing from the sim event we're interested in.
	  //   -- in many ways easier? but worried about corner cases where a collection happens to be empty for a given event. 

	  
	  // get the collections that we want to overlay, by looping over the list of collections passed to the producer : collections_
	  for (uint iColl = 0; iColl < collections_.size(); iColl++) {
		  
		// for now, Ecal and only Ecal uses contribs instead of multiple SimHits per channel, meaning, it requires special treatment
		bool isEcalHitCollection = false ;
		if (strstr (collections_[iColl].c_str(), "Ecal") )
		  isEcalHitCollection = true ;

		
		std::vector<SimCalorimeterHit> outHits;
		std::vector<SimCalorimeterHit> overlayHits = overlayEvent_.getCollection<SimCalorimeterHit>( collections_[iColl], overlayPassName_ );
		//		std::vector<SimTrackerHit> overlayTrackerHits = overlayEvent_->getCollection<SimTrackerHit>( collections_[iColl] );


		//do something else if we're getting tracker hits. this type of check, for one, doesn't work, and simply leads to segfault if it's a simtracker hit
		/*
		if ( dynamic_cast<std::vector<SimCalorimeterHit>*>( &overlayHits  ) == NULL ) {
		  
		   ldmx_log(debug) <<  "Nope, no simcalorimeter hit collection called " << collections_[iColl] << ", skipping"  ;
		  continue;
		}
		else
		   ldmx_log(debug) <<  "It's simcalo alright" ;
		overlayHits[0].Print();

		*/


		std::string outCollName = collections_[iColl]+"Overlay";
		// if we alredy added at least one overlay event, this collection already exists in the output collection map
		// otherwise, start out by just copying the sim hits, unaltered. 
		if ( collectionMap.find(outCollName) == collectionMap.end() ) {
		  
		  simHits = event.getCollection<SimCalorimeterHit>( collections_[iColl], simPassName_ );
		  // but don't copy ecal hits immediately: for them, wait until overlay contribs have been added. then add everything through the hitmap
		  if (! isEcalHitCollection ) { 
			v_outHits.push_back(simHits);
			collectionMap[ outCollName ] = v_outHits.size() - 1;
		  }
		  
		  
		  if (verbosity_ > 2) {
			ldmx_log(debug) <<  "in loop: start of collection " << collections_[iColl] ;
			ldmx_log(debug) <<  "in loop: printing current sim event: "  ;
		  }
		  ldmx_log(debug) <<  "in loop: size of sim hits vector " << collections_[iColl] << " is " << simHits.size()  ;
		  
		  
		  //we don't need to touch the hard process sim hits, really... but we might need the simhits in the hit map.
		  if (isEcalHitCollection || verbosity_ > 2) {
			for (const SimCalorimeterHit &simHit : simHits ) {
			  
			  if (verbosity_ > 2)
				simHit.Print();
			  
			  //store ids of existing hits. for these, need to add contribs...
			  if (isEcalHitCollection) {
				// this copies the hit, its ID and its coordinates directly
				hitMap[ simHit.getID() ] = simHit; 
			  }
			  
			} //over calo simhit collection
		  }// if we need to enter this loop at all
		}// if output collection doesn't already exist (i.e., we're in the first overlay event)
		
		
		
		/* ------------- now do overlay ------------ */
		
		if (verbosity_ > 2) {
		  ldmx_log(debug) <<  "in loop: printing overlay event: "  ;
		}
		ldmx_log(debug) <<  "in loop: size of overlay hits vector is " << overlayHits.size()  ;
		
		for ( SimCalorimeterHit &overlayHit : overlayHits ) {
		  
		  if (verbosity_ > 2)
			overlayHit.Print();
		  
		  const float overlayTime = overlayHit.getTime() + timeOffset;
		  overlayHit.setTime( overlayTime ); 
		  
		  if ( isEcalHitCollection ) { //special treatment for ecal
			int overlayHitID = overlayHit.getID();
			if ( hitMap.find( overlayHitID ) == hitMap.end() ) { // there wasn't already a simhit in this id
			  hitMap[overlayHitID] = SimCalorimeterHit();
			  hitMap[ overlayHitID ].setID(overlayHitID);
			  std::vector <float> hitPos = overlayHit.getPosition();
			  hitMap[ overlayHitID ].setPosition( hitPos[0], hitPos[1], hitPos[2]);
			}
			// add the overlay hit (as a) contrib 
			// incidentID = -1000, trackID = -1000, pdgCode = 0  <-- these are set in the header for now but could be parameters
			hitMap[ overlayHitID ].addContrib(overlayIncidentID_,overlayTrackID_,overlayPdgCode_, overlayHit.getEdep(), overlayTime);
		  } // ecal hits
		  else {
			std::map<std::string, int>::iterator itr = collectionMap.find( outCollName );
			if (verbosity_ > 2)
			  ldmx_log(debug) <<  "Adding non-Ecal overlay hit to outhit vector at " << itr->second ;
			v_outHits[ itr->second ].push_back( overlayHit );
		  }
		} //over overlay calo simhit collection


		if ( ! isEcalHitCollection )
		  ldmx_log(debug) <<  "Nhits in overlay collection " << outCollName << ": " <<  v_outHits[ collectionMap.find( outCollName )->second ].size() ;
		
		
	  } // over collections 
	  
	  if (! overlayFile_->nextEvent() ) {
		ldmx_log(error) << "At sim event " << event.getEventHeader().getEventNumber() <<": couldn't read next overlay event!" ;
		return;
	  }
	  
	  
	} // over overlay events 

	
	//after all events are done, the hitmap is final and can be written to the event output
	for (uint iColl = 0; iColl < collections_.size(); iColl++) {
	  // just loop through once to find the right collection name

	  // add overlaid ecal hits as contribs/from hitmap rather than as copied simhits 
	  if (strstr (collections_[iColl].c_str(), "Ecal") ) {

		if (verbosity_ > 2)
		  ldmx_log(debug) <<  "Hits in hitmap after overlay of " << collections_[iColl] << "Overlay :" ;
		for ( auto &mapHit : hitMap ) {
		  if (verbosity_ > 2)
			mapHit.second.Print();

		  if ( collectionMap.find(collections_[iColl]+"Overlay") == collectionMap.end() ) {
			ldmx_log(debug) <<  "Adding first hit from hit map as first outhit vector to v_outHits";
			v_outHits.push_back( {mapHit.second} );
			collectionMap[ collections_[iColl]+"Overlay" ] = v_outHits.size() - 1;
		  }
		  else
			v_outHits.back().push_back( mapHit.second );
		}
		
	  }// isEcal
	}//second loop over collections, to collect hits from hitmap

	
	// done collecting hits. 

	// this should be added to the sim file, so to "event"
	for (auto &itr : collectionMap ) {

	  ldmx_log(debug) <<  "Writing " << itr.first << " to event bus" ;
	  if (verbosity_ > 2) {
		ldmx_log(debug) <<  "List of hits added: " ;
		for ( auto &hit : v_outHits[ itr.second ] )
		  hit.Print();
	  }

	  event.add( itr.first, v_outHits[ itr.second ]);
	}
	
	return;
  }
  
  
  void OverlayProducer::onProcessStart() {
	if (verbosity_ > 2) {
	  ldmx_log(debug) <<  "onProcessStart() " ;
	}

	// replace by this line once the corresponding tweak to EventFile is ready:
	//	overlayFile_ = std::make_unique<EventFile>( overlayFileName_, true );
	overlayFile_ = std::make_unique<EventFile>( overlayFileName_ );
	overlayFile_->setupEvent( &overlayEvent_ );

	// we update the iterator at the end of each event. so do this once here to grab the first event in the processor
	// TODO this could also be done N random times to get a randomness in which events get matched to what sim event.
	// noticed that shifting by a fair chunk helps remove some weak but suspicious correlations between sim and overlay particle positions.
	// leave it hardwired until this is better understood  
	int nEventsShift_ = 23; 

	for (int iShift = 0 ; iShift < nEventsShift_ ; iShift++) {
	  if (! overlayFile_->nextEvent() ) {
		std::cerr << "Couldn't read next event!" ;
		return;
	  }
	}
	
	if (verbosity_ > 2) {
	  ldmx_log(debug) <<  "onProcessStart () successful. Used input file: " <<overlayFile_->getFileName() ;
	  ldmx_log(debug) <<  "onProcessStart () successful. Got event info: " ;
	  overlayFile_->getEvent()->Print( verbosity_ );
	}
	  
	  
	return;
  }

}

DECLARE_PRODUCER_NS(ldmx, OverlayProducer)
