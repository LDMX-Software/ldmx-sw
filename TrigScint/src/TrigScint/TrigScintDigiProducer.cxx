#include "TrigScint/TrigScintDigiProducer.h"

namespace trigscint {

TrigScintDigiProducer::TrigScintDigiProducer(const std::string &name,
                                             framework::Process &process)
    : Producer(name, process) {}

void TrigScintDigiProducer::configure(
    framework::config::Parameters &parameters) {
  // Configure this instance of the producer
  stripsPerArray_ = parameters.getParameter<int>("number_of_strips");
  numberOfArrays_ = parameters.getParameter<int>("number_of_arrays");
  meanNoise_ = parameters.getParameter<double>("mean_noise");
  mevPerMip_ = parameters.getParameter<double>("mev_per_mip");
  pePerMip_ = parameters.getParameter<double>("pe_per_mip");
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  sim_particles_passname_ =
      parameters.getParameter<std::string>("sim_particles_passname");
}

void TrigScintDigiProducer::onNewRun(const ldmx::RunHeader &) {
  noiseGenerator_ = std::make_unique<ldmx::NoiseGenerator>(meanNoise_, false);
  noiseGenerator_->setNoiseThreshold(1);
  // Set up seeds
  const auto &rseed = getCondition<framework::RandomNumberSeedService>(
      framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);

  noiseGenerator_->seedGenerator(
      rseed.getSeed("TrigScintDigiProducer::NoiseGenerator"));
  // Random number generator for module id
  rng_.seed(rseed.getSeed("TrigScintDigiProducer"));
}

ldmx::TrigScintID TrigScintDigiProducer::generateRandomID(int module) {
  // Uniform distributions for integer generation
  std::uniform_int_distribution<int> strips_dist(0, stripsPerArray_ - 1);
  ldmx::TrigScintID temp_id(module, strips_dist(rng_));
  if (module >= TrigScintSection::NUM_SECTIONS) {
    ldmx_log(fatal) << "TrigScintSection is not known";
  }

  return temp_id;
}

void TrigScintDigiProducer::produce(framework::Event &event) {
  std::map<ldmx::TrigScintID, int> cell_p_es, cell_min_p_es;
  std::map<ldmx::TrigScintID, float> xpos, ypos, zpos, edep, time, beam_frac;
  std::set<ldmx::TrigScintID> noise_hit_i_ds;

  auto num_rec_hits{0};

  // looper over sim hits and aggregate energy depositions for each detID
  const auto sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
      inputCollection_, inputPassName_)};
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_passname_)};

  int module{-1};
  for (const auto &sim_hit : sim_hits) {
    ldmx::TrigScintID id(sim_hit.getID());

    // Just set the module ID to use for noise hits here.  Given that
    // we are currently processing a single module at a time, setting
    // it within the loop shouldn't matter.
    module = id.module();
    std::vector<float> position = sim_hit.getPosition();
    ldmx_log(trace) << " Module ID = " << id.raw();

    // check if hits is from beam electron and, if so, add to beamFrac
    for (int i = 0; i < sim_hit.getNumberOfContribs(); i++) {
      auto contrib = sim_hit.getContrib(i);

      ldmx_log(trace) << "contrib " << i << " trackID: " << contrib.trackID
                      << " pdgID: " << contrib.pdgCode
                      << " edep: " << contrib.edep;
      ldmx_log(trace) << "\t particle id: "
                      << particle_map[contrib.trackID].getPdgID()
                      << " particle status: "
                      << particle_map[contrib.trackID].getGenStatus();

      if (particle_map[contrib.trackID].getPdgID() == 11 &&
          particle_map[contrib.trackID].getGenStatus() == 1) {
        if (beam_frac.find(id) == beam_frac.end()) {
          beam_frac[id] = contrib.edep;
        } else {
          beam_frac[id] += contrib.edep;
        }
      }
    }

    // for now, we take an energy weighted average of the hit in each strip to
    // simulate the hit position. AJW: these should be dropped, they are likely
    // to lead to a problem since we can't measure them anyway except roughly y
    // and z, which is encoded in the ids.
    if (edep.find(id) == edep.end()) {
      // first hit, initialize
      edep[id] = sim_hit.getEdep();
      time[id] = sim_hit.getTime() * sim_hit.getEdep();
      xpos[id] = position[0] * sim_hit.getEdep();
      ypos[id] = position[1] * sim_hit.getEdep();
      zpos[id] = position[2] * sim_hit.getEdep();
      num_rec_hits++;

    } else {
      // not first hit, aggregate, and store the largest radius hit
      xpos[id] += position[0] * sim_hit.getEdep();
      ypos[id] += position[1] * sim_hit.getEdep();
      zpos[id] += position[2] * sim_hit.getEdep();
      edep[id] += sim_hit.getEdep();
      // AJW: need to figure out a better way to model this...
      time[id] += sim_hit.getTime() * sim_hit.getEdep();
    }
  }

  // Create the container to hold the digitized trigger scintillator hits.
  std::vector<ldmx::TrigScintHit> trig_scint_hits;

  // loop over detIDs and simulate number of PEs
  for (std::map<ldmx::TrigScintID, float>::iterator it = edep.begin();
       it != edep.end(); ++it) {
    ldmx::TrigScintID id(it->first);

    double dep_energy = edep[id];
    time[id] = time[id] / edep[id];
    xpos[id] = xpos[id] / edep[id];
    ypos[id] = ypos[id] / edep[id];
    zpos[id] = zpos[id] / edep[id];
    double mean_pe = dep_energy / mevPerMip_ * pePerMip_;
    std::poisson_distribution<int> poisson_dist(mean_pe + meanNoise_);
    cell_p_es[id] = poisson_dist(rng_);

    // If a cell has a PE count above threshold, persit the hit.
    // Thresholds are introduced (and configurable) in clustering.
    // the cell PE >=1 suppresses artifical noise that is below one light
    // quantum in the SiPM and unphysical.
    if (cell_p_es[id] >= 1) {
      ldmx::TrigScintHit hit;
      hit.setID(id.raw());
      hit.setPE(cell_p_es[id]);
      hit.setMinPE(cell_min_p_es[id]);
      hit.setAmplitude(cell_p_es[id]);
      hit.setEnergy(dep_energy);
      hit.setTime(time[id]);
      hit.setXPos(xpos[id]);
      hit.setYPos(ypos[id]);
      hit.setZPos(zpos[id]);
      hit.setModuleID(module);
      hit.setBarID(id.bar());  // getFieldValue("bar"));
      hit.setNoise(false);
      hit.setBeamEfrac(beam_frac[id] / dep_energy);

      trig_scint_hits.push_back(hit);
    }

    ldmx_log(trace) << " ID = " << id.raw() << " Edep: " << edep[id]
                    << " numPEs: " << cell_p_es[id] << " time: " << time[id]
                    << " z: " << zpos[id] << "\t X: " << xpos[id]
                    << " Y: " << ypos[id] << " Z: " << zpos[id];
  }

  // ------------------------------- Noise simulation -----------------------//
  // ------------------------------------------------------------------------//
  // only simulating for single array until
  // all arrays are merged into one collection
  int num_empty_cells = stripsPerArray_ - num_rec_hits;
  std::vector<double> noise_hits_pe =
      noiseGenerator_->generateNoiseHits(num_empty_cells);

  ldmx::TrigScintID temp_id;

  for (auto &noise_hit_pe : noise_hits_pe) {
    ldmx::TrigScintHit hit;
    // generate random ID from remaining cells
    do {
      temp_id = generateRandomID(module);
    } while (edep.find(temp_id) != edep.end() ||
             noise_hit_i_ds.find(temp_id) != noise_hit_i_ds.end());

    ldmx::TrigScintID noise_id = temp_id;

    noise_hit_i_ds.insert(noise_id);
    hit.setID(noise_id.raw());
    hit.setPE(noise_hit_pe);
    hit.setMinPE(noise_hit_pe);
    hit.setAmplitude(noise_hit_pe);
    hit.setEnergy(0.);
    hit.setTime(0.);
    hit.setXPos(0.);
    hit.setYPos(0.);
    hit.setZPos(0.);
    hit.setModuleID(module);
    hit.setBarID(noise_id.bar());
    hit.setNoise(true);
    hit.setBeamEfrac(0.);

    trig_scint_hits.push_back(hit);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - -

  event.add(outputCollection_, trig_scint_hits);
}
}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintDigiProducer);
