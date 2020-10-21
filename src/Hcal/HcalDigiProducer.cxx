#include "TString.h"
#include "TFile.h"
#include "TTree.h"

#include "Hcal/HcalDigiProducer.h"
#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"

#include <iostream>
#include <exception>

namespace ldmx {

HcalDigiProducer::HcalDigiProducer(const std::string& name, Process& process) :
    Producer(name, process)
{
}

void HcalDigiProducer::configure(Parameters& parameters) {
  STRIPS_BACK_PER_LAYER_     = parameters.getParameter< int >("strips_back_per_layer");
  NUM_BACK_HCAL_LAYERS_      = parameters.getParameter< int >("num_back_hcal_layers");
  STRIPS_SIDE_TB_PER_LAYER_  = parameters.getParameter< int >("strips_side_tb_per_layer");
  NUM_SIDE_TB_HCAL_LAYERS_   = parameters.getParameter< int >("num_side_tb_hcal_layers");
  STRIPS_SIDE_LR_PER_LAYER_  = parameters.getParameter< int >("strips_side_lr_per_layer");
  NUM_SIDE_LR_HCAL_LAYERS_   = parameters.getParameter< int >("num_side_lr_hcal_layers");
  SUPER_STRIP_SIZE_          = parameters.getParameter< int >("super_strip_size");
  readoutThreshold_          = parameters.getParameter< int >("readoutThreshold");
  meanNoise_                 = parameters.getParameter< double >("meanNoise");
  mev_per_mip_               = parameters.getParameter< double >("mev_per_mip");
  pe_per_mip_                = parameters.getParameter< double >("pe_per_mip");
  strip_attenuation_length_  = parameters.getParameter< double >("strip_attenuation_length");
  strip_position_resolution_ = parameters.getParameter< double >("strip_position_resolution");
  sim_hit_pass_name_         = parameters.getParameter< std::string >("sim_hit_pass_name");
  noiseGenerator_ = std::make_unique<NoiseGenerator>(meanNoise_,false);
  noiseGenerator_->setNoiseThreshold(1); // hard-code this number, create noise hits for non-zero PEs! 
}

HcalID HcalDigiProducer::generateRandomID(HcalID::HcalSection sec){
  int layer, strip;
  HcalID::HcalSection section=sec;
  if( sec == HcalID::BACK ){
    layer=random_->Integer(NUM_BACK_HCAL_LAYERS_);
    strip=random_->Integer(STRIPS_BACK_PER_LAYER_/SUPER_STRIP_SIZE_);
  }else if( sec == HcalID::TOP || sec == HcalID::BOTTOM ){
    layer=random_->Integer(NUM_SIDE_TB_HCAL_LAYERS_);
    section=HcalID::HcalSection(random_->Integer(2)+1);
    strip=random_->Integer(STRIPS_SIDE_TB_PER_LAYER_);            
  }else if( sec == HcalID::LEFT || sec == HcalID::RIGHT ){
    layer=random_->Integer(NUM_SIDE_LR_HCAL_LAYERS_);
    section=HcalID::HcalSection(random_->Integer(2)+3);
    strip=random_->Integer(STRIPS_SIDE_LR_PER_LAYER_);            
  }else
    std::cout << "WARNING [HcalDigiProducer::generateRandomID]: HcalSection is not known" << std::endl;

  return HcalID(section, layer, strip);
}

void HcalDigiProducer::constructNoiseHit(std::vector<HcalHit> &hcalRecHits, HcalID::HcalSection section, double total_noise, double min_noise, 
                                         const std::map<unsigned int, float>& hcaldetIDEdep,std::unordered_set<unsigned int>& noiseHitIDs)
{
  HcalHit noiseHit;
  noiseHit.setPE(total_noise);
  noiseHit.setMinPE(min_noise); 
  noiseHit.setAmplitude(total_noise);
  noiseHit.setXPos(0.);
  noiseHit.setYPos(0.);
  noiseHit.setZPos(0.);
  noiseHit.setTime(-999.);
  noiseHit.setEnergy(total_noise*mev_per_mip_/pe_per_mip_);

  unsigned int rawID;
  do {rawID = generateRandomID(section).raw();}
  while( hcaldetIDEdep.find(rawID) != hcaldetIDEdep.end() || noiseHitIDs.find(rawID) != noiseHitIDs.end() );

  noiseHit.setID(rawID);
  noiseHitIDs.insert(rawID);
  noiseHit.setNoise(true);

  hcalRecHits.push_back( noiseHit );
}




void HcalDigiProducer::produce(Event& event)
{

  // Need to handle seeding on the first event
  if (!noiseGenerator_->hasSeed()) {
    const RandomNumberSeedService& rseed = getCondition<RandomNumberSeedService>(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    noiseGenerator_->seedGenerator(rseed.getSeed("HcalDigiProducer::NoiseGenerator"));
  }
  if (random_.get()==nullptr) {
    const RandomNumberSeedService& rseed = getCondition<RandomNumberSeedService>(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    random_ = std::make_unique<TRandom3>(rseed.getSeed("HcalDigiProducer"));
  }

  std::map<unsigned int, int>   hcalLayerPEs;
  std::map<unsigned int, int>   hcalLayerMinPEs;
  std::map<unsigned int, float> hcalXpos,hcalYpos,hcalZpos,hcaldetIDEdep, hcaldetIDTime;
  std::unordered_set<unsigned int> noiseHitIDs;
  int numSigHits_back=0,numSigHits_side_tb=0,numSigHits_side_lr=0;


  float side_hcal_z0(215.0f);
  float strip_width(50.0f);
  float super_strip_width  = SUPER_STRIP_SIZE_*strip_width;
  float total_super_strips = STRIPS_BACK_PER_LAYER_/SUPER_STRIP_SIZE_; // to help the integer round up
  float half_total_width   = STRIPS_BACK_PER_LAYER_*strip_width/2.0f;


  // first check if the super strip size divides nicely into the total number of strips
  if (STRIPS_BACK_PER_LAYER_ % SUPER_STRIP_SIZE_ != 0){
    EXCEPTION_RAISE( "InvalidArg" , 
                     "The specified superstrip size is not compatible with the total number of strips! (Number of strips is not divisible by super strip size)" );
  }

  // looper over sim hits and aggregate energy depositions for each detID
  auto hcalHits{event.getCollection<SimCalorimeterHit>(EventConstants::HCAL_SIM_HITS,sim_hit_pass_name_)};

  for (const SimCalorimeterHit &simHit : hcalHits ) {

    int detIDraw = simHit.getID();
    HcalID detID(detIDraw);
    int layer = detID.layer();
    int subsection = detID.section();
    int strip = detID.strip();
    std::vector<float> position = simHit.getPosition();       

    if (verbose_) {
      std::cout << detID << std::endl;
    }

    // re-assign the strip number based on super strip size -- ONLY FOR Back Hcal
    if (SUPER_STRIP_SIZE_ != 1 && subsection == 0){
      int newstrip = strip/SUPER_STRIP_SIZE_;
      detID=HcalID(detID.section(),detID.layer(),newstrip);
      detIDraw = detID.raw();
    }

    // for now, we take an energy weighted average of the hit in each stip to simulate the hit position. 
    // will use strip TOF and light yield between strips to estimate position.            
    if (hcaldetIDEdep.find(detIDraw) == hcaldetIDEdep.end()){
      // first hit, initialize
      hcaldetIDEdep[detIDraw] = simHit.getEdep();
      hcaldetIDTime[detIDraw] = simHit.getTime() * simHit.getEdep();
      hcalXpos[detIDraw]      = position[0]* simHit.getEdep();
      hcalYpos[detIDraw]      = position[1]* simHit.getEdep();
      hcalZpos[detIDraw]      = position[2]* simHit.getEdep();
    } else {
      // not first hit, aggregate, and store the largest radius hit
      hcalXpos[detIDraw]      += position[0]* simHit.getEdep();
      hcalYpos[detIDraw]      += position[1]* simHit.getEdep();
      hcalZpos[detIDraw]      += position[2]* simHit.getEdep();
      hcaldetIDEdep[detIDraw] += simHit.getEdep();
      hcaldetIDTime[detIDraw] += simHit.getTime() * simHit.getEdep();
    }

  }

  // loop over detIDs and simulate number of PEs
  std::vector<HcalHit> hcalRecHits;
  for (std::map<unsigned int, float>::iterator it = hcaldetIDEdep.begin(); it != hcaldetIDEdep.end(); ++it) {
    int detIDraw = it->first;
    double depEnergy = hcaldetIDEdep[detIDraw];
    hcaldetIDTime[detIDraw] = hcaldetIDTime[detIDraw] / hcaldetIDEdep[detIDraw];
    hcalXpos[detIDraw]      = hcalXpos[detIDraw] / hcaldetIDEdep[detIDraw];
    hcalYpos[detIDraw]      = hcalYpos[detIDraw] / hcaldetIDEdep[detIDraw];
    hcalZpos[detIDraw]      = hcalZpos[detIDraw] / hcaldetIDEdep[detIDraw];
    double meanPE           = depEnergy / mev_per_mip_ * pe_per_mip_;

    HcalID curDetId(detIDraw);

    int cur_subsection = curDetId.section();            
    int cur_layer      = curDetId.layer();
    int cur_strip      = curDetId.strip();

    if (curDetId.getSection() == HcalID::BACK)
      numSigHits_back++;
    else if (curDetId.getSection() == HcalID::TOP  || curDetId.getSection() == HcalID::BOTTOM)
      numSigHits_side_tb++;
    else if (curDetId.getSection() == HcalID::LEFT || curDetId.getSection() == HcalID::RIGHT)
      numSigHits_side_lr++;
    else std::cout << "WARNING [HcalDigiProducer::produce]: HcalSection is not known" << std::endl;

    // need to add in a weighting factor eventually, so keep it that way to make sure we don't forget about it
    double energy = depEnergy; 

    // quantize/smear the position            
    float cur_xpos(hcalXpos[detIDraw]),cur_ypos(hcalYpos[detIDraw]),cur_zpos(hcalZpos[detIDraw]); 

    // for back HCal, get PEs with attentuation
    if (cur_subsection == 0) {
      float distance_along_bar = (cur_layer % 2) ? fabs(cur_xpos) : fabs(cur_ypos);                

      // increase the PE count to the case with no attentuation (assuming 80% attenuation on the pe_per_mip number @ 1m)
      meanPE *= exp(1./strip_attenuation_length_); 

      float meanPE_close = meanPE * exp( -1. * ((half_total_width - distance_along_bar) / 1000.) / strip_attenuation_length_ );
      float meanPE_far   = meanPE * exp( -1. * ((half_total_width + distance_along_bar) / 1000.) / strip_attenuation_length_ );
      float PE_close     = random_->Poisson(meanPE_close+meanNoise_);
      float PE_far       = random_->Poisson(meanPE_far+meanNoise_);
      hcalLayerPEs[detIDraw] = PE_close + PE_far;
      hcalLayerMinPEs[detIDraw] = std::min(PE_close,PE_far);

      if (cur_layer % 2 == 0){ // even layers, vertical
        cur_xpos = (super_strip_width * (float(cur_strip)+0.5)) - half_total_width; 
        cur_ypos = hcalYpos[detIDraw] + random_->Gaus(0.,strip_position_resolution_); 
      }
      if (cur_layer % 2 == 1){ // odd layers, horizontal
        cur_ypos = (super_strip_width * (float(cur_strip)+0.5)) - half_total_width; 
        cur_xpos = hcalXpos[detIDraw] + random_->Gaus(0.,strip_position_resolution_); 
      }                
      cur_xpos = std::max(std::min(cur_xpos,half_total_width),-half_total_width);
      cur_ypos = std::max(std::min(cur_ypos,half_total_width),-half_total_width);                

      //This would be the quantized z position. 
      //The back_hcal_z0 and back_hcal_layer_thickness values must be derived fromn the geometry!
      //float back_hcal_z0(552);
      //float back_hcal_layer_thickness(44.0);
      //cur_zpos = back_hcal_z0+(cur_layer-1)*back_hcal_layer_thickness;                            
    }            
    // for sidecal don't worry about attenuation because it's single readout
    else {

      hcalLayerPEs[detIDraw]    = int(meanPE+meanNoise_); //random_->Poisson(meanPE+meanNoise_);
      hcalLayerMinPEs[detIDraw] = hcalLayerPEs[detIDraw];            

      // It looks like LEFT / RIGHT are inverted ?!? LEFT should be + and RIGHT -
      // The gdml file is wrong, left and right are indeed inverted (x,y coodrinates should be reversed). 
      // need to fic gdml and this part 

      //Note the side Hcal doesn't have super strips
      //This is the quantized position along the length of the bar - LEFT/RIGHT is for fixed HCAL geometry
      // float ecal_width_(525);
      //if (cur_subsection == HcalID::TOP)    cur_xpos =  half_total_width/2.0 - ecal_width/4.0;
      //if (cur_subsection == HcalID::BOTTOM) cur_xpos = -half_total_width/2.0 + ecal_width/4.0;
      //if (cur_subsection == HcalID::LEFT)   cur_ypos =  half_total_width/2.0 - ecal_width/4.0;
      //if (cur_subsection == HcalID::RIGHT)  cur_ypos = -half_total_width/2.0 + ecal_width/4.0;

      //This would be the quantized z position. The side_hcal_z0 value must be derived fromn the geometry!
      //float side_hcal_z0(215.5);
      //cur_zpos = side_hcal_z0+(cur_strip+0.5)*strip_width ;                        

      //This is the quantized position along the thickness of the bar - check RIGHT / LEFT
      //float back_hcal_layer_thickness(39);
      //float side_hcal_xy_offset(294);
      //if (cur_subsection == HcalID::TOP)    cur_ypos =  side_hcal_xy_offset+(cur_layer-1)*back_hcal_layer_thickness;
      //if (cur_subsection == HcalID::BOTTOM) cur_ypos = -side_hcal_xy_offset-(cur_layer-1)*back_hcal_layer_thickness;
      //if (cur_subsection == HcalID::LEFT)   cur_xpos = -side_hcal_xy_offset-(cur_layer-1)*back_hcal_layer_thickness;
      //if (cur_subsection == HcalID::RIGHT)  cur_xpos =  side_hcal_xy_offset+(cur_layer-1)*back_hcal_layer_thickness;               
    }            

    if (hcalLayerPEs[detIDraw] >= readoutThreshold_ ) {                
      HcalHit hit;
      hit.setID(detIDraw);
      hit.setPE(hcalLayerPEs[detIDraw]);
      hit.setMinPE(hcalLayerMinPEs[detIDraw]);
      hit.setAmplitude(hcalLayerPEs[detIDraw]);
      hit.setEnergy(energy);
      hit.setTime(hcaldetIDTime[detIDraw]);
      hit.setXPos(cur_xpos); // quantized and smeared positions
      hit.setYPos(cur_ypos); // quantized and smeared positions
      hit.setZPos(cur_zpos);
      hit.setNoise(false);

      hcalRecHits.push_back( hit );
    }

    if (verbose_) {
      HcalID detID(detIDraw);

      int layer      = detID.layer();
      int subsection = detID.section();
      int strip      = detID.strip();

      std::cout << "detID     : " << detIDraw << std::endl;
      std::cout << "Layer     : " << layer << std::endl;
      std::cout << "Subsection: " << subsection << std::endl;
      std::cout << "Strip: " << strip << std::endl;
      std::cout << "Edep: " << hcaldetIDEdep[detIDraw] << std::endl;
      std::cout << "numPEs: " << hcalLayerPEs[detIDraw] << std::endl;
      std::cout << "time: " << hcaldetIDTime[detIDraw] << std::endl;
      std::cout << "z: " << hcalZpos[detIDraw] << std::endl;
      std::cout << "Layer: " << layer 
                << "\t Strip: " << strip 
                << "\t X: " << hcalXpos[detIDraw] 
                << "\t Y: " << hcalYpos[detIDraw] 
                << "\t Z: " << hcalZpos[detIDraw] << std::endl;
    }// end verbose            
  } //end loop over map of values


  // ------------------------------- Noise simulation -------------------------------
  // simulate noise hits in back hcal
  int total_super_strips_back      = STRIPS_BACK_PER_LAYER_/SUPER_STRIP_SIZE_;
  int total_empty_channels         = 2*(total_super_strips_back*NUM_BACK_HCAL_LAYERS_-numSigHits_back);
  std::vector<double> noiseHits_PE = noiseGenerator_->generateNoiseHits( total_empty_channels ); // 2-sided readout
  int total_zero_channels          = total_empty_channels - noiseHits_PE.size();

  std::vector<double> zeroNoiseHits_PE(total_zero_channels, 0.0);
  noiseHits_PE.insert( noiseHits_PE.end(), zeroNoiseHits_PE.begin(), zeroNoiseHits_PE.end() );
  std::random_shuffle ( noiseHits_PE.begin(), noiseHits_PE.end() );
  int ctr_back_noise = 0;
  for (unsigned i = 0; i < noiseHits_PE.size()/2 ; ++i) {
    double cur_noise_pe_1 = noiseHits_PE[i*2];
    double cur_noise_pe_2 = noiseHits_PE[i*2+1];
    double total_noise    = cur_noise_pe_1 + cur_noise_pe_2;
    if (total_noise < readoutThreshold_) continue; 

    double min_noise = std::min(cur_noise_pe_1,cur_noise_pe_2);
    constructNoiseHit(hcalRecHits,HcalID::BACK,total_noise,min_noise,hcaldetIDEdep,noiseHitIDs);
    ctr_back_noise++;

  }
  if (verbose_) std::cout << "numSigHits_back = " << numSigHits_back << ", ctr_back_noise = " << ctr_back_noise << std::endl;

  // simulate noise hits in side, top / bottom hcal
  noiseHits_PE = noiseGenerator_->generateNoiseHits((STRIPS_SIDE_TB_PER_LAYER_*NUM_SIDE_TB_HCAL_LAYERS_)*2-numSigHits_side_tb);
  for (auto noise : noiseHits_PE ) {
    constructNoiseHit(hcalRecHits,HcalID::TOP,noise,noise,hcaldetIDEdep,noiseHitIDs);
    constructNoiseHit(hcalRecHits,HcalID::BOTTOM,noise,noise,hcaldetIDEdep,noiseHitIDs);
  }

  // simulate noise hits in side, left / right hcal
  noiseHits_PE = noiseGenerator_->generateNoiseHits((STRIPS_SIDE_LR_PER_LAYER_*NUM_SIDE_LR_HCAL_LAYERS_)*2-numSigHits_side_lr);
  for (auto noise : noiseHits_PE ) {
    constructNoiseHit(hcalRecHits,HcalID::LEFT,noise,noise,hcaldetIDEdep,noiseHitIDs);
    constructNoiseHit(hcalRecHits,HcalID::RIGHT,noise,noise,hcaldetIDEdep,noiseHitIDs);
  }

  event.add( "HcalRecHits", hcalRecHits );
}

}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);

