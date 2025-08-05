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

void EcalRecProducer::configure(framework::config::Parameters& ps) {
  // collection names
  digi_coll_name_ = ps.getParameter<std::string>("digiCollName");
  digi_pass_name_ = ps.getParameter<std::string>("digiPassName");
  sim_hit_coll_name_ = ps.getParameter<std::string>("simHitCollName");
  sim_hit_pass_name_ = ps.getParameter<std::string>("simHitPassName");
  rec_hit_coll_name_ = ps.getParameter<std::string>("recHitCollName");

  layer_weights_ = ps.getParameter<std::vector<double>>("layerWeights");
  second_order_energy_correction_ =
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

  std::vector<ldmx::EcalHit> ecal_rec_hits;
  auto ecal_digis =
      event.getObject<ldmx::HgcrocDigiCollection>(digi_coll_name_, digi_pass_name_);
  // loop through digis
  for (auto digi : ecal_digis) {
    // ID from first digi sample
    //  assuming rest of samples have same ID
    ldmx::EcalID id(digi.id());

    // ID to real space position
    auto [x_, y_, z_] = geometry.getPosition(id);

    // TOA is the time of arrival with respect to the 25ns clock window
    //  TODO what to do if hit NOT in first clock cycle?
    double time_rel_clock25 = digi.soi().toa() * (clock_cycle_ / 1024);  // ns
    double hit_time = time_rel_clock25;

    // get the estimated charge deposited from digi samples
    double charge(0.);

    ldmx_log(trace) << "Recon { "
                    // << "ID: " << id.raw() << ", "
                    << "TOA: " << hit_time << " ns } ";
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

      ldmx_log(trace) << "TOT Mode -> " << digi.tot() << "TDC -> " << charge
                      << " fC";
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

      ldmx_log(trace) << "ADC Mode -> " << charge << " fC";
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
    double energy_deposited_in_si = num_mips_equivalent * mip_si_energy_;

    ldmx_log(trace) << " -> " << num_mips_equivalent << " equiv MIPs -> "
                    << energy_deposited_in_si << " MeV";

    // incorporate layer_ weights
    double reconstructed_energy =
        (num_mips_equivalent *
             layer_weights_.at(
                 id.layer())       // energy lost in non-sensitive layers
         + energy_deposited_in_si  // energy deposited in Si itself
         ) *
        second_order_energy_correction_;

    // copy over information to rec hit structure in new collection
    ldmx::EcalHit rec_hit;
    rec_hit.setID(id.raw());
    rec_hit.setXPos(x_);
    rec_hit.setYPos(y_);
    rec_hit.setZPos(z_);
    rec_hit.setAmplitude(energy_deposited_in_si);
    rec_hit.setEnergy(reconstructed_energy);
    rec_hit.setTime(hit_time);

    ecal_rec_hits.push_back(rec_hit);
  }

  if (event.exists(sim_hit_coll_name_, sim_hit_pass_name_)) {
    // ecal sim hits_ exist ==> label which hits_ are real and which are pure
    // noise
    auto ecal_sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
        sim_hit_coll_name_, sim_hit_pass_name_)};
    std::set<int> real_hits;
    for (auto const& sim_hit : ecal_sim_hits) real_hits.insert(sim_hit.getID());
    for (auto& hit : ecal_rec_hits)
      hit.setNoise(real_hits.find(hit.getID()) == real_hits.end());
  }

  // add collection to event bus
  event.add(rec_hit_coll_name_, ecal_rec_hits);
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalRecProducer);
