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
  ldmx::TrigScintID tempID(module, strips_dist(rng_));
  if (module >= TrigScintSection::NUM_SECTIONS) {
    ldmx_log(fatal) << "TrigScintSection is not known";
  }

  return tempID;
}

void TrigScintDigiProducer::produce(framework::Event &event) {
  std::map<ldmx::TrigScintID, int> cellPEs, cellMinPEs;
  std::map<ldmx::TrigScintID, float> Xpos, Ypos, Zpos, Edep, Time, beamFrac;
  std::set<ldmx::TrigScintID> noiseHitIDs;

  auto numRecHits{0};

  // looper over sim hits and aggregate energy depositions for each detID
  const auto simHits{event.getCollection<ldmx::SimCalorimeterHit>(
      inputCollection_, inputPassName_)};
  auto particleMap{event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_passname_)};

  int module{-1};
  for (const auto &simHit : simHits) {
    ldmx::TrigScintID id(simHit.getID());

    // Just set the module ID to use for noise hits here.  Given that
    // we are currently processing a single module at a time, setting
    // it within the loop shouldn't matter.
    module = id.module();
    std::vector<float> position = simHit.getPosition();
    ldmx_log(trace) << " Modedule ID = " << id.raw();

    // check if hits is from beam electron and, if so, add to beamFrac
    for (int i = 0; i < simHit.getNumberOfContribs(); i++) {
      auto contrib = simHit.getContrib(i);

      ldmx_log(trace) << "contrib " << i << " trackID: " << contrib.trackID
                      << " pdgID: " << contrib.pdgCode
                      << " edep: " << contrib.edep;
      ldmx_log(trace) << "\t particle id: "
                      << particleMap[contrib.trackID].getPdgID()
                      << " particle status: "
                      << particleMap[contrib.trackID].getGenStatus();

      if (particleMap[contrib.trackID].getPdgID() == 11 &&
          particleMap[contrib.trackID].getGenStatus() == 1) {
        if (beamFrac.find(id) == beamFrac.end()) {
          beamFrac[id] = contrib.edep;
        } else {
          beamFrac[id] += contrib.edep;
        }
      }
    }

    // for now, we take an energy weighted average of the hit in each strip to
    // simulate the hit position. AJW: these should be dropped, they are likely
    // to lead to a problem since we can't measure them anyway except roughly y
    // and z, which is encoded in the ids.
    if (Edep.find(id) == Edep.end()) {
      // first hit, initialize
      Edep[id] = simHit.getEdep();
      Time[id] = simHit.getTime() * simHit.getEdep();
      Xpos[id] = position[0] * simHit.getEdep();
      Ypos[id] = position[1] * simHit.getEdep();
      Zpos[id] = position[2] * simHit.getEdep();
      numRecHits++;

    } else {
      // not first hit, aggregate, and store the largest radius hit
      Xpos[id] += position[0] * simHit.getEdep();
      Ypos[id] += position[1] * simHit.getEdep();
      Zpos[id] += position[2] * simHit.getEdep();
      Edep[id] += simHit.getEdep();
      // AJW: need to figure out a better way to model this...
      Time[id] += simHit.getTime() * simHit.getEdep();
    }
  }

  // Create the container to hold the digitized trigger scintillator hits.
  std::vector<ldmx::TrigScintHit> trigScintHits;

  // loop over detIDs and simulate number of PEs
  for (std::map<ldmx::TrigScintID, float>::iterator it = Edep.begin();
       it != Edep.end(); ++it) {
    ldmx::TrigScintID id(it->first);

    double depEnergy = Edep[id];
    Time[id] = Time[id] / Edep[id];
    Xpos[id] = Xpos[id] / Edep[id];
    Ypos[id] = Ypos[id] / Edep[id];
    Zpos[id] = Zpos[id] / Edep[id];
    double meanPE = depEnergy / mevPerMip_ * pePerMip_;
    std::poisson_distribution<int> poisson_dist(meanPE + meanNoise_);
    cellPEs[id] = poisson_dist(rng_);

    // If a cell has a PE count above threshold, persit the hit.
    // Thresholds are introduced (and configurable) in clustering.
    // the cell PE >=1 suppresses artifical noise that is below one light
    // quantum in the SiPM and unphysical.
    if (cellPEs[id] >= 1) {
      ldmx::TrigScintHit hit;
      hit.setID(id.raw());
      hit.setPE(cellPEs[id]);
      hit.setMinPE(cellMinPEs[id]);
      hit.setAmplitude(cellPEs[id]);
      hit.setEnergy(depEnergy);
      hit.setTime(Time[id]);
      hit.setXPos(Xpos[id]);
      hit.setYPos(Ypos[id]);
      hit.setZPos(Zpos[id]);
      hit.setModuleID(module);
      hit.setBarID(id.bar());  // getFieldValue("bar"));
      hit.setNoise(false);
      hit.setBeamEfrac(beamFrac[id] / depEnergy);

      trigScintHits.push_back(hit);
    }

    ldmx_log(trace) << " ID = " << id.raw() << " Edep: " << Edep[id]
                    << " numPEs: " << cellPEs[id] << " time: " << Time[id]
                    << " z: " << Zpos[id] << "\t X: " << Xpos[id]
                    << " Y: " << Ypos[id] << " Z: " << Zpos[id];
  }

  // ------------------------------- Noise simulation -----------------------//
  // ------------------------------------------------------------------------//
  // only simulating for single array until
  // all arrays are merged into one collection
  int numEmptyCells = stripsPerArray_ - numRecHits;
  std::vector<double> noiseHits_PE =
      noiseGenerator_->generateNoiseHits(numEmptyCells);

  ldmx::TrigScintID tempID;

  for (auto &noiseHitPE : noiseHits_PE) {
    ldmx::TrigScintHit hit;
    // generate random ID from remaining cells
    do {
      tempID = generateRandomID(module);
    } while (Edep.find(tempID) != Edep.end() ||
             noiseHitIDs.find(tempID) != noiseHitIDs.end());

    ldmx::TrigScintID noiseID = tempID;

    noiseHitIDs.insert(noiseID);
    hit.setID(noiseID.raw());
    hit.setPE(noiseHitPE);
    hit.setMinPE(noiseHitPE);
    hit.setAmplitude(noiseHitPE);
    hit.setEnergy(0.);
    hit.setTime(0.);
    hit.setXPos(0.);
    hit.setYPos(0.);
    hit.setZPos(0.);
    hit.setModuleID(module);
    hit.setBarID(noiseID.bar());
    hit.setNoise(true);
    hit.setBeamEfrac(0.);

    trigScintHits.push_back(hit);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - -

  event.add(outputCollection_, trigScintHits);
}
}  // namespace trigscint

DECLARE_PRODUCER(trigscint::TrigScintDigiProducer);
