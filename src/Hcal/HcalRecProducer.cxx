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
  // get toa relative to the startBX
  double toaRelStartBX(0.);
  int toaSample(0), iADC(0);
  for (auto it = digi.begin(); it < digi.end(); it++) {
    if (it->toa() > 0) {
      toaRelStartBX = it->toa() * (clock_cycle_ / 1024);  // ns
      // find in which ADC sample the TOA was taken
      toaSample = iADC;
    }
    iADC++;
  }

  // time w.r.t to the peak bunch
  // difference between time @ max. amplitude and time @ threshold (TOA)
  // double corr = (maxSample - toaSample) * clock_cycle_ - toaRelStartBX;

  // plus check if the max. amplitude sample is different from the SOI
  // corr += (maxSample - (int)iSOI) * 25.;

  // std::cout << "toa rel to the startBX " << toaRelClock25 << " which ADC
  // sample "
  //<< toaSample << std::endl;
  // std::cout << "max sampel " << maxSample << " toaSample " << toaSample <<
  // std::endl; std::cout << "timeWalk corr " << corr << std::endl;

  return toaRelStartBX;
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
    float distance_posend, distance_negend, distance_ecal;
    if (digiId.section() == ldmx::HcalID::HcalSection::BACK) {
      distance_posend = half_total_width;
      distance_negend = half_total_width;
    } else {
      if ((digiId.section() == ldmx::HcalID::HcalSection::TOP) ||
          (digiId.section() == ldmx::HcalID::HcalSection::BOTTOM))
        distance_ecal = ecal_dx;
      else
        distance_ecal = ecal_dy;
      distance_posend = 2 * half_total_width - distance_ecal / 2.;
      distance_negend = distance_ecal / 2.;
    }

    // Get the estimated voltage and time from digi samples
    double voltage(0.);
    double voltage_min(0.);
    double hitTime(0.);

    // Double readout
    if (digiId.section() == ldmx::HcalID::HcalSection::BACK) {
      auto digi_posend = hcalDigis.getDigi(iDigi);
      auto digi_negend = hcalDigis.getDigi(iDigi + 1);

      double voltage_posend, voltage_negend;
      int maxSample_posend, maxSample_negend;

      if (digi_posend.isTOT()) {
        voltage_posend = (digi_posend.tot() - pedestal_) * gain_;
        voltage_negend = (digi_negend.tot() - pedestal_) * gain_;
      } else {
        int iADC{0};
        double maxMeas_posend{0.}, maxMeas_negend{0.};

        for (auto it = digi_posend.begin(); it < digi_posend.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          if (amplitude > maxMeas_posend) {
            maxMeas_posend = amplitude;
            maxSample_posend = iADC;
          }
          iADC += 1;
        }

        iADC = 0;
        for (auto it = digi_negend.begin(); it < digi_negend.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          if (amplitude > maxMeas_negend) {
            maxMeas_negend = amplitude;
            maxSample_negend = iADC;
          }
          iADC += 1;
        }
        voltage_posend = maxMeas_posend;
        voltage_negend = maxMeas_negend;
      }

      // correct TOA
      double TOA_posend = correctTOA(digi_posend, maxSample_posend, iSOI);
      double TOA_negend = correctTOA(digi_negend, maxSample_negend, iSOI);
      // std::cout << "TOA posend " << TOA_posend << " negend " << TOA_negend
      //           << std::endl;

      // get x(y) coordinate from TOA measurement
      // position in bar = (diff_time*v)/2
      // if time_posend < time_negend: position is positive
      double v =
          299.792 / 1.6;  // velocity of light in polystyrene, n = 1.6 = c/v
      double pos = (TOA_negend - TOA_posend) * v / 2;

      // reverse voltage attenuation
      // if pos is positive, then the positive end will have less attenuation
      // than the negative end
      double att_posend =
          exp(-1. * ((distance_posend - pos) / 1000.) / attlength_);
      double att_negend =
          exp(-1. * ((distance_negend + pos) / 1000.) / attlength_);

      // set voltage
      voltage = (voltage_posend / att_posend + voltage_negend / att_negend) /
                2;  // mV
      voltage_min =
          std::min(voltage_posend / att_posend, voltage_negend / att_negend);

      // set position
      if ((digiId.layer() % 2) == 1) {
        position.SetX(pos);
      } else {
        position.SetY(pos);
      }

      // set hit time
      // TODO: does this need to revert shift because of propagation of light in
      // polysterene?
      hitTime = fabs(TOA_posend + TOA_negend) / 2;  // ns

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
        int iADC{0};
        for (auto it = digi.begin(); it < digi.end(); it++) {
          double amplitude = (it->adc_t() - pedestal_) * gain_;
          if (amplitude > maxMeas) {
            maxMeas = amplitude;
            maxSample_i = iADC;
          }
          iADC += 1;
        }
        // just use the maximum measured voltage
        voltage_i = maxMeas;  // mV
      }

      // reverse voltage attenuation
      // for now, assume that position along the bar is the half_total_width
      double distance_end =
          digiId.isNegativeEnd() ? distance_negend : distance_posend;
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
