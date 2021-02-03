#include "TrigScint/TrigScintQIEDigiProducer.h"
#include "Framework/RandomNumberSeedService.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Logger.h"

#include <iostream>

namespace trigscint {

  TrigScintQIEDigiProducer::TrigScintQIEDigiProducer
  (const std::string& name, framework::Process& process) :
    Producer(name, process) {    
  }

  TrigScintQIEDigiProducer::~TrigScintQIEDigiProducer() {
  }

  void TrigScintQIEDigiProducer::configure
  (framework::config::Parameters& parameters) {

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
    s_freq 		=
      parameters.getParameter< double >("qie_sf");
      
    if ( input_pulse_shape_ == "Expo") {
      pulse_params.clear();
      pulse_params.push_back
	(parameters.getParameter< double >("expo_k"));
      pulse_params.push_back
	(parameters.getParameter< double >("expo_tmax"));

      ldmx_log(debug) <<"expo_k ="<<	pulse_params[0];
      ldmx_log(debug) <<"expo_tmax ="<<	pulse_params[1];
    }

    // Debug mode: print parameter values.
    ldmx_log(debug) <<"maxts_ ="<<		maxts_;
    ldmx_log(debug) <<"toff_overall_ ="<<	toff_overall_;
    ldmx_log(debug) <<"input_pulse_shape_ ="<<input_pulse_shape_;
    ldmx_log(debug) <<"tdc_thr ="<<		tdc_thr;
    ldmx_log(debug) <<"pedestal ="<<		pedestal;
    ldmx_log(debug) <<"elec_noise ="<<	elec_noise;
    ldmx_log(debug) <<"sipm_gain ="<<		sipm_gain;
    ldmx_log(debug) <<"s_freq ="<<		s_freq;
  }

  void TrigScintQIEDigiProducer::produce(framework::Event& event) {

    // Need to handle seeding on the first event
    if (random_.get()==nullptr) {
      const auto& rseed = getCondition<framework::RandomNumberSeedService>
	(framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
      const auto& rseed2 =getCondition<framework::RandomNumberSeedService>
	(framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
        
      random_ =
	std::make_unique<TRandom3>(rseed.getSeed(outputCollection_));

      // Initialize SimQIE instance with
      // pedestal, electronic noise and the random seed
      smq = new SimQIE
	(pedestal,elec_noise,rseed2.getSeed(outputCollection_+"SimQIE"));

      smq->SetGain(sipm_gain);
      smq->SetFreq(s_freq);
      smq->SetNTimeSamples(maxts_);
      smq->SetTDCThreshold(tdc_thr);

    }

    // To simulate multiple pulses coming at different times, SiPMS
    float TrueEdep[stripsPerArray_];
    Expo* ex[stripsPerArray_]={0};
    for(int i=0;i<stripsPerArray_;i++){
      
      // Set the pulse shape with fixed parameters given by config. file
      ex[i] = new Expo(pulse_params[0],pulse_params[1]);
      TrueEdep[i]=0;
    }

    // loop over sim hits and aggregate energy depositions for each detID
    const auto simHits{event.getCollection< ldmx::SimCalorimeterHit >
	(inputCollection_,inputPassName_)};

    for (const auto& simHit : simHits) {

      ldmx::TrigScintID id(simHit.getID());

      ldmx_log(info) << "Processing sim hit with bar ID: "
		      << id.bar() ;

      // Simulating the noise corresponding to uncertainity in
      // detecting scintillating photons.
      // Poissonian distribution with mean = mean PEs generated
      double PulseAmp =
	random_->Poisson(simHit.getEdep() / mevPerMip_ * pePerMip_);

      // Adding a pulse for every sim hit recorded.
      // time offset = global offset+simhit time
      ex[id.bar()]->AddPulse(toff_overall_+simHit.getTime(),PulseAmp);

      // incrementing true energy deposited in appropriate bar.
      TrueEdep[id.bar()]+=simHit.getEdep();
    }

    // A container to hold the digitized trigger scintillator hits.
    std::vector<TrigScintQIEDigis> QDigis;

    double TotalNoise = meanNoise_*maxts_;

    // time period[ns] = 1000/sampling freq.[MHz]
    double SamplingTime = 1000/s_freq;

    // Loop over all the bars available.
    for(int bar_id=0;bar_id<stripsPerArray_;bar_id++) {

      // Dark current simulation
      // e-hole pairs may be generated at random times in SiPM
      // due to thermal fluctuations. 
      // Every e- thus generated, mimicks a Photo Electron.
      // Hence we will creat 1PE pulses for each electron generated.
      int n_noise_pulses = random_->Poisson(TotalNoise);
      for(int i=0;i<n_noise_pulses;i++) {
	ex[bar_id]->AddPulse
	  (random_->Uniform(0,maxts_*SamplingTime),1);
      }

      // Storing the "good" digis
      if(smq->PulseCut(ex[bar_id])) {
	TrigScintQIEDigis QIEInfo;
	QIEInfo.chanID = bar_id;

	QIEInfo.SetADC(smq->Out_ADC(ex[bar_id]));
	QIEInfo.SetTDC(smq->Out_TDC(ex[bar_id]));
	QIEInfo.SetCID(smq->CapID(ex[bar_id]));
	QIEInfo.truePE = TrueEdep[bar_id] / mevPerMip_ * pePerMip_;

	// true -> there is atleast some true energy deposited in the bar
	// false-> All the pulses recorded are from dark noise
	QIEInfo.IsNoisy= (TrueEdep[bar_id]==0);
	QDigis.push_back(QIEInfo);
      }
    }
    event.add(outputCollection_, QDigis);
  }

}

DECLARE_PRODUCER_NS(ldmx, TrigScintQIEDigiProducer);
