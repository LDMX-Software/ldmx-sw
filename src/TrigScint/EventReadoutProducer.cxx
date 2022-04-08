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
	//now calculate the pedestal as the average of the middle half of the sorted vector 
	float ped = 0;
	std::sort(charge.begin(), charge.end());
	int quartLength=(int)charge.size()/4;
	for (int i = quartLength; i < 3*quartLength ; i++) {
	  ped+=charge[i];
	}
	ped/=2*quartLength;
	//median: technically only true for odd number of elements but good enough 
	float medQ=charge[ (int)charge.size()/2 ];
	float minQ=charge[0];
	float maxQ=charge[charge.size() -1];

	outEvent.setTotQ(totPosQ-nPos*ped); //store (event) ped subtracted total charge, before dividing by N 
	//	outEvent.setTotQ(totPosQ-adc.size()*ped); //store (event) ped subtracted total charge, before dividing by N 
	//	outEvent.setTotQ(avgQ-adc.size()*ped); //store (event) ped subtracted total charge, before dividing by N 
	avgQ/=adc.size();
	outEvent.setPedestal(ped);
	outEvent.setAvgQ(avgQ);
	outEvent.setMedQ(medQ);
	outEvent.setMinQ(minQ);
	outEvent.setMaxQ(maxQ);


	//and the noise as the RMSE of that 
	float diffSq = 0;
    for (int i = quartLength; i < 3*quartLength ; i++) {
	  //	for (auto& val: charge)	{
	  diffSq+=(charge[i]-ped)*(charge[i]-ped);
	}
	diffSq/=2*quartLength; //adc.size(); 
	outEvent.setNoise( sqrt(diffSq) );

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
