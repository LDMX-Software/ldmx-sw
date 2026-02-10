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
   * i.e. it assumes the noise is distributed around a mean (setPedestal)
   * with a certain RMS (setNoise) and then calculates
   * how many hits should be generated for a given number of empty
   * channels and a minimum readout value (setNoiseThreshold)
   */
  noise_generator_ = std::make_unique<ldmx::NoiseGenerator>();
}

double HcalDigiProducer::makeTimeDelta(TimeSpreadType kind,
                                       std::vector<double>& parameters) const {
  switch (kind) {
    case TimeSpreadType::UNIFORM:
      return random_time_->Uniform(parameters[0], parameters[1]);
    case TimeSpreadType::CONSTANT:
      return parameters[0];
    case TimeSpreadType::GAUSSIAN:
      return random_time_->Gaus(parameters[0], parameters[1]);
  }
  return 0;
}

void HcalDigiProducer::configure(framework::config::Parameters& ps) {
  // settings of readout chip
  //  used  in actual digitization
  auto hgcroc_params = ps.get<framework::config::Parameters>("hgcroc");
  hgcroc_ = std::make_unique<ldmx::HgcrocEmulator>(hgcroc_params);
  clock_cycle_ = hgcroc_params.get<double>("clock_cycle");
  n_adcs_ = hgcroc_params.get<int>("n_adcs");
  i_soi_ = hgcroc_params.get<int>("i_soi");
  noise_ = hgcroc_params.get<bool>("noise");

  // If true, ignore readout threshold
  // and generate pedestal noise digis in every empty channel
  zero_suppression_ = ps.get<bool>("zero_suppression");

  // Save full analog pulse shapes from the HGCROC emulation
  save_pulse_truth_info_ = ps.get<bool>("save_pulse_truth_info");

  // collection names
  input_coll_name_ = ps.get<std::string>("input_coll_name");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  digi_coll_name_ = ps.get<std::string>("digi_coll_name");
  pulse_truth_coll_name_ = ps.get<std::string>("pulse_truth_coll_name");

  // physical constants
  //  used to calculate unit conversions
  mev_ = ps.get<double>("mev");
  attlength_ = ps.get<double>("attenuation_length");

  // Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024. / clock_cycle_;

  // Configure generator that will produce noise hits in empty channels
  gain_ = ps.get<double>("avg_gain");
  readout_threshold_ = ps.get<double>("avg_readout_threshold");
  pedestal_ = ps.get<double>("avg_pedestal");
  noise_rms_ = ps.get<double>("avg_noise_rms");

  flat_time_shift_ = ps.get<double>("flat_time_shift");
  // Time spread parameters
  // Per hit
  const auto& time_spread_per_hit{
      ps.get<framework::config::Parameters>("time_spread_per_hit")};
  int kind = time_spread_per_hit.get<int>("kind");
  time_spread_per_hit_parameters_ =
      time_spread_per_hit.get<std::vector<double>>("parameters");
  do_time_spread_per_hit_ = kind != -1;
  time_spread_per_hit_type_ = static_cast<TimeSpreadType>(kind);
  // Per spill / event
  const auto& time_spread_per_spill{
      ps.get<framework::config::Parameters>("time_spread_per_spill")};
  kind = time_spread_per_spill.get<int>("kind");
  time_spread_per_spill_parameters_ =
      time_spread_per_spill.get<std::vector<double>>("parameters");
  do_time_spread_per_spill_ = kind != -1;
  time_spread_per_spill_type_ = static_cast<TimeSpreadType>(kind);
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

  // Random number generator for time shifts
  random_time_ =
      std::make_unique<TRandom2>(rseed.getSeed("HcalDigiProducer::randomTime"));
}

