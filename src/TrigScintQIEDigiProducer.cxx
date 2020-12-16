#include "TrigScint/TrigScintQIEDigiProducer.h"
#include "Framework/RandomNumberSeedService.h"
#include "Framework/Exception.h"

#include <iostream>

namespace ldmx {

  TrigScintQIEDigiProducer::TrigScintQIEDigiProducer
  (const std::string& name, Process& process) :
    Producer(name, process) {    
  }

  TrigScintQIEDigiProducer::~TrigScintQIEDigiProducer() {
  }

  void TrigScintQIEDigiProducer::configure(Parameters& parameters) {

    // Configure this instance of the producer
    stripsPerArray_   =
      parameters.getParameter< int >("number_of_strips");
    numberOfArrays_   =
      parameters.getParameter< int >("number_of_arrays");
    meanNoise_        =
      parameters.getParameter< double >("mean_noise");
    mevPerMip_        =
      parameters.getParameter< double >("mev_per_mip");
    pePerMip_         =
      parameters.getParameter< double >("pe_per_mip");
    inputCollection_  =
      parameters.getParameter< std::string >("input_collection");
    inputPassName_    =
      parameters.getParameter< std::string >("input_pass_name" );
    outputCollection_ =
      parameters.getParameter< std::string >("output_collection");
    verbose_          =
      parameters.getParameter< bool >("verbose");

    // QIE specific parameters initialization
    maxts_ 	      	=
      parameters.getParameter< int >("maxts");
    toff_overall_     	=
      parameters.getParameter< double >("toff_overall");
    input_pulse_shape_ 	=
      parameters.getParameter< std::string >("input_pulse_shape");
    tdc_thr 		=
      parameters.getParameter< double >("tdc_thr");
    pedestal 		=
      parameters.getParameter< double >("pedestal");
    elec_noise 		=
      parameters.getParameter< double >("elec_noise");
    sipm_gain 		=
      parameters.getParameter< double >("sipm_gain");
      
    if ( input_pulse_shape_ == "Expo") {
      pulse_params.clear();
      pulse_params.push_back
	(parameters.getParameter< double >("expo_k"));
      pulse_params.push_back
	(parameters.getParameter< double >("expo_tmax"));
    }

    noiseGenerator_ =std::make_unique<NoiseGenerator>(meanNoise_, false);

  }

  TrigScintID TrigScintQIEDigiProducer::generateRandomID(int module) {

    TrigScintID tempID(module,random_->Integer(stripsPerArray_));
    if ( module >= TrigScintSection::NUM_SECTIONS ) {
      // Throw an exception
      std::cout<<"WARNING [TrigScintQIEDigiProducer::generateRandomID]: "
	       <<"TrigScintSection is not known"
	       <<std::endl;
    }

    return tempID;
  }

  void TrigScintQIEDigiProducer::produce(Event& event) {

    // Need to handle seeding on the first event
    if (!noiseGenerator_->hasSeed()) {
      const auto& rseed = getCondition<RandomNumberSeedService>
	(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
        
      random_ =
	std::make_unique<TRandom3>(rseed.getSeed(outputCollection_));
      noiseGenerator_->setNoiseThreshold(1);
      (RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
      noiseGenerator_->seedGenerator(rseed.getSeed(outputCollection_
						   +"NoiseGenerator"));
      const auto& rseed2 =
	getCondition<RandomNumberSeedService>
	(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
      smq = new SimQIE
	(pedestal,elec_noise,rseed2.getSeed(outputCollection_+"SimQIE"));
      smq->SetGain(sipm_gain);
      smq->SetFreq();
      smq->SetNTimeSamples(maxts_);
      smq->SetTDCThreshold(tdc_thr);
    }

    std::map<TrigScintID, int> cellPEs;
    std::map<TrigScintID, float> Edep;
    std::set<TrigScintID> noiseHitIDs;

    // To simulate multiple pulses coming at different times, SiPMS
    float TrueEdep[stripsPerArray_];
    Expo* ex[stripsPerArray_]={0};
    for(int i=0;i<stripsPerArray_;i++){
      ex[i] = new Expo(pulse_params[0],pulse_params[1]);
      TrueEdep[i]=0;
    }
    auto numRecHits{0};

    // loop over sim hits and aggregate energy depositions for each detID
    const auto simHits{event.getCollection< SimCalorimeterHit >
	(inputCollection_,inputPassName_)};
    auto particleMap{event.getMap< int, SimParticle >("SimParticles")};

    int module{-1};

    for (const auto& simHit : simHits) {

      TrigScintID id(simHit.getID());

      // set module ID for Edep.
      module = id.module();
      std::vector<float> position = simHit.getPosition();

      if (verbose_) {
	std::cout << id << std::endl;
      }

      if (Edep.find(id) == Edep.end()) {

      	// first hit, initialize
      	Edep[id] = simHit.getEdep();
      	numRecHits++;
      }
      else {

      	// not first hit, aggregate, and store the largest radius hit
      	Edep[id] += simHit.getEdep();
      }

      double PulseAmp =
	random_->Poisson(simHit.getEdep() / mevPerMip_ * pePerMip_);
      ex[id.bar()]->AddPulse(toff_overall_+simHit.getTime(),PulseAmp);
      TrueEdep[id.bar()]+=simHit.getEdep();
    }

    // A container to hold the digitized trigger scintillator hits.
    std::vector<TrigScintQIEDigis> QDigis;

    double TotalNoise = meanNoise_*maxts_;
    double SamplingTime = 25;	// 1/sampling freq.
    
    for(int bar_id=0;bar_id<stripsPerArray_;bar_id++) {

      // Dark current simulation
      int n_noise_pulses = random_->Poisson(TotalNoise);
      	for(int i=0;i<n_noise_pulses;i++) {
      	  ex[bar_id]->AddPulse
	    (random_->Uniform(0,maxts_*SamplingTime),1);
      	}

      // Storing the "good" digis
      if(smq->PulseCut(ex[bar_id])) {
	TrigScintQIEDigis QIEInfo(maxts_);
	QIEInfo.chanID = bar_id;

	QIEInfo.SetADC(smq->Out_ADC(ex[bar_id]));
	QIEInfo.SetTDC(smq->Out_TDC(ex[bar_id]));
	QIEInfo.SetCID(smq->CapID(ex[bar_id]));
	QIEInfo.truePE = TrueEdep[bar_id] / mevPerMip_ * pePerMip_;

	QIEInfo.IsNoisy= (TrueEdep[bar_id]==0);
	// QIEInfo.IsNoisy= (ex[bar_id]->npulses==n_noise_pulses);
	QDigis.push_back(QIEInfo);
      }
    }
    event.add(outputCollection_, QDigis);
  }

}

DECLARE_PRODUCER_NS(ldmx, TrigScintQIEDigiProducer);
