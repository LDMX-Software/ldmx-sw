/**
 * @file HcalDigiProducer.cxx
 * @brief Class that performs basic HCal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Framework/RandomNumberSeedService.h"
#include "Hcal/HcalDigiProducer.h"

namespace hcal {

HcalDigiProducer::HcalDigiProducer(const std::string& name,
				   framework::Process& process) :
  Producer(name, process) {
  // noise generator by default uses a Gausian model for noise
  //  i.e. It assumes the noise is distributed around a mean (setPedestal)
  //  with a certain RMS (setNoise) and then calculates
  //  how many hits should be generated for a given number of empty
  //  channels and a minimum readout value (setNoiseThreshold)
  noiseGenerator_ = std::make_unique<ldmx::NoiseGenerator>();
}

HcalDigiProducer::~HcalDigiProducer() { }
  
void HcalDigiProducer::configure(framework::config::Parameters& ps) {
  
  //settings of readout chip
  //  used  in actual digitization
  auto hgcrocParams = ps.getParameter<framework::config::Parameters>("hgcroc");
  hgcroc_ = std::make_unique<ldmx::HgcrocEmulator>(hgcrocParams);
  gain_ = hgcrocParams.getParameter<double>("gain");
  pedestal_ = hgcrocParams.getParameter<double>("pedestal");
  clockCycle_ = hgcrocParams.getParameter<double>("clockCycle");
  nADCs_ = hgcrocParams.getParameter<int>("nADCs");
  iSOI_ = hgcrocParams.getParameter<int>("iSOI");
  noise_ = hgcrocParams.getParameter<bool>("noise");
  
  //collection names
  inputCollName_  = ps.getParameter<std::string>("inputCollName");
  inputPassName_  = ps.getParameter<std::string>("inputPassName");
  digiCollName_   = ps.getParameter<std::string>("digiCollName");
  
  // physical constants
  //  used to calculate unit conversions
  MeV_ = ps.getParameter<double>("MeV");
  attlength_ = ps.getParameter<double>("attenuationLength");
  
  //Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024./clockCycle_;
  
  // Configure generator that will produce noise hits in empty channels
  readoutThreshold_ = hgcrocParams.getParameter<double>("readoutThreshold");
}
  
void HcalDigiProducer::produce(framework::Event& event) {
  
  // Get the Hcal Geometry
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  
  //Empty collection to be filled
  ldmx::HgcrocDigiCollection hcalDigis;
  hcalDigis.setNumSamplesPerDigi( nADCs_ ); 
  hcalDigis.setSampleOfInterestIndex( iSOI_ );
  
  std::set<unsigned int> filledDetIDs; //detector IDs that already have a hit in them
  
  /******************************************************************************************
   * HGCROC Emulation on Simulated Hits
   *****************************************************************************************/
  //get simulated hcal hits from Geant4
  //  the class HcalHitIO in the SimApplication module handles the translation from G4CalorimeterHits to SimCalorimeterHits
  //  this class ensures that only one SimCalorimeterHit is generated per cell, but
  //  multiple "contributions" are still handled within SimCalorimeterHit 
  auto hcalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(inputCollName_, inputPassName_)};
  
  /* debug printout
     std::cout << "Energy to Voltage Conversion: " << MeV_ << " mV/MeV" << std::endl;
  */
  
  for (auto const& simHit : hcalSimHits ) {

    std::vector<double> voltages, times;
    for ( int iContrib = 0; iContrib < simHit.getNumberOfContribs(); iContrib++ ) {
      ///* debug printout
      //if(simHit.getContrib(iContrib).edep > 3){
      //  std::cout << simHit.getContrib(iContrib).edep << " MeV" << std::endl;
      //}
      // */
      voltages.push_back( simHit.getContrib( iContrib ).edep*MeV_ );
      times.push_back( simHit.getContrib( iContrib ).time );
    }
    
    unsigned int hitID = simHit.getID();
    filledDetIDs.insert( hitID );
    
    ldmx::HcalID detID(hitID);
    int section = detID.section();
    int layer = detID.layer();
    int strip = detID.strip();
    
    // If back Hcal, then return two digis: close and far
    // the ``close'' digi will be less attenuated, less shifted.
    // the end of the digi is determined by its distance (x,y) along the bar ( 0 if positive (top, left), 1 if negative (bottom,right))
    if( section==ldmx::HcalID::HcalSection::BACK ){
      std::vector<float> position = simHit.getPosition();
      
      // amplitude attenuation
      float distance_along_bar = (layer % 2) ? position[0] : position[1];
      double half_total_width = hcalGeometry.halfTotalWidth();
      double attenuation_close = exp( -1. * ((half_total_width - fabs(distance_along_bar)) / 1000.) / attlength_);
      double attenuation_far   = exp( -1. * ((half_total_width + fabs(distance_along_bar)) / 1000.) / attlength_);
      
      // time shift
      // here is where Ralf's simulation should be included
      float v = 299.792/1.6; // velocity of light in Polystyrene, n = 1.6 = c/v mm/ns
      double shift_close = fabs((half_total_width - fabs(distance_along_bar)) / v);
      double shift_far = fabs((half_total_width + fabs(distance_along_bar)) / v);
      
      // digitize
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddClose;
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddFar;
      if ( hgcroc_->digitize( hitID , voltages , times , digiToAddClose, attenuation_close, shift_close) &&
	   hgcroc_->digitize( hitID , voltages , times , digiToAddFar, attenuation_far, shift_far) ) {
	int end_close = (distance_along_bar > 0) ? 0 : 1;
	ldmx::HcalDigiID closeID(section,layer,strip,end_close);
	hcalDigis.addDigi( closeID.raw() , digiToAddClose);
	
	int end_far = (distance_along_bar < 0) ? 0 : 1;
	ldmx::HcalDigiID farID(section,layer,strip,end_far);
	hcalDigis.addDigi( farID.raw() , digiToAddFar);
      } // need to digitize both or none (?)
	    }
    else{
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd;
      if ( hgcroc_->digitize( hitID , voltages , times , digiToAdd ) ) {
	ldmx::HcalDigiID digiID(section,layer,strip,0);
	hcalDigis.addDigi( digiID.raw() , digiToAdd );
      }
    }
  }
  
  /******************************************************************************************
   * Noise Simulation on Empty Channels
   * TOADD
   *****************************************************************************************/
  
  event.add( digiCollName_, hcalDigis );
  
  return;
} //produce

} //namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalDigiProducer);
