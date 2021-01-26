#include "TrigScint/TrigScintRecHitProducer.h"
#include "Framework/RandomNumberSeedService.h"
#include "Framework/Exception/Exception.h"

#include <iostream>

namespace ldmx {

  TrigScintRecHitProducer::TrigScintRecHitProducer(const std::string &name,
						   Process &process)
    : Producer(name, process) {}

  TrigScintRecHitProducer::~TrigScintRecHitProducer() {}

  void TrigScintRecHitProducer::configure(Parameters &parameters) {

    // Configure this instance of the producer
    pedestal_ = parameters.getParameter<double>("pedestal");
    gain_ = parameters.getParameter<double>("gain");
    mevPerMip_ = parameters.getParameter<double>("mev_per_mip");
    pePerMip_ = parameters.getParameter<double>("pe_per_mip");
    inputCollection_ = parameters.getParameter<std::string>("input_collection");
    inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
    outputCollection_ = parameters.getParameter<std::string>("output_collection");
    verbose_ = parameters.getParameter<bool>("verbose");

  }

  // }

  void TrigScintRecHitProducer::produce(Event &event) {

    // initialize QIE object for linearizing ADCs
    SimQIE qie;

    // looper over sim hits and aggregate energy depositions for each detID
    const auto digis{
      event.getCollection<TrigScintQIEDigis>(inputCollection_, inputPassName_)};

    std::vector<TrigScintHit> trigScintHits;
    for (const auto &digi : digis) {
    
      TrigScintHit hit;
      hit.setModuleID(0);
      hit.setBarID(digi.chanID);
      hit.setBeamEfrac(-1.);
      //std::cout << "Channel:" << digi.chanID << std::endl;
      //std::cout << "ADC:" << digi.adcs[1] << std::endl;
      //std::cout << "Charge:" << qie.ADC2Q( digi.adcs[1] ) << std::endl;
      //std::cout << "PEs:" << ( qie.ADC2Q( digi.adcs[1] ) - pedestal_ ) * 1e4 / gain_ << std::endl;
      //std::cout << "Energy:" << ( qie.ADC2Q( digi.adcs[1] ) - pedestal_ ) * 1e4 / gain_ * mevPerMip_ / pePerMip_ << std::endl;
      hit.setAmplitude( qie.ADC2Q( digi.adcs[1] ) ); // femptocoulombs
      if( digi.tdcs[1] > 49 )
	hit.setTime(-1);
      else
	hit.setTime(digi.tdcs[1]*0.5); 
      // hit.setEnergy( ( qie.ADC2Q( digi.adcs[1] ) - pedestal_ ) * 1e4 / gain_ * mevPerMip_ / pePerMip_ ); // MeV
      // hit.setPE( ( qie.ADC2Q( digi.adcs[1] ) - pedestal_ ) * 1e4 / gain_ ); 
      hit.setEnergy( ( qie.ADC2Q( digi.adcs[1] ) - pedestal_ ) * 6250 / gain_ * mevPerMip_ / pePerMip_ ); // MeV
      hit.setPE( ( qie.ADC2Q( digi.adcs[1] ) - pedestal_ ) * 6250 / gain_ ); 
      printf("\nTrigScintHit.pe_ %.3f",( qie.ADC2Q( digi.adcs[1] ) - pedestal_ ) * 1e4 / gain_);
      trigScintHits.push_back(hit);

    }
    // Create the container to hold the digitized trigger scintillator hits.

    event.add(outputCollection_, trigScintHits);
  }
} // namespace ldmx

DECLARE_PRODUCER_NS(ldmx, TrigScintRecHitProducer);
