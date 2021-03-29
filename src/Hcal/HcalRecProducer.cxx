/**
 * @file HcalRecProducer.cxx
 * @brief Class that performs basic HCal reconstruction
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Hcal/HcalRecProducer.h"

namespace hcal {

HcalRecProducer::HcalRecProducer(const std::string& name,
                                 framework::Process& process)
    : Producer(name, process) {}

void HcalRecProducer::configure(framework::config::Parameters& ps) {
  // collection names
  digiCollName_ = ps.getParameter<std::string>("digiCollName");
  digiPassName_ = ps.getParameter<std::string>("digiPassName");
  simHitCollName_ = ps.getParameter<std::string>("simHitCollName");
  simHitPassName_ = ps.getParameter<std::string>("simHitPassName");
  recHitCollName_ = ps.getParameter<std::string>("recHitCollName");

  mip_energy_ = ps.getParameter<double>("mip_energy");
  pe_per_mip_ = ps.getParameter<double>("pe_per_mip");
  clock_cycle_ = ps.getParameter<double>("clock_cycle");
  voltage_per_mip_ = ps.getParameter<double>("voltage_per_mip");
  pedestal_ = ps.getParameter<double>("pedestal");
  gain_ = ps.getParameter<double>("gain");
  attlength_ = ps.getParameter<double>("attenuationLength");
}

double HcalRecProducer::correctTOA(
    const ldmx::HgcrocDigiCollection::HgcrocDigi digi, int maxSample,
    unsigned int iSOI) {
  // get TOA in ns
  double timeRelClock25 = digi.begin()->toa() * (clock_cycle_ / 1024);  // ns

  // find in which ADC sample the TOA was taken
  int TOASample = (int)timeRelClock25 / 25.;  // divide by 25 ns

  // subtract from however many samples you had to go backward
  // this gives you the TOA relative to the peak bunch
  double TOA = (maxSample - TOASample) * 25 - timeRelClock25;

  // now correct for difference between peak bunch sample and sample of interest
  TOA += (maxSample - (int)iSOI) * 25.;
  return TOA;
}

void HcalRecProducer::produce(framework::Event& event) {
  // Get the Hcal Geometry
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  std::vector<ldmx::HcalHit> hcalRecHits;
  auto hcalDigis =
      event.getObject<ldmx::HgcrocDigiCollection>(digiCollName_, digiPassName_);
  int numDigiHits = hcalDigis.getNumDigis();

  // get sample of interest index
  unsigned int iSOI = hcalDigis.getSampleOfInterestIndex();

  // Loop through digis
  int iDigi = 0;
  while (iDigi < numDigiHits) {
    auto digi = hcalDigis.getDigi(iDigi);

    // ID from first digi sample
    ldmx::HcalDigiID digiId(digi.id());
    ldmx::HcalID id(digiId.section(), digiId.layer(), digiId.strip());

    // Position from ID
    auto position = hcalGeometry.getStripCenterPosition(id);
    double half_total_width = hcalGeometry.getHalfTotalWidth(digiId.section());
    double ecal_dx = hcalGeometry.getEcalDx();
    double ecal_dy = hcalGeometry.getEcalDy();

    // Compute distance to the end of the bar
    // For back Hcal, we take the half of the bar
    // For side Hcal, we take the length of the bar (2*half-width)-Ecal_dxy as
    // an approximation
    float distance_end, distance_ecal;
    if (digiId.section() == ldmx::HcalID::HcalSection::BACK) {
      distance_end = half_total_width;
    } else {
      if ((digiId.section() == ldmx::HcalID::HcalSection::TOP) ||
          (digiId.section() == ldmx::HcalID::HcalSection::BOTTOM))
        distance_ecal = ecal_dx;
      else
        distance_ecal = ecal_dy;
      distance_end = 2 * half_total_width - distance_ecal / 2;
    }

    // Get the estimated voltage and time from digi samples
    double voltage(0.);
    double voltage_min(0.);
    double hitTime(0.);

    // Double readout
    if (digiId.section() == ldmx::HcalID::HcalSection::BACK) {
      auto digi_close = hcalDigis.getDigi(iDigi);
      auto digi_far = hcalDigis.getDigi(iDigi + 1);

      double voltage_close, voltage_far;
      int maxSample_close, maxSample_far;

      if (digi_close.isTOT()) {
        voltage_close = (digi_close.tot() - pedestal_) * gain_;
        voltage_far = (digi_far.tot() - pedestal_) * gain_;
      } else {
        int iSample{0};
        double maxMeas_close{0.}, maxMeas_far{0.};

        for (auto it = digi_close.begin(); it < digi_close.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          if (amplitude > maxMeas_close) {
            maxMeas_close = amplitude;
            maxSample_close = iSample;
          }
          iSample += 1;
        }

        iSample = 0;
        for (auto it = digi_far.begin(); it < digi_far.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          if (amplitude > maxMeas_far) {
            maxMeas_far = amplitude;
            maxSample_far = iSample;
          }
          iSample += 1;
        }
        voltage_close = maxMeas_close;
        voltage_far = maxMeas_far;
      }

      // correct TOA
      double TOA_close = correctTOA(digi_close, maxSample_close, iSOI);
      double TOA_far = correctTOA(digi_far, maxSample_far, iSOI);

      // get x(y) coordinate from TOA measurement
      // position in bar = (diff_time*v)/2
      double v =
          299.792 / 1.6;  // velocity of light in polystyrene, n = 1.6 = c/v
      double pos = (TOA_far - TOA_close) * v / 2;
      // If the close pulse has a negative end (digiId should correspond to this
      // pulse), multiply the position by -1.
      if (digiId.isNegativeEnd()) pos = pos * -1;

      // reverse voltage attenuation
      double att_close =
          exp(-1. * ((distance_end - fabs(pos)) / 1000.) / attlength_);
      double att_far =
          exp(-1. * ((distance_end + fabs(pos)) / 1000.) / attlength_);

      // set voltage
      voltage = (voltage_close / att_close + voltage_far / att_far) / 2;  // mV
      voltage_min = std::min(voltage_close / att_close, voltage_far / att_far);

      // set position
      if ((digiId.layer() % 2) == 1) {
        position.SetX(pos);
      } else {
        position.SetY(pos);
      }

      // set hit time
      // TODO: does this need to revert shift because of propagation of light in
      // polysterene?
      hitTime = fabs(TOA_close + TOA_far) / 2;  // ns

      iDigi += 2;
    }       // end double readout loop
    else {  // single readout

      double voltage_i;
      int maxSample_i;
      if (digi.isTOT()) {
        // TOT - number of clock ticks that pulse was over threshold
        // this is related to the amplitude of the pulse approximately through a
        // linear drain rate the amplitude of the pulse is related to the energy
        // deposited

        // convert the time over threshold into a total energy deposited in the
        // bar (time over threshold [ns] - pedestal) * gain
        voltage_i = (digi.tot() - pedestal_) * gain_;
      } else {
        // ADC mode of readout
        // ADC - voltage measurement at a specific time of the pulse
        double maxMeas{0.};
        int iSample{0};
        for (auto it = digi.begin(); it < digi.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          if (amplitude > maxMeas) {
            maxMeas = amplitude;
            maxSample_i = iSample;
          }
          iSample += 1;
        }
        // just use the maximum measured voltage
        voltage_i = maxMeas;  // mV
      }

      // reverse voltage attenuation
      // for now, assume fabs(pos) = half_total_width as an approximation
      double att = exp(-1. * ((distance_end - fabs(half_total_width)) / 1000.) /
                       attlength_);

      // set voltage
      voltage = voltage_i / att;
      voltage_min = voltage_i / att;

      // correct TOA
      double TOA = correctTOA(digi, maxSample_i, iSOI);

      // set hit time
      // TODO: does this need to revert shift because of propagation of light in
      // polysterene?
      hitTime = TOA;  // ns

      iDigi++;
    }  // end single readout loop

    double num_mips_equivalent = voltage / voltage_per_mip_;
    double energy_deposited = num_mips_equivalent * mip_energy_;

    // TODO: need to incorporate corrections if necessary
    double reconstructed_energy = energy_deposited;

    int PEs = num_mips_equivalent * pe_per_mip_;
    int minPEs = (voltage_min / voltage_per_mip_) * pe_per_mip_;

    // copy over information to rec hit structure in new collection
    ldmx::HcalHit recHit;
    recHit.setID(id.raw());
    recHit.setXPos(position.X());
    recHit.setYPos(position.Y());
    recHit.setZPos(position.Z());
    recHit.setPE(PEs);
    recHit.setMinPE(minPEs);
    recHit.setAmplitude(PEs);
    recHit.setEnergy(energy_deposited);
    recHit.setTime(hitTime);
    hcalRecHits.push_back(recHit);
  }

  if (event.exists(simHitCollName_, simHitPassName_)) {
    // hcal sim hits exist ==> label which hits are real and which are pure
    // noise
    auto hcalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(
        simHitCollName_, simHitPassName_)};
    std::set<int> real_hits;
    for (auto const& sim_hit : hcalSimHits) real_hits.insert(sim_hit.getID());
    for (auto& hit : hcalRecHits)
      hit.setNoise(real_hits.find(hit.getID()) == real_hits.end());
  }

  // add collection to event bus
  event.add(recHitCollName_, hcalRecHits);
}

}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalRecProducer);
