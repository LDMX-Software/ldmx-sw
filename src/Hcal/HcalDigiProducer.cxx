/**
 * @file HcalDigiProducer.cxx
 * @brief Class that performs basic HCal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Hcal/HcalDigiProducer.h"

#include "Framework/RandomNumberSeedService.h"

namespace hcal {

HcalDigiProducer::HcalDigiProducer(const std::string& name,
                                   framework::Process& process)
    : Producer(name, process) {
  /*
   * Noise generator by default uses a Gausian model for noise
   * i.e. It assumes the noise is distributed around a mean (setPedestal)
   * with a certain RMS (setNoise) and then calculates
   * how many hits should be generated for a given number of empty
   * channels and a minimum readout value (setNoiseThreshold)
   */
  noiseGenerator_ = std::make_unique<ldmx::NoiseGenerator>();
}

void HcalDigiProducer::configure(framework::config::Parameters& ps) {
  // settings of readout chip
  //  used  in actual digitization
  auto hgcrocParams = ps.getParameter<framework::config::Parameters>("hgcroc");
  hgcroc_ = std::make_unique<ldmx::HgcrocEmulator>(hgcrocParams);
  gain_ = hgcrocParams.getParameter<double>("gain");
  pedestal_ = hgcrocParams.getParameter<double>("pedestal");
  clockCycle_ = hgcrocParams.getParameter<double>("clockCycle");
  nADCs_ = hgcrocParams.getParameter<int>("nADCs");
  iSOI_ = hgcrocParams.getParameter<int>("iSOI");
  noise_ = hgcrocParams.getParameter<bool>("noise");

  // collection names
  inputCollName_ = ps.getParameter<std::string>("inputCollName");
  inputPassName_ = ps.getParameter<std::string>("inputPassName");
  digiCollName_ = ps.getParameter<std::string>("digiCollName");

  // physical constants
  //  used to calculate unit conversions
  MeV_ = ps.getParameter<double>("MeV");
  attlength_ = ps.getParameter<double>("attenuationLength");

  // Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024. / clockCycle_;

  // Configure generator that will produce noise hits in empty channels
  readoutThreshold_ = hgcrocParams.getParameter<double>("readoutThreshold");
}

