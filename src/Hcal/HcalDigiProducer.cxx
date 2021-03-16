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
  noiseGenerator_->setNoise(
      hgcrocParams.getParameter<double>("noiseRMS"));  // rms noise in mV
  noiseGenerator_->setPedestal(
      gain_ * pedestal_);  // mean noise amplitude (if using Gaussian Model for
                           // the noise) in mV
  noiseGenerator_->setNoiseThreshold(
      gain_ * readoutThreshold_);  // threshold for readout in mV
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
    double half_total_width = hcalGeometry.getHalfTotalWidth(section);
    double ecal_dx = hcalGeometry.getEcalDx();
    double ecal_dy = hcalGeometry.getEcalDy();

    // find position along the bar
    float distance_along_bar, distance_ecal;
    if (section == ldmx::HcalID::HcalSection::BACK)
      distance_along_bar = (layer % 2) ? position[0] : position[1];
    if ((section == ldmx::HcalID::HcalSection::TOP) ||
        ((section == ldmx::HcalID::HcalSection::BOTTOM))) {
      distance_along_bar = position[0];
      distance_ecal = ecal_dx;
    } else if ((section == ldmx::HcalID::HcalSection::LEFT) ||
               (section == ldmx::HcalID::HcalSection::RIGHT)) {
      distance_along_bar = position[1];
      distance_ecal = ecal_dy;
    }

    /**
       Define end of the bar for the HcalDigiID, based on its distance (x,y)
       along the bar. A positive end corresponds to top,left, will have an
       endID=0. A negative end corresponds to bottom,right, will have an
       endID=1.

       - if the position along the bar > 0, the close end is 0, else 1.
       - if the position along the bar > 0, the far end is 1, else 0.
    */
    int end_close = (distance_along_bar > 0) ? 0 : 1;
    int end_far = (distance_along_bar < 0) ? 0 : 1;

    /**
       Calculate voltage attenuation and time shift for the close and far pulse.
       For the back Hcal, the half point of the bar coincides with the
       coordinates of the origin. For the side Hcal, the length of the bar is:
       - 2 *(half_width) - Ecal_dx(y) away from one end and,
       - Ecal_dx(y) away from the other.
     */
    float v =
        299.792 / 1.6;  // velocity of light in Polystyrene, n = 1.6 = c/v mm/ns
    float distance_close, distance_far;
    if (section == ldmx::HcalID::HcalSection::BACK) {
      distance_close = half_total_width;
      distance_far = half_total_width;
    } else if ((section == ldmx::HcalID::HcalSection::TOP) ||
               (section == ldmx::HcalID::HcalSection::LEFT)) {
      if (end_close == 0) {
        distance_close = 2 * half_total_width - distance_ecal / 2;
        distance_far = distance_ecal / 2;
      } else {
        distance_close = distance_ecal / 2;
        distance_far = 2 * half_total_width - distance_ecal / 2;
      }
    } else if ((section == ldmx::HcalID::HcalSection::BOTTOM) ||
               ((section == ldmx::HcalID::HcalSection::RIGHT))) {
      if (end_close == 0) {
        distance_close = distance_ecal / 2;
        distance_far = 2 * half_total_width - distance_ecal / 2;
      } else {
        distance_far = distance_ecal / 2;
        distance_close = 2 * half_total_width - distance_ecal / 2;
      }
    }
    double attenuation_close =
        exp(-1. * ((distance_close - fabs(distance_along_bar)) / 1000.) /
            attlength_);
    double attenuation_far = exp(
        -1. * ((distance_far + fabs(distance_along_bar)) / 1000.) / attlength_);
    double shift_close = fabs((distance_close - fabs(distance_along_bar)) / v);
    double shift_far = fabs((distance_far + fabs(distance_along_bar)) / v);

    // Get voltages and times.
    std::vector<double> voltages_close, times_close;
    std::vector<double> voltages_far, times_far;
    for (int iContrib = 0; iContrib < simHit.getNumberOfContribs();
         iContrib++) {
      double voltage = simHit.getContrib(iContrib).edep * MeV_;
      double time =
          simHit.getContrib(iContrib).time;  // global time (t=0ns at target)
      // time += position.at(2) / 299.702547; // shift light-speed particle
      // traveling along z

      voltages_close.push_back(voltage * attenuation_close);
      times_close.push_back(time + shift_close + 50.);
      voltages_far.push_back(voltage * attenuation_far);
      times_far.push_back(time + shift_far + 50.);
    }

    /**
     * Digitize:
     * For back Hcal return two digis: close and far.
     * For bide Hcal we choose which pulse (close or far) to readout based on
     * the position of the hit. For Top (Left)
     *  - x(y) > 0: read close pulse
     *  - x(y) < 0: read far pulse
     * For bottom (right)
     *  - x(y) > 0: read far pulse
     *  - x(y) < 0: read close pulse
     **/
    if (section == ldmx::HcalID::HcalSection::BACK) {
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddClose;
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddFar;
      if (hgcroc_->digitize(hitID, voltages_close, times_close,
                            digiToAddClose) &&
          hgcroc_->digitize(hitID, voltages_far, times_far, digiToAddFar)) {
        ldmx::HcalDigiID closeID(section, layer, strip, end_close);
        ldmx::HcalDigiID farID(section, layer, strip, end_far);

        hcalDigis.addDigi(closeID.raw(), digiToAddClose);
        hcalDigis.addDigi(farID.raw(), digiToAddFar);
      }  // Back Hcal needs to digitize both pulses or none
    } else {
      bool is_close = false;
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd;
      if ((section == ldmx::HcalID::HcalSection::TOP) ||
          (section == ldmx::HcalID::HcalSection::LEFT)) {
        if (distance_along_bar > 0) is_close = true;
      } else if ((section == ldmx::HcalID::HcalSection::BOTTOM) ||
                 (section == ldmx::HcalID::HcalSection::RIGHT)) {
        if (distance_along_bar < 0) is_close = true;
      }
      if (is_close) {
        if (hgcroc_->digitize(hitID, voltages_close, times_close, digiToAdd)) {
          ldmx::HcalDigiID digiID(section, layer, strip, end_close);
          hcalDigis.addDigi(digiID.raw(), digiToAdd);
        }
      } else {
        if (hgcroc_->digitize(hitID, voltages_far, times_far, digiToAdd)) {
          ldmx::HcalDigiID digiID(section, layer, strip, end_far);
          hcalDigis.addDigi(digiID.raw(), digiToAdd);
        }
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
      // for back Hcal we have double readout, therefore we multiply the number
      // of channels by 2.
      // if (l == 0) nChannels *= 2;
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
      int sectionID, layerID, stripID, endID;
      do {
        sectionID = noiseInjector_->Integer(hcalGeometry.getNumSections());
        layerID = noiseInjector_->Integer(hcalGeometry.getNumLayers(sectionID));
        // set layer to 1 if the generator says it is 0 (geometry map starts
        // from 1)
        if (layerID == 0) layerID = 1;
        stripID = noiseInjector_->Integer(hcalGeometry.getNumStrips(sectionID));
        endID = noiseInjector_->Integer(2);
        if ((sectionID == ldmx::HcalID::HcalSection::TOP) ||
            (sectionID == ldmx::HcalID::HcalSection::LEFT)) {
          endID = 0;
        } else if ((sectionID == ldmx::HcalID::HcalSection::BOTTOM) ||
                   (sectionID == ldmx::HcalID::HcalSection::RIGHT)) {
          endID = 1;
        }
        auto detID = ldmx::HcalDigiID(sectionID, layerID, stripID, endID);
        noiseID = detID.raw();
      } while (filledDetIDs.find(noiseID) != filledDetIDs.end());
      filledDetIDs.insert(noiseID);

      // get a time for this noise hit
      times[0] = noiseInjector_->Uniform(clockCycle_);

      // noise generator gives the amplitude above the readout threshold
      // we need to convert it to the amplitude above the pedestal
      voltages[0] = noiseHit + gain_ * readoutThreshold_ - gain_ * pedestal_;

      if (sectionID == ldmx::HcalID::HcalSection::BACK) {
        std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd_close,
            digiToAdd_far;
        int endID_opp = (endID == 0) ? 1 : 0;
        auto detID_opp =
            ldmx::HcalDigiID(sectionID, layerID, stripID, endID_opp);
        if (hgcroc_->digitize(noiseID, voltages, times, digiToAdd_close) &&
            hgcroc_->digitize(detID_opp.raw(), voltages, times,
                              digiToAdd_far)) {
          hcalDigis.addDigi(noiseID, digiToAdd_close);
          hcalDigis.addDigi(detID_opp.raw(), digiToAdd_far);
        }
      } else {
        std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd;
        if (hgcroc_->digitize(noiseID, voltages, times, digiToAdd)) {
          hcalDigis.addDigi(noiseID, digiToAdd);
        }
      }
    }  // loop over noise amplitudes
  }    // if we should add noise

  event.add(digiCollName_, hcalDigis);

  return;
}  // produce

}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalDigiProducer);
