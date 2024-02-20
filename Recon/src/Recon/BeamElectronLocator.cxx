#include "Recon/BeamElectronLocator.h"

namespace recon {

BeamElectronLocator::BeamElectronLocator(const std::string& name, framework::Process& process)
    : framework::Producer(name, process) {}

BeamElectronLocator::~BeamElectronLocator() {}

void BeamElectronLocator::configure(framework::config::Parameters &parameters) {
  inputColl_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputColl_ = parameters.getParameter<std::string>("output_collection");
  granularityXmm_ =
      parameters.getParameter<float>("granularity_X_mm");
  granularityYmm_ =
      parameters.getParameter<float>("granularity_Y_mm");
 tolerance_ =
      parameters.getParameter<float>("min_granularity_mm");
 

  ldmx_log(debug) << "BeamElectronLocator is using parameters: "
                  << " \n\tinput_collection = " << inputColl_
                  << " \n\tinput_pass_name = " << inputPassName_
                  << " \n\toutput_collection = " << outputColl_
                  << " \n\tgranularity_X_mm = " << granularityXmm_
                  << " \n\tgranularity_Y_mm = " << granularityYmm_
                  << " \n\tmin_granularity_mm = " << tolerance_ ;
}

  void BeamElectronLocator::produce(framework::Event &event) {
	//* steps
	//1. find those hits in the target sim hit collection that are associated with the electron
	//            -- this can be done using the existing truth hit producer 
	//2. figure out how to select only one location per electron
	//3. if using TS granularity, send to function binning them in X and Y, depending on choice in config
	//4. set the hit coordinates and add "nice-to-haves" like its momentum 
	//*/
	
	// this relies on having run the electron counter, 
	int nElectrons = event.getElectronCount();
  
	if (nElectrons < 0) {
	  ldmx_log(fatal)
		<< "Can't use unset number of counted electrons as electron count! "
		"Run the electron counter first!";
	  // TODO: throw an exeption instead
	  // TODO decide if this umber is actually used in some clustering, or remove 
	  return;

	}
  
   
	// Check if the input collection exists. If not,
	// don't bother processing the event.
	if (!event.exists(inputColl_, inputPassName_)) {
	  ldmx_log(fatal) << "Attemping to use non-existing input collection "
					  << inputColl_ << "_" << inputPassName_
					  << " to locate electrons! Exiting.";
	  return;
	}

	// the simhits have approximately infinite resolution. run some sort of grouping.
	// 


	std::vector<ldmx::BeamElectronTruth> beamElectronInfo;
	const auto simHits{event.getCollection<ldmx::SimCalorimeterHit>(
      inputColl_, inputPassName_)};

	for (const auto &simHit : simHits) {
	  //check if we already caught this position, else, add it 
	  bool isMatched = false;
	  std::vector<float> pos=simHit.getPosition();
	  for (auto foundElectrons : beamElectronInfo ) {
		//this check makes it square rather than a dR circle
		if ( fabs( pos[0] - foundElectrons.getX() )> tolerance_
			 && fabs( pos[1] - foundElectrons.getY() )> tolerance_ ) {
		  isMatched=true;
		} // if coordinates match something we already found 
	  }// over found electrons 
	  if (!isMatched) {		  
		ldmx::BeamElectronTruth electronInfo;
		electronInfo.setXYZ( pos[0], pos[1], pos[2]);
		//find a way to do this later
		//electronInfo.setThreeMomentum(simHit.getPx(), simHit.getPy(), simHit.getPz()); 
		beamElectronInfo.push_back(electronInfo);
	  }
	}// over simhits in the collection 
 
  
  //for these, i should 	
	//set up a 2d array, with some n number of bins set by the total width and granularity 


  }


  //rethink this... i want a grid with 0 and 1, simply. hit or no hit. 
  
  int BeamElectronLocator::bin(float coordinate, float binWidth, float min, float max) {
	// a function to discretize floating points to some precision 
	int n=0;
	while (coordinate < min + n*binWidth ) {
	  n++;
	  if  (n*binWidth > max) {
		//n--; // this moves the point back, but, perhaps better to see that we go out of bounds on return 
		break;
	  }
	}
	// the resulting coordinate is between n which is the lower edge, and the next 
	return n;
  }



  
}  // namespace recon    

DECLARE_PRODUCER_NS(recon, BeamElectronLocator)
