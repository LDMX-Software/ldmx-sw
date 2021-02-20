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

HcalRecProducer::~HcalRecProducer() {}

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

void HcalRecProducer::produce(framework::Event& event) {
  // Get the Hcal Geometry
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  std::vector<ldmx::HcalHit> hcalRecHits;
  auto hcalDigis =
      event.getObject<ldmx::HgcrocDigiCollection>(digiCollName_, digiPassName_);
  int numDigiHits = hcalDigis.getNumDigis();

  // Loop through digis
  int iDigi = 0;
  while (iDigi < numDigiHits) {
    auto digi = hcalDigis.getDigi(iDigi);

    // ID from first digi sample
    ldmx::HcalDigiID digiId(digi.id());
    ldmx::HcalID id(digiId.section(), digiId.layer(), digiId.strip());

    // Position from ID
    auto position = hcalGeometry.getStripCenterPosition(id);

    // TOA is the time of arrival with respect to the 25ns clock window
    // TODO what to do if hit NOT in first clock cycle?
    double timeRelClock25 = digi.begin()->toa() * (clock_cycle_ / 1024);  // ns
    double hitTime = timeRelClock25;

    // get the estimated voltage from digi samples
    double voltage(0.);
    double voltage_min(0.);

    // double readout
    if (digiId.section() == ldmx::HcalID::HcalSection::BACK) {
      auto digi_close = hcalDigis.getDigi(iDigi);
      auto digi_far = hcalDigis.getDigi(iDigi + 1);

      // DigiID (only the close one for now)
      ldmx::HcalDigiID id_close(digi.id());

      // get x(y) coordinate from TOA
      double v =
          299.792 / 1.6;  // velocity of light in polystyrene, n = 1.6 = c/v
                          // (here, Ralf's simulation should be included)
      double half_total_width =
          hcalGeometry.getHalfTotalWidth(digiId.section());

      // position in bar = (diff_time*v)/2;
      double timeRelClock25_close =
          digi_close.begin()->toa() * (clock_cycle_ / 1024);  // ns
      double timeRelClock25_far =
          digi_far.begin()->toa() * (clock_cycle_ / 1024);  // ns
      double pos = (timeRelClock25_far - timeRelClock25_close) * v / 2;
      if (id_close.isNegativeEnd()) pos = pos * -1;
      if ((digiId.layer() % 2) == 1)
        position.SetX(pos);
      else
        position.SetY(pos);

      // time
      hitTime = fabs(timeRelClock25_close + timeRelClock25_far) / 2;

      // attenuation
      double att_close =
          exp(-1 * ((half_total_width - fabs(pos)) / 1000.) / attlength_);
      double att_far =
          exp(-1 * ((half_total_width + fabs(pos)) / 1000.) / attlength_);

      if (digi_close.isTOT()) {
        double voltage_close = (digi_close.tot() - pedestal_) * gain_;
        double voltage_far = (digi_far.tot() - pedestal_) * gain_;
        voltage =
            (voltage_close / att_close + voltage_far / att_far) / 2;  // mV
        voltage_min =
            std::min(voltage_close / att_close, voltage_far / att_far);
      } else {
        double maxMeas_close{0.};
        int numWholeClocks{0};
        for (auto it = digi_close.begin(); it < digi_close.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          double time = numWholeClocks * clock_cycle_;  // ns
          if (amplitude > maxMeas_close) maxMeas_close = amplitude;
        }
        double maxMeas_far{0.};
        numWholeClocks = 0;
        for (auto it = digi_far.begin(); it < digi_far.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          double time = numWholeClocks * clock_cycle_;  // ns
          if (amplitude > maxMeas_far) maxMeas_far = amplitude;
        }
        voltage =
            (maxMeas_close / att_close + maxMeas_far / att_far) / 2;  // mV
        voltage_min =
            std::min(maxMeas_close / att_close, maxMeas_far / att_far);
      }

      iDigi += 2;
    }       // end double readout loop
    else {  // single readout

      if (digi.isTOT()) {
        // TOT - number of clock ticks that pulse was over threshold
        // this is related to the amplitude of the pulse approximately through a
        // linear drain rate the amplitude of the pulse is related to the energy
        // deposited

        // convert the time over threshold into a total energy deposited in the
        // bar (time over threshold [ns] - pedestal) * gain
        voltage = (digi.tot() - pedestal_) * gain_;
      } else {
        // ADC mode of readout
        // ADC - voltage measurement at a specific time of the pulse
        TH1F measurements("measurements", "measurements", 10. * clock_cycle_,
                          0., 10. * clock_cycle_);
        double maxMeas{0.};
        int numWholeClocks{0};
        for (auto it = digi.begin(); it < digi.end(); it++) {
          double amplitude_fC = (it->adc_t() - pedestal_) * gain_;
          double time =
              numWholeClocks * clock_cycle_;  //+ offestWithinClock; //ns
          measurements.Fill(time, amplitude_fC);
          if (amplitude_fC > maxMeas) maxMeas = amplitude_fC;
        }
        // just use the maximum measured voltage
        voltage = maxMeas;
      }
      voltage_min = voltage;

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
    recHit.setNoise(false);
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
