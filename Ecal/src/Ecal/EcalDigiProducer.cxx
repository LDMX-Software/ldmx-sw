/**
 * @file EcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Tamas Almos Vami, UCSB
 */

#include "Ecal/EcalDigiProducer.h"

#include "DetDescr/EcalGeometry.h"
#include "Framework/RandomNumberSeedService.h"

namespace ecal {

void EcalDigiProducer::configure(framework::config::Parameters& ps) {
  // settings of readout chip
  //  used  in actual digitization
  auto hgcroc_params = ps.getParameter<framework::config::Parameters>("hgcroc");
  hgcroc_ = std::make_unique<ldmx::HgcrocEmulator>(hgcroc_params);
  clockCycle_ = hgcroc_params.getParameter<double>("clockCycle");
  nADCs_ = hgcroc_params.getParameter<int>("nADCs");
  iSOI_ = hgcroc_params.getParameter<int>("iSOI");
  noise_ = hgcroc_params.getParameter<bool>("noise");

  // collection names
  inputCollName_ = ps.getParameter<std::string>("inputCollName");
  inputPassName_ = ps.getParameter<std::string>("inputPassName");
  digiCollName_ = ps.getParameter<std::string>("digiCollName");

  zero_suppression_ = ps.getParameter<bool>("zero_suppression");

  // physical constants
  //  used to calculate unit conversions
  MeV_ = ps.getParameter<double>("MeV");

  // Time -> clock counts conversion
  //  time [ns] * ( 2^10 / max time in ns ) = clock counts
  ns_ = 1024. / clockCycle_;

  readoutThreshold_ = ps.getParameter<double>("avgReadoutThreshold");
  pedestal_ = ps.getParameter<double>("avgPedestal");
  noiseRMS_ = ps.getParameter<double>("avgNoiseRMS");
}

void EcalDigiProducer::onNewRun(const ldmx::RunHeader&) {
  // noise generator by default uses a Gausian model for noise
  //  i.e. It assumes the noise is distributed around a mean (setPedestal)
  //  with a certain RMS (setNoise) and then calculates
  //  how many hits should be generated for a given number of empty
  //  channels and a minimum readout value (setNoiseThreshold)
  noiseGenerator_ = std::make_unique<ldmx::NoiseGenerator>();
  // Configure generator that will produce noise hits in empty channels
  // rms noise in mV
  noiseGenerator_->setNoise(noiseRMS_);
  // mean noise amplitude (if using Gaussian Model for the noise) in mV
  noiseGenerator_->setPedestal(pedestal_);
  // threshold for readout in mV
  noiseGenerator_->setNoiseThreshold(readoutThreshold_);
  // Set up seeds
  const auto& rseed = getCondition<framework::RandomNumberSeedService>(
      framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  noiseGenerator_->seedGenerator(
      rseed.getSeed("EcalDigiProducer::NoiseGenerator"));
  // Random number generator for layer / module / cell
  rng_.seed(rseed.getSeed("EcalDigiProducer"));
  // Setting up the read-out chip
  hgcroc_->seedGenerator(rseed.getSeed("EcalDigiProducer::HgcrocEmulator"));
  hgcroc_->condition(
      getCondition<conditions::DoubleTableCondition>("EcalHgcrocConditions"));
}

void EcalDigiProducer::produce(framework::Event& event) {
  // Empty collection to be filled
  ldmx::HgcrocDigiCollection ecal_digis;
  ecal_digis.setNumSamplesPerDigi(nADCs_);
  ecal_digis.setSampleOfInterestIndex(iSOI_);

  // detector IDs that already have a hit in them
  std::set<unsigned int> filled_det_i_ds;

  /******************************************************************************************
   * HGCROC Emulation on Simulated Hits
   *****************************************************************************************/
  // std::cout << "Sim Hits" << std::endl;
  // get simulated ecal hits from Geant4
  //  the class EcalHitIO in the SimApplication module handles the translation
  //  from G4CalorimeterHits to SimCalorimeterHits this class ensures that only
  //  one SimCalorimeterHit is generated per cell, but multiple "contributions"
  //  are still handled within SimCalorimeterHit
  auto ecal_sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      inputCollName_, inputPassName_)};

  /* debug printout
  std::cout << "Energy to Voltage Conversion: " << MeV_ << " mV/MeV" <<
  std::endl;
   */

