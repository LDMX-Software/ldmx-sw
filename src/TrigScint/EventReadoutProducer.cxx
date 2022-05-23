#include "Framework/Exception/Exception.h"
#include "TrigScint/EventReadoutProducer.h"

#include <iostream>

namespace trigscint {

EventReadoutProducer::EventReadoutProducer(const std::string &name,
                                                 framework::Process &process)
    : Producer(name, process) {}

EventReadoutProducer::~EventReadoutProducer() {}

void EventReadoutProducer::configure(
    framework::config::Parameters &parameters) {
  // Configure this instance of the producer
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  nPedSamples_ = parameters.getParameter<int>("number_pedestal_samples");
  timeShift_ = parameters.getParameter<int>("time_shift");
  fiberToShift_ = parameters.getParameter<int>("fiber_to_shift");
  verbose_ = parameters.getParameter<bool>("verbose");


  ldmx_log(debug) << "In configure, got parameters:" <<
	"\noutput_collection = " << outputCollection_ <<
	"\ninput_collection = " << inputCollection_ <<
	"\ninput_pass_name  = " << inputPassName_ <<
	"\nnumber_pedestal_samples  = " <<  nPedSamples_ <<
	"\ntime_shift = " <<  timeShift_ <<
	"\nfiber_to_shift  = " <<  fiberToShift_ <<
	"\nverbose          = " << verbose_ ;
  
}

void EventReadoutProducer::produce(framework::Event &event) {
  // initialize QIE object for linearizing ADCs
  SimQIE qie;
                                                                                               
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      inputCollection_, inputPassName_)};

  std::vector<trigscint::EventReadout> channelReadoutEvents;
  for (const auto &digi : digis) {
    trigscint::EventReadout outEvent;
    auto adc{digi.getADC()};
    auto tdc{digi.getTDC()};
	
	//copy over from qie digi for convenience
    outEvent.setChanID(digi.getChanID());
    outEvent.setElecID(digi.getElecID());
    outEvent.setTimeSinceSpill(digi.getTimeSinceSpill());
	// elecID increases monotonically with 8 channels per fiber
	outEvent.setFiberNb( (int)(digi.getElecID()/8) );  
	if ( outEvent.getFiberNb() == fiberToShift_ )
	  outEvent.setTimeOffset( timeShift_);

    outEvent.setADC(adc);
    outEvent.setTDC(tdc);
	std::vector <float> charge;
	std::vector <float> chargeErr;

	float avgQ = 0;
	float totPosQ = 0;
	int iS=0;
	int nPos=0;
	float earlyPed=0;
	for (auto& val: adc) {
	  float Q=qie.ADC2Q(val);
	  charge.push_back( Q );
	  chargeErr.push_back( qie.QErr(Q) );
	  avgQ+=Q; //charge.back();
	  if ( Q > 0 ) {
		totPosQ+=Q;
		nPos++;
	  }
	  if (verbose_)
	    ldmx_log(debug) << "got adc value " << val << " and charge " << Q; //qie.ADC2Q(val);
	  if (iS < nPedSamples_)
		earlyPed+=Q;
	  iS++;
	}
	outEvent.setQ(charge); //set in proper order before sorting 
	outEvent.setQError(chargeErr); //set in proper order before sorting
	earlyPed/=nPedSamples_;
	outEvent.setEarlyPedestal(earlyPed);

	// oscillation check. for this the pulse charge needs to be in order, so set this up now
	// the period is 4.
	// ansatz: an oscillation is a repeated shape. normalize to maxq=1, in every interval of 4 samples
	// if after normalization the same numbers are repeated, it's an oscillation
	// edge case: multiple single PE peaks with that repetition. we probably don't need to keep those anyway 
	if (verbose_)
	  ldmx_log(debug) << "going into oscillations check ";
	std::vector <float> chargeCheck = {NULL};
	float minCharge=10;  //no point in looking at oscillations just around the pedestal. nearest edge is 10.35 fC  //ADC=0 corresponds to -16 fC. 
	for (int i=3; i<charge.size()-4; i++) {
	  float maxSamp=minCharge; 
	  for (int iQ=0; iQ<4; iQ++) { //find the local max in the 4 samples
		//		ldmx_log(debug) << "got charge " << charge[i+iQ];	
		if (charge[i+iQ] > maxSamp)
		  maxSamp=charge[i+iQ];
	  }
	  if (verbose_)
		ldmx_log(debug) << "got max charge " << maxSamp;
	  for (int iQ=0; iQ<4; iQ++) //store the locally normalized numbers. even if the period is 5 this should work for a while
		chargeCheck.push_back(charge[i+iQ]/maxSamp);
	  i+=3; //to increment by 4, do 3 here and 1 in the loop 
	}
	
	
	//now calculate the pedestal as the average of the middle half of the sorted vector 
	float ped = 0;
	std::sort(charge.begin(), charge.end());
	int pedLength=(int)charge.size()/5; //actually pulse can be up to 12 samples = 2/5*30
	int pedOffset=2; //but still skip the lowest (and highest) few 
	//	for (int i = pedLength; i < 3*pedLength ; i++) { //use 2nd and third 5th 
	for (int i = pedOffset; i < 2*pedLength+pedOffset ; i++) { //use 1st and 2nd 5th 
	  ped+=charge[i];
	}
	ped/=2*pedLength;
	//median: technically only true for odd number of elements but good enough 
	float medQ=charge[ (int)charge.size()/2 ];
	float minQ=charge[0];
	float maxQ=charge[charge.size() -1];

	outEvent.setTotQ(totPosQ);//-nPos*ped); //store (event) ped subtracted total charge, before dividing by N  --> actually, ped subtraction makes it confusing 
	//	outEvent.setTotQ(totPosQ-adc.size()*ped); //store (event) ped subtracted total charge, before dividing by N 
	//	outEvent.setTotQ(avgQ-adc.size()*ped); //store (event) ped subtracted total charge, before dividing by N 
	avgQ/=adc.size();
	outEvent.setPedestal(ped);
	outEvent.setAvgQ(avgQ);
	outEvent.setMedQ(medQ);
	outEvent.setMinQ(minQ);
	outEvent.setMaxQ(maxQ);

	//and the noise as the RMSE of that, same interval as pedestal 
	float diffSq = 0;
	//    for (int i = pedLength; i < 3*pedLength ; i++) {
    for (int i = pedOffset; i < 2*pedLength+pedOffset ; i++) {
	  diffSq+=(charge[i]-ped)*(charge[i]-ped);
	}
	diffSq/=2*pedLength; //adc.size(); 
	outEvent.setNoise( sqrt(diffSq) );

	//oscillation check 
	uint flagOscillation = 0;
	//no need to run tedious oscillation check for all-neg channels 
	if (maxQ > minCharge) {
	  int maxID=0; 
	  //find the first occurence of a local max
	  for (int i=0; i<chargeCheck.size()-4; i++) {
		if (chargeCheck[i]==1) {  //an actual local max has been normalised by its own value
		  maxID=i;
		  //		  ldmx_log(debug) << "storing max index " <<maxID <<" and size of vector is " << chargeCheck.size()-4;
		  break;
		}
	  }
	  int lastMatchSample=0;
	  //start from local max
	  bool doBreak=false;
	  for (int i=maxID; i<chargeCheck.size()-4; i++) {
		if (verbose_)
		ldmx_log(debug) << "Checking how many matching groups of four we can find, starting at index " << i;
		
		for (int iQ=0; iQ<4; iQ++) { //check if they are consistently close
		  if (verbose_)
			ldmx_log(debug) << "Comparing " << chargeCheck[i+iQ] << " (sample "<< i+iQ << ") to " << chargeCheck[i+4+iQ] << " (sample "<< i+4+iQ << "), ratio is " << chargeCheck[i+iQ]/chargeCheck[i+4+iQ];
		  //we can be generous in these crietira since we will require an unbroken suite of 8 matches to call it oscillation 
		  if ( fabs(chargeCheck[i+iQ]/chargeCheck[i+4+iQ]-1) <0.5 || //need this tolerance to be kind of large, most actual peaks won't pass it by far anyway.
			   (chargeCheck[i+4+iQ]<0.01 && fabs(chargeCheck[i+iQ]/chargeCheck[i+4+iQ]) <5 ) ) // for very small numbers, one ADC difference can be a factor 3 so add some margin 
			lastMatchSample=i+iQ;
		  else {
			if (verbose_)
			  ldmx_log(debug) << "Oscillation check for channel " << digi.getChanID() << " breaking at time sample " << i+iQ;
			doBreak=true; //break outer loop 
			break; //break this loop
		}
		}
		if (doBreak) {
		  break;
		}
		if (verbose_)
		  ldmx_log(debug) << "Current lastMatchSample " << lastMatchSample;
		if (lastMatchSample-maxID>=2*4 ) { //we had at least a couple of oscillations (2nd period was fully matched by third)
		  flagOscillation = 1;  //there is another check possible later too, commented for now
		  break; //we've seen what we need to see
		}
		i+=3;
	  }
	} //if positive maxQ 
  
	
	// //use the top and bottom ends of the sorted q as another oscillation catcher: we don't expect that the top values will be high and basically identical unless they are from an oscillation
	int nHigh=0;
	int quartLength=(int)charge.size()/4;
	for (int i = 4*quartLength-2; i >= 3*quartLength ; i--) { //maxQ is already at last index
	   if (charge[i]/maxQ > 0.66)
		 nHigh++;
	}
	  
	
	uint flagSpike = (maxQ/outEvent.getTotQ() > 0.9 ) || (charge[charge.size()-2]/maxQ < 0.25) ; // skip "unnaturally" narrow hits (all charge in one sample or huge drop to second highest)
	uint flagPlateau = ( ped>15 || nHigh >= 5); //( fabs(ped) > 15 ); //threshold //   //skip events that have strange plateaus   
	uint flagLongPulse = 0; //easier to deal with in hit reconstruction directly. copy channel flags to hit flags and add this one there
	uint flagNoise= (outEvent.getNoise() > 3.5 || outEvent.getNoise()==0);  // =0 is typically from funky events but could be too harsh maybe 
	/* //let this wait for now 
	//if we have many high counts, a small event pedestal (where they weren't included), and an avgQ ~ maxQ/nHigh, then this is an oscillation
	if ( (quartLength-nHigh<2 && ped<10) || fabs( maxQ/(avgQ*nHigh)-1 ) <0.1 )
	  flagOscillation=1;
*/

	
	/* //this seems to not catch what we want it to
if ( fabs(chan.getPedestal()) < 15 //threshold //   //skip events that have strange plateaus                                                    
               //              && (chan.getAvgQ()/chan.getPedestal()<0.8)  //skip events that have strange oscillations                                   
               && 1 < nSampAboveThr && nSampAboveThr < 10 ) // skip one-time sample flips and long weird pulses                                           
	*/
  uint flag=flagSpike+2*flagPlateau+4*flagLongPulse+8*flagOscillation+16*flagNoise;
  ldmx_log(debug) << "Got quality flag "  << flag << " made up of (spike/plateau/long pulse/oscillation/noise) " << flagSpike << "+" << flagPlateau << "+" << flagLongPulse << "+" << flagOscillation  << "+" << flagNoise ;
  outEvent.setQualityFlag( flag );

	
	//	if (ped > 15 )
	//  continue;
	ldmx_log(debug) << "In event "  << event.getEventHeader().getEventNumber() <<
	  ", set pedestal = " << outEvent.getPedestal() << //ped <<
	  " fC, noise = " << outEvent.getNoise() << " fC for channel "<< 
	  outEvent.getChanID();
	
    channelReadoutEvents.push_back( outEvent );
  }
  // Create the container to hold the
  // digitized trigger scintillator hits.

  event.add(outputCollection_, channelReadoutEvents);
  ldmx_log(debug) << "\n";
}
}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, EventReadoutProducer);
