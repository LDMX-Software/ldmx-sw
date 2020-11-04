/**
 * @file OverlayProducer.h
 * @brief Class to overlay in-time pile-up events from an overlay file 
 * @author Lene Kristian Bryngemark, Stanford University 
 */

#ifndef EVENTPROC_OVERLAYPRODUCER_H
#define EVENTPROC_OVERLAYPRODUCER_H

//ROOT
#include "TFile.h"
#include "TRandom2.h"

// STL
#include <string>
#include <vector>
#include <map>

//LDMX Framework
#include "Event/EventDef.h" // Includes all possible event bus objects
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h" // Needed to declare processor
#include "Framework/Parameters.h" // Needed to import parameters from configuration file


namespace ldmx {
    
    /**
     * @class OverlayProducer
     * @brief 
     */
  class OverlayProducer : public ldmx::Producer {
  public:

	OverlayProducer(const std::string& name, ldmx::Process& process) : ldmx::Producer(name, process), overlayEvent_{"overlay"}{  }

	//	~OverlayProducer();
	//destructor causes linking problems 
	
	virtual void configure(Parameters& parameters) final override;

	virtual void produce(Event& event) final override;

	virtual void onProcessStart(); 

  private:


	/**
	 * Pileup overlay events input file name
	 */
	std::string overlayFileName_;

	/** 
	 * Pileup overlay events input file
	 */
	std::unique_ptr<EventFile> overlayFile_;      
	    
	/**
	 * The overlay ldmx event bus
	 */
	Event overlayEvent_;

	/**
	 * List of simhit collection(s) to loop over and add hits from, combining sim and pileup
	 */
	std::vector <std::string> collections_;     

	/**
	 * Pileup overlay events input pass name
	 */
	std::string overlayPassName_;   

	/**
	 * To use for finding the sim event bus passengers, mostly a disambiguation
	 */
	std::string simPassName_;
	
	/**
	 * Let the total number of events be poisson distributed, or fix at the chosen value, poissonMu_
	 */
	int doPoisson_{0};

	/**
	 * (average) total number of events 
	 */
	double poissonMu_{0.};

	/**
	 * Random number generator for number of events.
	 * TRandom2 slightly (~10%) faster than TRandom3; shorter period but our input files will have way shorter period anyway.
	 */
	std::unique_ptr<TRandom2> rndm_;

	/**
	 * Random number generator for peileup event time offset.
	 * TRandom2 slightly (~10%) faster than TRandom3; shorter period but our input files will have way shorter period anyway. 
	 */
	std::unique_ptr<TRandom2> rndmTime_;  

	/**
	 * Width of pileup bunch spread in time (in [ns]), specified as a sigma of a Gaussian distribution
	 */
	double timeSigma_{0.};

	/**
	 * Average position in time (in [ns]) of pileup bunches, relative to the sim event.
	 * Should realistically be 0. Using a non-zero mean and sigma = 0 is however useful for validation.
	 */
	double timeMean_{0.};

	/**
	 * Spacing in time (in [ns]) between electron bunches.
	 */
	double bunchSpacing_{0.};

	/**
	 * Number of bunches before and after the sim event to pull pileup events from 
	 * (0 --> all events occur in the same bunch as the sim event).
	 */
	int nBunchesToSample_{0};    

	/**
	 * Local control of processor verbosity 
	 */
	int verbosity_;

	/**
	 * For Ecal, overlay hits need to be added as contribs.
	 * But these are required to be unique, by the Ecal rconstruction code.
	 * So assign a nonsensical trackID, incidentID, and PDG ID to the contribs from overlay.
	 * These are hardwired right here.
	 */
	int overlayIncidentID_{ -1000 };
	int overlayTrackID_{ -1000 };
	int overlayPdgCode_{ 0 };
	
  };
}

#endif /* EVENTPROC_OVERLAYPRODUCER_H */
