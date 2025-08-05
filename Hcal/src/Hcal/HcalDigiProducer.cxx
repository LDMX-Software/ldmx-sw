/**
 * @file HcalDigiProducer.cxx
 * @brief Class that performs basic HCal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 * @author Tamas Almos Vami, UCSB
 */

#include "Hcal/HcalDigiProducer.h"

namespace hcal {

HcalDigiProducer::HcalDigiProducer(const std::string& name,
                                   framework::Process& process)
    : Producer(name, process) {
  /*
   * Noise generator by default uses a Gausian model for noise
   * i.e. It assumes the noise is distributed around a mean (setPedestal)
   * with a certain RMS (setNoise) and then calculates
   * how many hits_ should be generated for a given number of empty
   * channels and a minimum readout value (setNoiseThreshold)
   */
  noise_generator_ = std::make_unique<ldmx::NoiseGenerator>();
}

void HcalDigiProducer::configure(framework::config::Parameters& ps) {
  // settings of readout chip
  //  used  in actual digitization
  auto hgcrocParams = ps.getParameter<framework::config::Parameters>("hgcroc");
  hgcroc_ = std::make_unique<ldmx::HgcrocEmulator>(hgcrocParams);
  clock_cycle_ = hgcrocParams.getParameter<double>("clockCycle");
  nADCs_ = hgcrocParams.getParameter<int>("nADCs");
  iSOI_ = hgcrocParams.getParameter<int>("iSOI");
  noise_ = hgcrocParams.getParameter<bool>("noise");

  // If true, ignore readout threshold
  // and generate pedestal noise digis in every empty channel
  zeroSuppression_ = ps.getParameter<bool>("zeroSuppression");

  // Save full analog pulse shapes from the HGCROC emulation
  savePulseTruthInfo_ = ps.getParameter<bool>("savePulseTruthInfo");

  // collection names
  input_coll_name_ = ps.getParameter<std::string>("inputCollName");
  input_pass_name_ = ps.getParameter<std::string>("inputPassName");
  digi_coll_name_ = ps.getParameter<std::string>("digiCollName");
  pulseTruthCollName_ = ps.getParameter<std::string>("pulseTruthCollName");

  // physical constants
  //  used to calculate unit conversions
  MeV_ = ps.getParameter<double>("MeV");
  attlength_ = ps.getParameter<double>("attenuationLength");

  // Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024. / clock_cycle_;

  // Configure generator that will produce noise hits_ in empty channels
  gain_ = ps.getParameter<double>("avgGain");
  readout_threshold_ = ps.getParameter<double>("avgReadoutThreshold");
  pedestal_ = ps.getParameter<double>("avgPedestal");
  noise_rms_ = ps.getParameter<double>("avgNoiseRMS");
}

void HcalDigiProducer::onNewRun(const ldmx::RunHeader&) {
  // rms noise in mV
  noise_generator_->setNoise(gain_ * noise_rms_);
  // mean noise amplitude (if using Gaussian Model for the noise) in mV
  noise_generator_->setPedestal(gain_ * pedestal_);
  // threshold for readout in mV
  noise_generator_->setNoiseThreshold(gain_ * readout_threshold_);

  // Set up seeds
  const auto& rseed = getCondition<framework::RandomNumberSeedService>(
      framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  noise_generator_->seedGenerator(
      rseed.getSeed("HcalDigiProducer::NoiseGenerator"));

  // Random number generator for layer_ / module_ / cell
  rng_.seed(rseed.getSeed("HcalDigiProducer"));
  // Setting up the read-out chip
  hgcroc_->seedGenerator(rseed.getSeed("HcalDigiProducer::HgcrocEmulator"));
}

void HcalDigiProducer::produce(framework::Event& event) {
  // Get the Hgcroc Conditions
  hgcroc_->condition(
      getCondition<conditions::DoubleTableCondition>("HcalHgcrocConditions"));

  // Get the Hcal Geometry
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  // Empty collection to be filled
  ldmx::HgcrocDigiCollection hcalDigis;
  hcalDigis.setNumSamplesPerDigi(nADCs_);
  hcalDigis.setSampleOfInterestIndex(iSOI_);

  std::map<unsigned int, std::vector<const ldmx::SimCalorimeterHit*>> hitsByID;

  // get simulated hcal hits_ from Geant4 and group them by id
  auto hcalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(
      input_coll_name_, input_pass_name_)};

  for (auto const& simHit : hcalSimHits) {
    // get ID
    unsigned int hitID = simHit.getID();

    auto idh = hitsByID.find(hitID);
    if (idh == hitsByID.end()) {
      hitsByID[hitID] = std::vector<const ldmx::SimCalorimeterHit*>(1, &simHit);
    } else {
      idh->second.push_back(&simHit);
    }
  }

  ldmx::HgcrocPulseTruthCollection hcalPulseTruthColl;
  if (savePulseTruthInfo_) {
    hgcroc_->pulseTruthColl_ = &hcalPulseTruthColl;
    hgcroc_->savePulseTruthInfo_ = true;
  }

  /******************************************************************************************
   * HGCROC Emulation on Simulated Hits (grouped by HcalID)
   ******************************************************************************************/
  for (auto const& simBar : hitsByID) {
    ldmx::HcalID detID(simBar.first);
    int section = detID.section();
    int layer_ = detID.layer();
    int strip = detID.strip();

    // get position
    double half_total_width = hcalGeometry.getHalfTotalWidth(section, layer_);
    double ecal_dx = hcalGeometry.getEcalDx();
    double ecal_dy = hcalGeometry.getEcalDy();

    // contributions
    std::vector<std::pair<double, double>> pulses_posend;
    std::vector<std::pair<double, double>> pulses_negend;

    for (auto psimHit : simBar.second) {
      const ldmx::SimCalorimeterHit& simHit = *psimHit;

      std::vector<float> position = simHit.getPosition();

      /**
       * Define two pulses: with positive and negative ends.
       * For this we need to:
       * (1) Find the position along the bar:
       *     For back Hcal: x_ (y_) for horizontal (vertical) layers.
       *     For side Hcal: x_ (top,bottom) and y_ (left,right).
       *
       * (2) Define the end of the bar:
       *     The end of an HcalDigiID is based on its distance (x_,y_) along the
       *     bar.
       *     - A positive end (endID=0), corresponds to top,left.
       *     - A negative end (endID=1), corresponds to bottom,right.
       *     For back Hcal:
       *     - if the position along the bar > 0, the close pulse's end is 0,
       *     else 1.
       *     For side Hcal:
       *     - if the position along the bar > half_width point of the bar, the
       *     close pulse's end is 0, else 1.
       *     The far pulse's end will be opposite to the close pulse's end.
       *
       * (3) Find the distance to each end (positive and negative) from the
       *     origin.
       *     For the back Hcal, the half point of the bar coincides with the
       *     coordinates of the origin.
       *     For the side Hcal, the length of the bar from the origin is:
       *     - 2 *(half_width) - Ecal_dx(y_)/2 away from the positive end, and,
       *     - Ecal_dx(y_) away from the negative end.
       */
      float distance_along_bar, distance_ecal;
      float distance_close, distance_far;
      int end_close;
      const auto orientation{hcalGeometry.getScintillatorOrientation(detID)};
      if (section == ldmx::HcalID::HcalSection::BACK) {
        distance_along_bar =
            (orientation ==
             ldmx::HcalGeometry::ScintillatorOrientation::horizontal)
                ? position[0]
                : position[1];
        end_close = (distance_along_bar > 0) ? 0 : 1;
        distance_close = half_total_width;
        distance_far = half_total_width;
      } else {
        if ((section == ldmx::HcalID::HcalSection::TOP) ||
            ((section == ldmx::HcalID::HcalSection::BOTTOM))) {
          distance_along_bar = position[0];
          distance_ecal = ecal_dx;
        } else if ((section == ldmx::HcalID::HcalSection::LEFT) ||
                   (section == ldmx::HcalID::HcalSection::RIGHT)) {
          distance_along_bar = position[1];
          distance_ecal = ecal_dy;
        } else {
          distance_along_bar = -9999.;
          EXCEPTION_RAISE(
              "BadCode",
              "We should never end up here "
              "All cases of HCAL considered, end_close is meaningless");
        }
        end_close = (distance_along_bar > half_total_width) ? 0 : 1;
        distance_close = (end_close == 0)
                             ? 2 * half_total_width - distance_ecal / 2
                             : distance_ecal / 2;
        distance_far = (end_close == 0)
                           ? distance_ecal / 2
                           : 2 * half_total_width - distance_ecal / 2;
      }

      // Calculate voltage attenuation and time shift for the close and far
      // pulse.
      // velocity of light in Polystyrene, n = 1.6 = c/v mm/ns
      float v = 299.792 / 1.6;
      double att_close =
          exp(-1. * ((distance_close - fabs(distance_along_bar)) / 1000.) /
              attlength_);
      double att_far =
          exp(-1. * ((distance_far + fabs(distance_along_bar)) / 1000.) /
              attlength_);
      double shift_close =
          fabs((distance_close - fabs(distance_along_bar)) / v);
      double shift_far = fabs((distance_far + fabs(distance_along_bar)) / v);

      // Get voltages and times.
      for (int iContrib = 0; iContrib < simHit.getNumberOfContribs();
           iContrib++) {
        double voltage = simHit.getContrib(iContrib).edep * MeV_;
        // global time (t=0ns at target)
        double time = simHit.getContrib(iContrib).time;
        // shift light-speed particle traveling along z_
        time -= position.at(2) / 299.702547;

        if (end_close == 0) {
          pulses_posend.emplace_back(voltage * att_close, time + shift_close);
          pulses_negend.emplace_back(voltage * att_far, time + shift_far);
        } else {
          pulses_posend.emplace_back(voltage * att_far, time + shift_far);
          pulses_negend.emplace_back(voltage * att_close, time + shift_close);
        }
      }
    }

    /**
     * Now we have all the sub-hits_ from all the simhits
     * Digitize:
     * For back Hcal return two digis.
     * For side Hcal we choose which pulse to readout based on
     * the position of the hit and the sub-section.
     * For Top and Left we read the positive end digi.
     * For Bottom and Right we read the negative end digi.
     **/
    if (section == ldmx::HcalID::HcalSection::BACK) {
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddPosend,
          digiToAddNegend;
      ldmx::HcalDigiID posendID(section, layer_, strip, 0);
      ldmx::HcalDigiID negendID(section, layer_, strip, 1);

      bool posEndActivity =
          hgcroc_->digitize(posendID.raw(), pulses_posend, digiToAddPosend);
      bool negEndActivity =
          hgcroc_->digitize(negendID.raw(), pulses_negend, digiToAddNegend);

      if (posEndActivity && negEndActivity && zeroSuppression_) {
        hcalDigis.addDigi(posendID.raw(), digiToAddPosend);
        hcalDigis.addDigi(negendID.raw(), digiToAddNegend);
      }  // If zeroSuppression == true, Back Hcal needs to digitize both
         // pulses or none

      if (!zeroSuppression_) {
        if (posEndActivity) {
          hcalDigis.addDigi(posendID.raw(), digiToAddPosend);
        } else {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(posendID.raw(), 0.0);
          hcalDigis.addDigi(posendID.raw(), digi);
        }
        if (negEndActivity) {
          hcalDigis.addDigi(negendID.raw(), digiToAddNegend);
        } else {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(negendID.raw(), 0.0);
          hcalDigis.addDigi(negendID.raw(), digi);
        }
      }

    } else {
      bool is_posend = false;
      std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd;
      if ((section == ldmx::HcalID::HcalSection::TOP) ||
          (section == ldmx::HcalID::HcalSection::LEFT)) {
        is_posend = true;
      } else if ((section == ldmx::HcalID::HcalSection::BOTTOM) ||
                 (section == ldmx::HcalID::HcalSection::RIGHT)) {
        is_posend = false;
      }
      if (is_posend) {
        ldmx::HcalDigiID digiID(section, layer_, strip, 0);
        if (hgcroc_->digitize(digiID.raw(), pulses_posend, digiToAdd)) {
          hcalDigis.addDigi(digiID.raw(), digiToAdd);
        } else if (!zeroSuppression_) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(digiID.raw(), 0.0);
          hcalDigis.addDigi(digiID.raw(), digi);
        }
      } else {
        ldmx::HcalDigiID digiID(section, layer_, strip, 1);
        if (hgcroc_->digitize(digiID.raw(), pulses_negend, digiToAdd)) {
          hcalDigis.addDigi(digiID.raw(), digiToAdd);
        } else if (!zeroSuppression_) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(digiID.raw(), 0.0);
          hcalDigis.addDigi(digiID.raw(), digi);
        }
      }
    }
  }

  /******************************************************************************************
   * Noise Simulation on Empty Channels
   *****************************************************************************************/
  if (noise_) {
    std::vector<ldmx::HcalDigiID> channelMap;
    int numChannels = 0;
    for (int section = 0; section < hcalGeometry.getNumSections(); section++) {
      for (int layer_ = 1; layer_ <= hcalGeometry.getNumLayers(section);
           layer_++) {
        // Note zero-indexed strip numbering...
        for (int strip = 0; strip < hcalGeometry.getNumStrips(section, layer_);
             strip++) {
          if (section == ldmx::HcalID::HcalSection::BACK) {
            auto digiIDend0 = ldmx::HcalDigiID(section, layer_, strip, 0);
            auto digiIDend1 = ldmx::HcalDigiID(section, layer_, strip, 1);
            channelMap.push_back(digiIDend0);
            channelMap.push_back(digiIDend1);
            numChannels += 2;
          } else {
            auto digiID = ldmx::HcalDigiID(section, layer_, strip, 0);
            channelMap.push_back(digiID);
            numChannels++;
          }
        }
      }
    }

    // Uniform distributions for integer generation
    std::uniform_int_distribution<int> section_dist(
        0, hcalGeometry.getNumSections() - 1);
    std::uniform_int_distribution<int> end_dist(0, 1);
    std::uniform_int_distribution<int> clock_dist(0, clock_cycle_);

    // Fast noise sim
    if (zeroSuppression_) {
      int numEmptyChannels = numChannels - hcalDigis.getNumDigis();
      // noise generator gives us a list of noise amplitudes [mV] that randomly
      // populate the empty channels and are above the readout threshold
      auto noiseHitAmplitudes{
          noise_generator_->generateNoiseHits(numEmptyChannels)};
      std::vector<std::pair<double, double>> fake_pulse(1, {0., 0.});

      for (double noiseHit : noiseHitAmplitudes) {
        // generate detector ID for noise hit
        // making sure that it is in an empty channel
        unsigned int noiseID;
        int sectionID, layerID, stripID, endID;
        do {
          // Get a random section value
          sectionID = section_dist(rng_);

          // Get a random value for the layer_
          std::uniform_int_distribution<int> layer_dist(
              0, hcalGeometry.getNumLayers(sectionID) - 1);
          layerID = layer_dist(rng_);
          // set layer_ to 1 if the generator says it is 0 (geometry map starts
          // from 1)
          if (layerID == 0) layerID = 1;

          // Get a random value for the  strip
          std::uniform_int_distribution<int> strips_dist(
              0, hcalGeometry.getNumStrips(sectionID, layerID) - 1);
          stripID = strips_dist(rng_);

          //  Get a random value for the  end
          if ((sectionID == ldmx::HcalID::HcalSection::TOP) ||
              (sectionID == ldmx::HcalID::HcalSection::LEFT)) {
            endID = 0;
          } else if ((sectionID == ldmx::HcalID::HcalSection::BOTTOM) ||
                     (sectionID == ldmx::HcalID::HcalSection::RIGHT)) {
            endID = 1;
          } else {
            endID = end_dist(rng_);
          }
          auto detID = ldmx::HcalDigiID(sectionID, layerID, stripID, endID);
          noiseID = detID.raw();
        } while (hitsByID.find(noiseID) != hitsByID.end());
        hitsByID[noiseID] =
            std::vector<const ldmx::SimCalorimeterHit*>();  // mark this as used

        // get a time for this noise hit
        fake_pulse[0].second = clock_dist(rng_);

        // noise generator gives the amplitude above the readout threshold
        // we need to convert it to the amplitude above the pedestal
        double gain = hgcroc_->gain(noiseID);
        fake_pulse[0].first = noiseHit +
                              gain * hgcroc_->readoutThreshold(noiseID) -
                              gain * hgcroc_->pedestal(noiseID);

        if (sectionID == ldmx::HcalID::HcalSection::BACK) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAddPosend,
              digiToAddNegend;
          ldmx::HcalDigiID posendID(sectionID, layerID, stripID, 0);
          ldmx::HcalDigiID negendID(sectionID, layerID, stripID, 1);
          if (hgcroc_->digitize(posendID.raw(), fake_pulse, digiToAddPosend) &&
              hgcroc_->digitize(negendID.raw(), fake_pulse, digiToAddNegend)) {
            hcalDigis.addDigi(posendID.raw(), digiToAddPosend);
            hcalDigis.addDigi(negendID.raw(), digiToAddNegend);
          }
        } else {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digiToAdd;
          if (hgcroc_->digitize(noiseID, fake_pulse, digiToAdd)) {
            hcalDigis.addDigi(noiseID, digiToAdd);
          }
        }
      }  // loop over noise amplitudes
    } else {  // If zeroSuppression_ == false, add noise digis for all bars
              // without simhits
      for (auto digiID : channelMap) {
        // Convert from digi ID to det ID (since simhits don't know about
        // different ends of the bar)
        ldmx::HcalID detid(digiID.section(), digiID.layer(), digiID.strip());
        unsigned int rawdetID = detid.raw();
        if (hitsByID.find(rawdetID) == hitsByID.end()) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(digiID.raw(), 0.0);
          hcalDigis.addDigi(digiID.raw(), digi);
        }
      }
    }
  }  // if we should add noise

  event.add(digi_coll_name_, hcalDigis);
  if (savePulseTruthInfo_) event.add(pulseTruthCollName_, hcalPulseTruthColl);

  return;
}  // produce

}  // namespace hcal

DECLARE_PRODUCER(hcal::HcalDigiProducer);