void HcalDigiProducer::produce(framework::Event& event) {
  // Handle seeding on the first event
  if (!noiseGenerator_->hasSeed()) {
    const auto& rseed = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    noiseGenerator_->seedGenerator(
        rseed.getSeed("HcalDigiProducer::NoiseGenerator"));
  }
  if (noiseInjector_.get() == nullptr) {
    const auto& rseed = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    noiseInjector_ = std::make_unique<TRandom3>(
        rseed.getSeed("HcalDigiProducer::NoiseInjector"));
  }
  if (!hgcroc_->hasSeed()) {
    const auto& rseed = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    hgcroc_->seedGenerator(rseed.getSeed("HcalDigiProducer::HgcrocEmulator"));
  }

  // Get the Hcal Geometry
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  // Empty collection to be filled
  ldmx::HgcrocDigiCollection hcalDigis;
  hcalDigis.setNumSamplesPerDigi(nADCs_);
  hcalDigis.setSampleOfInterestIndex(iSOI_);

  std::set<unsigned int>
      filledDetIDs;  // detector IDs that already have a hit in them

  /******************************************************************************************
   * HGCROC Emulation on Simulated Hits
   *****************************************************************************************/
  // get simulated hcal hits from Geant4
  //  the class HcalHitIO in the SimApplication module handles the translation
  //  from G4CalorimeterHits to SimCalorimeterHits this class ensures that only
  //  one SimCalorimeterHit is generated per cell, but multiple "contributions"
  //  are still handled within SimCalorimeterHit
  auto hcalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(
      inputCollName_, inputPassName_)};

  for (auto const& simHit : hcalSimHits) {
    // get ID
    unsigned int hitID = simHit.getID();
    filledDetIDs.insert(hitID);

    ldmx::HcalID detID(hitID);
    int section = detID.section();
    int layer = detID.layer();
    int strip = detID.strip();

    // get position
    std::vector<float> position = simHit.getPosition();
    auto positionMap = hcalGeometry.getStripCenterPosition(detID);

    // get voltages and times
    std::vector<double> voltages, times;
    std::vector<double> voltages_far, times_far;
    for (int iContrib = 0; iContrib < simHit.getNumberOfContribs();
         iContrib++) {
      double voltage = simHit.getContrib(iContrib).edep * MeV_;
      double time =
          simHit.getContrib(iContrib).time;  // global time (t=0ns at target)
      // time += position.at(2) / 299.702547; // shift light-speed particle
      // traveling along z

      /* Attenuate voltages and shift time if double-readout.
       * Return two digis: close and far.
       * The close digi will be less attenuated, less shifted.
       * The end of the digi is determined by its distance (x,y) along the bar.
       *  a positive end (top, left) will have end = 0.
       *  a negative end (bottom, right) will have end = 1.
       */
      if (section == ldmx::HcalID::HcalSection::BACK) {
        double half_total_width = hcalGeometry.getHalfTotalWidth(section);
        float distance_along_bar = (layer % 2) ? position[0] : position[1];
        double attenuation_close =
            exp(-1. * ((half_total_width - fabs(distance_along_bar)) / 1000.) /
                attlength_);
        double attenuation_far =
            exp(-1. * ((half_total_width + fabs(distance_along_bar)) / 1000.) /
                attlength_);

        float v = 299.792 /
                  1.6;  // velocity of light in Polystyrene, n = 1.6 = c/v mm/ns
        double shift_close =
            fabs((half_total_width - fabs(distance_along_bar)) / v);
        double shift_far =
            fabs((half_total_width + fabs(distance_along_bar)) / v);

        voltages.push_back(voltage * attenuation_close);
        times.push_back(time + shift_close + 50.);
        voltages_far.push_back(voltage * attenuation_far);
        times_far.push_back(time + shift_far + 50.);
      } else {
        voltages.push_back(voltage);
        times.push_back(time);
      }
    }

    // digitize
    if (section == ldmx::HcalID::HcalSection::BACK) {
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddClose;
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddFar;
      if (hgcroc_->digitize(hitID, voltages, times, digiToAddClose) &&
          hgcroc_->digitize(hitID, voltages_far, times_far, digiToAddFar)) {
        float distance_along_bar = (layer % 2) ? position[0] : position[1];
        int end_close = (distance_along_bar > 0) ? 0 : 1;
        int end_far = (distance_along_bar < 0) ? 0 : 1;

        ldmx::HcalDigiID closeID(section, layer, strip, end_close);
        ldmx::HcalDigiID farID(section, layer, strip, end_far);

        hcalDigis.addDigi(closeID.raw(), digiToAddClose);
        hcalDigis.addDigi(farID.raw(), digiToAddFar);
      }  // need to digitize both or none
    } else {
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd;
      if (hgcroc_->digitize(hitID, voltages, times, digiToAdd)) {
        ldmx::HcalDigiID digiID(section, layer, strip, 0);
        hcalDigis.addDigi(digiID.raw(), digiToAdd);
      }
    }
  }

  /******************************************************************************************
   * Noise Simulation on Empty Channels
   *****************************************************************************************/
  if (noise_) {
    int numChannels = 0;
    for (int l = 0; l < hcalGeometry.getNumSections(); l++) {
      int nChannels =
          hcalGeometry.getNumLayers(l) * hcalGeometry.getNumStrips(l);
      if (l == 0) nChannels *= 2;
      numChannels += nChannels;
    }
    int numEmptyChannels = numChannels - hcalDigis.getNumDigis();
    // noise generator gives us a list of noise amplitudes [mV] that randomly
    // populate the empty channels and are above the readout threshold
    auto noiseHitAmplitudes{
        noiseGenerator_->generateNoiseHits(numEmptyChannels)};
    std::vector<double> voltages(1, 0.), times(1, 0.);
    for (double noiseHit : noiseHitAmplitudes) {
      // generate detector ID for noise hit
      // making sure that it is in an empty channel
      unsigned int noiseID;
      do {
        int sectionID = noiseInjector_->Integer(hcalGeometry.getNumSections());
        int layerID = noiseInjector_->Integer(hcalGeometry.getNumLayers(0));
        int stripID = noiseInjector_->Integer(hcalGeometry.getNumStrips(0));
        auto detID = ldmx::HcalDigiID(sectionID, layerID, stripID, 0);
        noiseID = detID.raw();
      } while (filledDetIDs.find(noiseID) != filledDetIDs.end());
      filledDetIDs.insert(noiseID);

      // get a time for this noise hit
      times[0] = noiseInjector_->Uniform(clockCycle_);

      // noise generator gives the amplitude above the readout threshold
      // we need to convert it to the amplitdue above the pedestal
      voltages[0] = noiseHit + gain_ * readoutThreshold_ - gain_ * pedestal_;

      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd;
      if (hgcroc_->digitize(noiseID, voltages, times, digiToAdd)) {
        hcalDigis.addDigi(noiseID, digiToAdd);
      }
    }  // loop over noise amplitudes
  }    // if we should add noise

  event.add(digiCollName_, hcalDigis);

  return;
}  // produce

}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalDigiProducer);