  for (auto const& sim_hit : ecal_sim_hits) {
    std::vector<std::pair<double, double>> pulses_at_chip;
    for (int i_contrib = 0; i_contrib < sim_hit.getNumberOfContribs();
         i_contrib++) {
      /* debug printout
      std::cout << simHit.getContrib(iContrib).edep << " MeV" << std::endl;
       */
      /**
       * HACK ALERT
       * The shifting of the time should _not_ be done this sloppily.
       * In reality, each chip has a set time phase that it samples at (relative
       * to target), so the time shifting should be at the emulator level.
       */
      pulses_at_chip.emplace_back(
          sim_hit.getContrib(i_contrib).edep * MeV_,
          sim_hit.getContrib(i_contrib).time  // global time (t=0ns at target)
              - sim_hit.getPosition().at(2) /
                    299.702547  // shift light-speed particle traveling along z
      );
    }

    unsigned int hit_id = sim_hit.getID();
    filled_det_i_ds.insert(hit_id);

    ldmx_log(debug) << " Emulation of hitID = " << hit_id
                    << " with energy = " << sim_hit.getEdep()
                    << " MeV at time = "
                    << sim_hit.getTime() -
                           sim_hit.getPosition().at(2) / 299.702547;

    // container emulator uses to write out samples and
    // transfer samples into the digi collection
    std::vector<ldmx::HgcrocDigiCollection::Sample> digi_to_add;
    if (hgcroc_->digitize(hit_id, pulses_at_chip, digi_to_add)) {
      ldmx_log(debug) << "  --> The HGCROC will read-out this hit!";
      ecal_digis.addDigi(hit_id, digi_to_add);
    }
  }

  /******************************************************************************************
   * Noise Simulation on Empty Channels
   *****************************************************************************************/
  if (noise_) {
    // put noise into some empty channels

    // geometry constants
    //  These are used in the noise generation so that we can randomly
    //  distribute the noise uniformly throughout the ECal channels.
    const auto& geom = getCondition<ldmx::EcalGeometry>(
        ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);
    int n_ecal_layers = geom.getNumLayers();
    int n_modules_per_layer = geom.getNumModulesPerLayer();
    int n_cells_per_module = geom.getNumCellsPerModule();
    int num_empty_channels = n_ecal_layers * n_modules_per_layer * n_cells_per_module -
                           ecal_digis.getNumDigis();

    // Uniform distributions for integer generation
    std::uniform_int_distribution<int> layer_dist(0, n_ecal_layers - 1);
    std::uniform_int_distribution<int> module_dist(0, n_modules_per_layer - 1);
    std::uniform_int_distribution<int> cell_dist(0, n_cells_per_module - 1);

    if (zero_suppression_) {
      // noise generator gives us a list of noise amplitudes [mV] that randomly
      // populate the empty channels and are above the readout threshold
      auto noise_hit_amplitudes{
          noiseGenerator_->generateNoiseHits(num_empty_channels)};
      std::vector<std::pair<double, double>> fake_pulse(1, {0., 0.});
      for (double noise_hit : noise_hit_amplitudes) {
        // generate detector ID for noise hit
        // making sure that it is in an empty channel
        unsigned int noise_id;
        do {
          int layer_id = layer_dist(rng_);
          int module_id = module_dist(rng_);
          int cell_id = cell_dist(rng_);
          auto det_id = ldmx::EcalID(layer_id, module_id, cell_id);
          noise_id = det_id.raw();
        } while (filled_det_i_ds.find(noise_id) != filled_det_i_ds.end());
        filled_det_i_ds.insert(noise_id);

        // noise generator gives the amplitude above the readout threshold
        //  we need to convert it to the amplitude above the pedestal
        noise_hit +=
            hgcroc_->gain(noise_id) *
            (hgcroc_->readoutThreshold(noise_id) - hgcroc_->pedestal(noise_id));

        // create a digi as put it into the collection
        ecal_digis.addDigi(noise_id, hgcroc_->noiseDigi(noise_id, noise_hit));
      }  // loop over noise amplitudes
    } else {
      // no zero suppression, put some noise emulation in **all** empty channels
      // loop through all channels
      for (int layer{0}; layer < n_ecal_layers; layer++) {
        for (int module{0}; module < n_modules_per_layer; module++) {
          for (int cell{0}; cell < n_cells_per_module; cell++) {
            unsigned int channel{ldmx::EcalID(layer, module, cell).raw()};
            // check if channel already has a (real) hit in it
            if (filled_det_i_ds.find(channel) != filled_det_i_ds.end()) continue;
            // create a digi as put it into the collection
            ecal_digis.addDigi(channel, hgcroc_->noiseDigi(channel));
          }  // cells in each module
        }  // modules in each layer
      }  // layers in ECal
    }  // yes or no zero suppression
  }  // if we should do the noise

  event.add(digiCollName_, ecal_digis);

  return;
}  // produce

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalDigiProducer);
