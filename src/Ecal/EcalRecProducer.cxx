/**
 * @file EcalRecProducer.cxx
 * @brief Class that performs basic ECal reconstruction
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/EcalRecProducer.h"
#include "DetDescr/EcalGeometry.h"
#include "Ecal/EcalReconConditions.h"
#include "Ecal/Event/EcalHit.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace ecal {

EcalRecProducer::EcalRecProducer(const std::string& name,
                                 framework::Process& process)
    : Producer(name, process) {}

EcalRecProducer::~EcalRecProducer() {}

void EcalRecProducer::configure(framework::config::Parameters& ps) {
  // collection names
  digiCollName_ = ps.getParameter<std::string>("digiCollName");
  digiPassName_ = ps.getParameter<std::string>("digiPassName");
  simHitCollName_ = ps.getParameter<std::string>("simHitCollName");
  simHitPassName_ = ps.getParameter<std::string>("simHitPassName");
  recHitCollName_ = ps.getParameter<std::string>("recHitCollName");

  layerWeights_ = ps.getParameter<std::vector<double>>("layerWeights");
  secondOrderEnergyCorrection_ =
      ps.getParameter<double>("secondOrderEnergyCorrection");

  mip_si_energy_ = ps.getParameter<double>("mip_si_energy");
  clock_cycle_ = ps.getParameter<double>("clock_cycle");
  charge_per_mip_ = ps.getParameter<double>("charge_per_mip");
}

void EcalRecProducer::produce(framework::Event& event) {
  // Get the Ecal Geometry
  const auto& geometry = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the reconstruction parameters
  EcalReconConditions the_conditions(
      getCondition<conditions::DoubleTableCondition>(
          EcalReconConditions::CONDITIONS_NAME));

  std::vector<ldmx::EcalHit> ecalRecHits;
  auto ecalDigis =
      event.getObject<ldmx::HgcrocDigiCollection>(digiCollName_, digiPassName_);
  // loop through digis
  for (auto digi : ecalDigis) {

    // ID from first digi sample
    //  assuming rest of samples have same ID
    ldmx::EcalID id(digi.id());

    // ID to real space position
    auto [x,y,z] = geometry.getPosition(id);

    // TOA is the time of arrival with respect to the 25ns clock window
    //  TODO what to do if hit NOT in first clock cycle?
    double timeRelClock25 = digi.soi().toa() * (clock_cycle_ / 1024);  // ns
    double hitTime = timeRelClock25;

    // get the estimated charge deposited from digi samples
    double charge(0.);

    /* debug printout
    std::cout << "Recon { "
        << "ID: " << id << ", "
        << "TOA: " << hitTime << "ns } ";
        */
    if (digi.isTOT()) {
      // TOT - number of clock ticks that pulse was over threshold
      //  this is related to the amplitude of the pulse approximately through a
      //  linear drain rate the amplitude of the pulse is related to the energy
      //  deposited

      // convert the time over threshold into a total energy deposited in the
      // silicon
      //  (time over threshold [ns] - pedestal) * gain
      charge = (digi.tot() - the_conditions.totPedestal(id)) *
               the_conditions.totGain(id);

      /* debug printout
      std::cout << "TOT Mode -> " << digi.tot() << "TDC -> " << charge << " fC";
       */
    } else {
      // ADC mode of readout
      // ADC - voltage measurement at a specific time of the pulse
      // Pulse Shape:
      //  p[0]/(1.0+exp(p[1](t-p[2]+p[3]-p[4])))/(1.0+exp(p[5]*(t-p[6]+p[3]-p[4])))
      //  p[0] = amplitude to be fit (TBD)
      //  p[1] = -0.345 shape parameter - rate of up slope
      //  p[2] = 70.6547 shape parameter - time of up slope relative to shape
      //  fit p[3] = 77.732 shape parameter - time of peak relative to shape fit
      //  p[4] = peak time to be fit (TBD)
      //  p[5] = 0.140068 shape parameter - rate of down slope
      //  p[6] = 87.7649 shape paramter - time of down slope relative to shape
      //  fit
      // These measurements can be used to fit the pulse shape if TOT is not
      // available. For now, we simply take the measurement of the SOI as the
      // peak amplitude.

      charge = (digi.soi().adc_t() - the_conditions.adcPedestal(id)) *
               the_conditions.adcGain(id);

      /* debug printout
      std::cout << "ADC Mode -> " << charge << " fC";
       */
    }

    /** Negative Electron (charge) count
     * This reconstruction error occurs when the ADC value
     * is below the ADC pedestal for that channel. In the
     * normal running mode, this will never happen because
     * our front-end (the digi emulator or the digitizer itself)
     * will suppress any signals that are below the readout
     * threshold. Nevertheless, in some running modes, we
     * don't have this zero suppression, so we need to 
     * check that the reconstruction charge (count of electrons)
     * is non-negative.
     */
    if (charge < 0) continue;

    double num_mips_equivalent = charge / charge_per_mip_;
    double energy_deposited_in_Si = num_mips_equivalent * mip_si_energy_;

    /* debug printout
    std::cout << " -> " << num_mips_equivalent
        << " equiv MIPs -> " << energy_deposited_in_Si << " MeV"
        << std::endl;
     */

    // incorporate layer weights
    double reconstructed_energy =
        (num_mips_equivalent *
             layerWeights_.at(
                 id.layer())       // energy lost in non-sensitive layers
         + energy_deposited_in_Si  // energy deposited in Si itself
         ) *
        secondOrderEnergyCorrection_;

    // copy over information to rec hit structure in new collection
    ldmx::EcalHit recHit;
    recHit.setID(id.raw());
    recHit.setXPos(x);
    recHit.setYPos(y);
    recHit.setZPos(z);
    recHit.setAmplitude(energy_deposited_in_Si);
    recHit.setEnergy(reconstructed_energy);
    recHit.setTime(hitTime);

    ecalRecHits.push_back(recHit);
  }

  if (event.exists(simHitCollName_, simHitPassName_)) {
    // ecal sim hits exist ==> label which hits are real and which are pure
    // noise
    auto ecalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(
        simHitCollName_, simHitPassName_)};
    std::set<int> real_hits;
    for (auto const& sim_hit : ecalSimHits) real_hits.insert(sim_hit.getID());
    for (auto& hit : ecalRecHits)
      hit.setNoise(real_hits.find(hit.getID()) == real_hits.end());
  }

  // add collection to event bus
  event.add(recHitCollName_, ecalRecHits);
}

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalRecProducer);
