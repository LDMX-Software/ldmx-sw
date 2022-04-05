/**
 * @file TestBeamHitProducer.cxx
 * @brief An producer drawing the most basic quanities of Trigger Scintillator bars
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/TestBeamHitProducer.h"

namespace trigscint {

  TestBeamHitProducer::TestBeamHitProducer(const std::string& name,
						   framework::Process& process)
    : Producer(name, process) {}
  TestBeamHitProducer::~TestBeamHitProducer() {}
  
  void TestBeamHitProducer::configure(framework::config::Parameters &parameters){

    inputCol_  = parameters.getParameter< std::string >("inputCollection");
    outputCollection_  = parameters.getParameter< std::string >("outputCollection");
    inputPassName_  = parameters.getParameter< std::string >("inputPassName");
    peds_  = parameters.getParameter< std::vector<double> >("pedestals");
    gain_  = parameters.getParameter< std::vector<double> >("gain");  //to do: vector
    startSample_  = parameters.getParameter< int >("startSample");
    pulseWidth_  = parameters.getParameter< int >("pulseWidth");
    pulseWidthLYSO_  = parameters.getParameter< int >("pulseWidthLYSO");
    nInstrumentedChannels_  = parameters.getParameter< int >("nInstrumentedChannels");
    doCleanHits_  = parameters.getParameter< bool >("doCleanHits");

    std::cout << " [ TestBeamHitProducer ] In configure(), got parameters " 
	      << "\n\t inputCollection = " << inputCol_
	      << "\n\t inputPassName = " << inputPassName_
	      << "\n\t outputCollection = " << outputCollection_
	      << "\n\t startSample = " << startSample_
	      << "\n\t pulseWidth = " << pulseWidth_
	      << "\n\t pulseWidthLYSO = " << pulseWidthLYSO_
			  << "\n\t gain[0] = " << gain_[0] // TODO: vector 
	      << "\n\t nInstrumentedChannels = " << nInstrumentedChannels_
	      << "\n\t doCleanHits = " << doCleanHits_
	      << "\n\t pedestals[0] = " << peds_[0]
	      << "\t." << std::endl;

    return;
  }

  void TestBeamHitProducer::produce( framework::Event &event) {


	/*
	  hit producer. 
	  sum up all charge within a certain time window. could work two ways: 
	      - find a time sample above a threshold or where TDC is 0-50 (0-2)
		       * this could allow for finding several pulses per event, just keep sliding along in time
			   -- in that case, record nPulses, startsample, pulsewidth for each hit 
		  - use a fixed start sample, and keep summing from there until nSamples is reached or all time samples used 

	  specifics:
		  - each channel has its own pedestal to subtract. also try using the first 5 samples to establish a threshold in the event. store both 
		  - store hit Q and hit PE conversion 
		  - for now use the same reconstruction paradigm for LYSO and plastic. two notes though
		    1. LYSO pulse looks wider, and might need wider window. plastic is fine with 5. start by 8 for LYSO (even if for large pulses, sometimes 10 seem to be needed). could probe this by correlating plastic and LYSO, and checking if at some amplitude (in plastic), we start cutting off charge in LYSO 
			2. there is a time offset between fibers. need to have a parameter fiber2offset to use for elecID >= 8 
			-- added this as a variable in EventReadout, along with fiber number. so this class doesn't need any detailed knowledge 

	  variables to write out:
	      - start sample
		  - pulse width
		  - n samples above threshold
		  - nPulses
		  - early pedestal (from first 5)
		  - assumed/average channel pedestal
		  - total Q 
		  - ped subtracted total Q 
		  - PE value 
		  - max amplitude in pulse
		  - store material assumption? isLYSO? 
	 */


	float mevPerMip = 0.3;
	float pePerMip = 100;
	  
    const auto channels{event.getCollection<trigscint::EventReadout >(inputCol_, inputPassName_)};

	int evNb = event.getEventNumber();
	int nChan = channels.size();
	std::vector<trigscint::TestBeamHit> hits;
	for (auto chan : channels) {
	  trigscint::TestBeamHit hit;
	  int bar = chan.getChanID();
	  if (bar >= nInstrumentedChannels_) //don't run hit reconstruction on junk signal
		continue;
	  int width=pulseWidth_;
	  if (bar %2 == 0) { //LYSO channel: allow for wider pulses
		width=pulseWidthLYSO_; // avoid hardwiring
	  }
	  hit.setPulseWidth(width);
      hit.setStartSample(startSample_);
	  float ped = peds_.at(bar); //chan.getPedestal() ;
	  float earlyPed = chan.getEarlyPedestal();
	  hit.setPedestal(ped);
	  hit.setEarlyPedestal(earlyPed);
	  int isClean=0; //false;
	  float threshold =fabs(ped); // 2*fabs(peds_[ bar ]); // or sth
	  if (doCleanHits_)
		threshold = 10*fabs(ped); // stricter cut

	  int startT = startSample_ + chan.getTimeOffset();
	  float maxQ = -999.;
	  int nSampAbovePed = 0;
	  int nSampAboveThr = 0;
	  float totSubtrQ = 0;
	  std::vector<float> q = chan.getQ();
	  // go to the start sample defined for this channel.
	  for (int iT = startT; iT < q.size() ; iT++) {
		ldmx_log(debug) << "in event " << evNb << "; channel " << bar << ", got charge[" << iT << "] = " << q.at(iT);
		// for the defined number of samples, subtract pedestal. if > 0, increment sample counter.
		float subQ=q.at(iT)-ped; //this might be addition, if ped is negative. should see this as channel dependence in nSampAbove 
		// once beyond nSamples, want to see how long positive threshold subtracted tail is --> increment sample counter in any case.
  		if (subQ > 0)
		  nSampAbovePed++;
		if (subQ > threshold)
		  nSampAboveThr++;
		if (iT - startT < width ) { //we're in the pulse integration window
		  if ( subQ > maxQ )  // keep track of max Q. this is the pulse amplitude
			maxQ = subQ; //q.at(iT);
		  if (subQ > 0)
			totSubtrQ+=subQ; 		// add positive subtracted Q to total pulse charge.
		}
		else if (subQ < 0 || q.at(iT) < 0 )   // if after the full pulse width, subQ < pedestal, break. special case to break at q=0 for cases where ped < 0
		  break;
		//done
	  }// over time samples 

	  // first check that hit passes any quality cuts
	  if (doCleanHits_) {
		if (maxQ/totSubtrQ < 2./3.) // skip "unnaturally" narrow hits 
		  if ( fabs(chan.getPedestal()) < 15 //threshold //   //skip events that have strange plateaus
			   //			   && (chan.getAvgQ()/chan.getPedestal()<0.8)  //skip events that have strange oscillations
			   && 1 < nSampAboveThr && nSampAboveThr < 10 ) // skip one-time sample flips and long weird pulses 
			isClean=1;
	  }

	  // set pulse properties like PE and amplitude
	  hit.setSampAbovePed(nSampAbovePed);
	  hit.setSampAboveThr(nSampAboveThr);
	  hit.setQ(totSubtrQ);
	  hit.setAmplitude(maxQ);
	  hit.setPE( totSubtrQ*6250./gain_[bar] );
	  hit.setHitQuality( isClean );	  
	  // set bar id. set moduleNB = 0
	  hit.setBarID( bar );	  
	  hit.setModuleID( 0 );	 // test beam 
	  //the rest are a little ill-defined for now (PE-energy conversion not known/different between LYSO and plastic)
	  hit.setTime(-999);//maybe?
	  hit.setBeamEfrac(-1.);
	  hit.setEnergy( hit.getPE()*mevPerMip/pePerMip );

	  //add hit
	  hits.push_back(hit);
	}// over channels
	
	// at end of event, write the collection of trigger scintillator hits.
  event.add(outputCollection_, hits);

		
    return;
  }



}

DECLARE_PRODUCER_NS(trigscint, TestBeamHitProducer)