void HcalDigiProducer::produce(framework::Event& event) {
  // Get the Hgcroc Conditions
  hgcroc_->condition(
      getCondition<conditions::DoubleTableCondition>("HcalHgcrocConditions"));

  // Get the Hcal Geometry
  const auto& hcal_geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  // Empty collection to be filled
  ldmx::HgcrocDigiCollection hcal_digis;
  hcal_digis.setNumSamplesPerDigi(n_adcs_);
  hcal_digis.setSampleOfInterestIndex(i_soi_);

  std::map<unsigned int, std::vector<const ldmx::SimCalorimeterHit*>>
      hits_by_id;

  // get simulated hcal hits from Geant4 and group them by id
  auto hcal_sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      input_coll_name_, input_pass_name_)};

  for (auto const& sim_hit : hcal_sim_hits) {
    // get ID
    unsigned int hit_id = sim_hit.getID();

    auto idh = hits_by_id.find(hit_id);
    if (idh == hits_by_id.end()) {
      hits_by_id[hit_id] =
          std::vector<const ldmx::SimCalorimeterHit*>(1, &sim_hit);
    } else {
      idh->second.push_back(&sim_hit);
    }
  }

  ldmx::HgcrocPulseTruthCollection hcal_pulse_truth_coll;
  if (save_pulse_truth_info_) {
    hgcroc_->pulse_truth_coll_ = &hcal_pulse_truth_coll;
    hgcroc_->save_pulse_truth_info_ = true;
  }

  /******************************************************************************************
   * HGCROC Emulation on Simulated Hits (grouped by HcalID)
   ******************************************************************************************/
  double time_delta{flat_time_shift_};
  if (do_time_spread_per_spill_) {
    time_delta += makeTimeDelta(time_spread_per_spill_type_,
                                time_spread_per_spill_parameters_);
  }
  for (auto const& sim_bar : hits_by_id) {
    ldmx::HcalID det_id(sim_bar.first);
    int section = det_id.section();
    int layer = det_id.layer();
    int strip = det_id.strip();

    // get position
    double half_total_width = hcal_geometry.getHalfTotalWidth(section, layer);
    double ecal_dx = hcal_geometry.getEcalDx();
    double ecal_dy = hcal_geometry.getEcalDy();

    // contributions
    std::vector<std::pair<double, double>> pulses_posend;
    std::vector<std::pair<double, double>> pulses_negend;

    for (auto psim_hit : sim_bar.second) {
      const ldmx::SimCalorimeterHit& sim_hit = *psim_hit;

      std::vector<float> position = sim_hit.getPosition();

      /**
       * Define two pulses: with positive and negative ends.
       * For this we need to:
       * (1) Find the position along the bar:
       *     For back Hcal: x (y) for horizontal (vertical) layers.
       *     For side Hcal: x (top,bottom) and y (left,right).
       *
       * (2) Define the end of the bar:
       *     The end of an HcalDigiID is based on its distance (x,y) along the
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
      const auto orientation{hcal_geometry.getScintillatorOrientation(det_id)};
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
      for (int i_contrib = 0; i_contrib < sim_hit.getNumberOfContribs();
           i_contrib++) {
        double voltage = sim_hit.getContrib(i_contrib).edep_ * mev_;
        // global time (t=0ns at target)
        double time = sim_hit.getContrib(i_contrib).time_;
        // shift light-speed particle traveling along z_
        time -= position.at(2) / 299.702547;
        if (do_time_spread_per_hit_) {
          time += makeTimeDelta(time_spread_per_hit_type_,
                                time_spread_per_hit_parameters_);
        }
        time += time_delta;

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
     * Now we have all the sub-hits from all the simhits
     * Digitize:
     * For back Hcal return two digis.
     * For side Hcal we choose which pulse to readout based on
     * the position of the hit and the sub-section.
     * For Top and Left we read the positive end digi.
     * For Bottom and Right we read the negative end digi.
     **/
    if (section == ldmx::HcalID::HcalSection::BACK) {
      std::vector<ldmx::HgcrocDigiCollection::Sample> digi_to_add_posend,
          digi_to_add_negend;
      ldmx::HcalDigiID posend_id(section, layer, strip, 0);
      ldmx::HcalDigiID negend_id(section, layer, strip, 1);

      bool pos_end_activity =
          hgcroc_->digitize(posend_id.raw(), pulses_posend, digi_to_add_posend);
      bool neg_end_activity =
          hgcroc_->digitize(negend_id.raw(), pulses_negend, digi_to_add_negend);

      if (pos_end_activity && neg_end_activity && zero_suppression_) {
        hcal_digis.addDigi(posend_id.raw(), digi_to_add_posend);
        hcal_digis.addDigi(negend_id.raw(), digi_to_add_negend);
      }  // If zeroSuppression == true, Back Hcal needs to digitize both
         // pulses or none

      if (!zero_suppression_) {
        if (pos_end_activity) {
          hcal_digis.addDigi(posend_id.raw(), digi_to_add_posend);
        } else {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(posend_id.raw(), 0.0);
          hcal_digis.addDigi(posend_id.raw(), digi);
        }
        if (neg_end_activity) {
          hcal_digis.addDigi(negend_id.raw(), digi_to_add_negend);
        } else {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(negend_id.raw(), 0.0);
          hcal_digis.addDigi(negend_id.raw(), digi);
        }
      }

    } else {
      bool is_posend = false;
      std::vector<ldmx::HgcrocDigiCollection::Sample> digi_to_add;
      if ((section == ldmx::HcalID::HcalSection::TOP) ||
          (section == ldmx::HcalID::HcalSection::LEFT)) {
        is_posend = true;
      } else if ((section == ldmx::HcalID::HcalSection::BOTTOM) ||
                 (section == ldmx::HcalID::HcalSection::RIGHT)) {
        is_posend = false;
      }
      if (is_posend) {
        ldmx::HcalDigiID digi_id(section, layer, strip, 0);
        if (hgcroc_->digitize(digi_id.raw(), pulses_posend, digi_to_add)) {
          hcal_digis.addDigi(digi_id.raw(), digi_to_add);
        } else if (!zero_suppression_) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(digi_id.raw(), 0.0);
          hcal_digis.addDigi(digi_id.raw(), digi);
        }
      } else {
        ldmx::HcalDigiID digi_id(section, layer, strip, 1);
        if (hgcroc_->digitize(digi_id.raw(), pulses_negend, digi_to_add)) {
          hcal_digis.addDigi(digi_id.raw(), digi_to_add);
        } else if (!zero_suppression_) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(digi_id.raw(), 0.0);
          hcal_digis.addDigi(digi_id.raw(), digi);
        }
      }
    }
  }

  /******************************************************************************************
   * Noise Simulation on Empty Channels
   *****************************************************************************************/
  if (noise_) {
    std::vector<ldmx::HcalDigiID> channel_map;
    int num_channels = 0;
    for (int section = 0; section < hcal_geometry.getNumSections(); section++) {
      for (int layer = 1; layer <= hcal_geometry.getNumLayers(section);
           layer++) {
        // Note zero-indexed strip numbering...
        for (int strip = 0; strip < hcal_geometry.getNumStrips(section, layer);
             strip++) {
          if (section == ldmx::HcalID::HcalSection::BACK) {
            auto digi_i_dend0 = ldmx::HcalDigiID(section, layer, strip, 0);
            auto digi_i_dend1 = ldmx::HcalDigiID(section, layer, strip, 1);
            channel_map.push_back(digi_i_dend0);
            channel_map.push_back(digi_i_dend1);
            num_channels += 2;
          } else {
            auto digi_id = ldmx::HcalDigiID(section, layer, strip, 0);
            channel_map.push_back(digi_id);
            num_channels++;
          }
        }
      }
    }

    // Uniform distributions for integer generation
    std::uniform_int_distribution<int> section_dist(
        0, hcal_geometry.getNumSections() - 1);
    std::uniform_int_distribution<int> end_dist(0, 1);
    std::uniform_int_distribution<int> clock_dist(0, clock_cycle_);

    // Fast noise sim
    if (zero_suppression_) {
      int num_empty_channels = num_channels - hcal_digis.getNumDigis();
      // noise generator gives us a list of noise amplitudes [mV] that randomly
      // populate the empty channels and are above the readout threshold
      auto noise_hit_amplitudes{
          noise_generator_->generateNoiseHits(num_empty_channels)};
      std::vector<std::pair<double, double>> fake_pulse(1, {0., 0.});

      for (double noise_hit : noise_hit_amplitudes) {
        // generate detector ID for noise hit
        // making sure that it is in an empty channel
        unsigned int noise_id;
        int section_id, layer_id, strip_id, end_id;
        do {
          // Get a random section value
          section_id = section_dist(rng_);

          // Get a random value for the layer_
          std::uniform_int_distribution<int> layer_dist(
              0, hcal_geometry.getNumLayers(section_id) - 1);
          layer_id = layer_dist(rng_);
          // set layer_ to 1 if the generator says it is 0 (geometry map starts
          // from 1)
          if (layer_id == 0) layer_id = 1;

          // Get a random value for the  strip
          std::uniform_int_distribution<int> strips_dist(
              0, hcal_geometry.getNumStrips(section_id, layer_id) - 1);
          strip_id = strips_dist(rng_);

          //  Get a random value for the  end
          if ((section_id == ldmx::HcalID::HcalSection::TOP) ||
              (section_id == ldmx::HcalID::HcalSection::LEFT)) {
            end_id = 0;
          } else if ((section_id == ldmx::HcalID::HcalSection::BOTTOM) ||
                     (section_id == ldmx::HcalID::HcalSection::RIGHT)) {
            end_id = 1;
          } else {
            end_id = end_dist(rng_);
          }
          auto det_id =
              ldmx::HcalDigiID(section_id, layer_id, strip_id, end_id);
          noise_id = det_id.raw();
        } while (hits_by_id.find(noise_id) != hits_by_id.end());
        hits_by_id[noise_id] =
            std::vector<const ldmx::SimCalorimeterHit*>();  // mark this as used

        // get a time for this noise hit
        fake_pulse[0].second = clock_dist(rng_);

        // noise generator gives the amplitude above the readout threshold
        // we need to convert it to the amplitude above the pedestal
        double gain = hgcroc_->gain(noise_id);
        fake_pulse[0].first = noise_hit +
                              gain * hgcroc_->readoutThreshold(noise_id) -
                              gain * hgcroc_->pedestal(noise_id);

        if (section_id == ldmx::HcalID::HcalSection::BACK) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi_to_add_posend,
              digi_to_add_negend;
          ldmx::HcalDigiID posend_id(section_id, layer_id, strip_id, 0);
          ldmx::HcalDigiID negend_id(section_id, layer_id, strip_id, 1);
          if (hgcroc_->digitize(posend_id.raw(), fake_pulse,
                                digi_to_add_posend) &&
              hgcroc_->digitize(negend_id.raw(), fake_pulse,
                                digi_to_add_negend)) {
            hcal_digis.addDigi(posend_id.raw(), digi_to_add_posend);
            hcal_digis.addDigi(negend_id.raw(), digi_to_add_negend);
          }
        } else {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi_to_add;
          if (hgcroc_->digitize(noise_id, fake_pulse, digi_to_add)) {
            hcal_digis.addDigi(noise_id, digi_to_add);
          }
        }
      }  // loop over noise amplitudes
    } else {  // If zero_suppression_ == false, add noise digis for all bars
              // without simhits
      for (auto digi_id : channel_map) {
        // Convert from digi ID to det ID (since simhits don't know about
        // different ends of the bar)
        ldmx::HcalID detid(digi_id.section(), digi_id.layer(), digi_id.strip());
        unsigned int rawdet_id = detid.raw();
        if (hits_by_id.find(rawdet_id) == hits_by_id.end()) {
          std::vector<ldmx::HgcrocDigiCollection::Sample> digi =
              hgcroc_->noiseDigi(digi_id.raw(), 0.0);
          hcal_digis.addDigi(digi_id.raw(), digi);
        }
      }
    }
  }  // if we should add noise

  event.add(digi_coll_name_, hcal_digis);
  if (save_pulse_truth_info_)
    event.add(pulse_truth_coll_name_, hcal_pulse_truth_coll);

  return;
}  // produce

}  // namespace hcal

DECLARE_PRODUCER(hcal::HcalDigiProducer);
