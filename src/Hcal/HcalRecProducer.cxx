/**
 * @file HcalRecProducer.cxx
 * @brief Class that performs basic HCal reconstruction
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Hcal/HcalRecProducer.h"

#include "Hcal/HcalReconConditions.h"

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

  // parameters
  mip_energy_ = ps.getParameter<double>("mip_energy");
  pe_per_mip_ = ps.getParameter<double>("pe_per_mip");
  clock_cycle_ = ps.getParameter<double>("clock_cycle");
  voltage_per_mip_ = ps.getParameter<double>("voltage_per_mip");
  attlength_ = ps.getParameter<double>("attenuationLength");
  nADCs_ = ps.getParameter<int>("nADCs");

  // configuring corrections graphs derived on the fly
  // TODO: maybe we should save these as a graph instead?
  rateUpSlope_ = ps.getParameter<double>("rateUpSlope");
  timeUpSlope_ = ps.getParameter<double>("timeUpSlope");
  rateDnSlope_ = ps.getParameter<double>("rateDnSlope");
  timeDnSlope_ = ps.getParameter<double>("timeDnSlope");
  timePeak_ = ps.getParameter<double>("timePeak");
  pulseFunc_ =
      TF1("pulseFunc",
          "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/"
          "((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
          (double)nADCs_ * clock_cycle_ * -1, (double)nADCs_ * clock_cycle_);
  pulseFunc_.FixParameter(1, rateUpSlope_);
  pulseFunc_.FixParameter(2, timeUpSlope_);
  pulseFunc_.FixParameter(3, timePeak_);
  pulseFunc_.FixParameter(5, rateDnSlope_);
  pulseFunc_.FixParameter(6, timeDnSlope_);
  pulseFunc_.FixParameter(4, 0);
  pulseFunc_.FixParameter(0, 1);

  // build amplitude correction (Ampl[t-1]/Ampl[t]) with pulse-shape
  int n = 0;
  for (double t = -clock_cycle_; t < clock_cycle_; t += 0.01) {
    double ampl_t = pulseFunc_.Eval(t);
    double ampl_tm1 = pulseFunc_.Eval(t - clock_cycle_);
    if (ampl_tm1 > ampl_t) continue;
    correctionAmpl_.SetPoint(n, ampl_tm1 / ampl_t, ampl_t);
    if (n == 0) minAmplFraction_ = ampl_tm1 / ampl_t;
    n++;
  }

  // build TOA timewalk correction with pulse-shape
  double toaThreshold = ps.getParameter<double>("avgToaThreshold");
  double gain = ps.getParameter<double>("avgGain");
  double pedestal = ps.getParameter<double>("avgPedestal");
  n = 0;
  for (double ampl = toaThreshold + 0.1; ampl < 10000; ampl += 0.01) {
    pulseFunc_.FixParameter(0, ampl);
    double ampl_t = gain * pedestal + pulseFunc_.Eval(0);
    double toa =
        fabs(pulseFunc_.GetX(toaThreshold, (double)nADCs_ * clock_cycle_ * -1,
                             (double)nADCs_ * clock_cycle_));
    correctionTOA_.SetPoint(n, ampl_t, toa);
    if (n == 0) minAmpl_ = ampl_t;
    n++;
  }
}

double HcalRecProducer::getTOA(
    const ldmx::HgcrocDigiCollection::HgcrocDigi digi, double pedestal,
    unsigned int iSOI) {
  // get toa relative to the startBX
  double toaRelStartBX(0.), maxMeas{0.};
  int toaSample(0), maxSample(0), iADC(0);
  for (int i_sample{0}; i_sample < digi.size(); i_sample++) {
    auto sample{digi.at(i_sample)};
    if (sample.toa() > 0) {
      toaRelStartBX = sample.toa() * (clock_cycle_ / 1024);  // ns
      // find in which ADC sample the TOA was taken
      toaSample = iADC;
    }
    if ((sample.adc_t() - pedestal) > maxMeas) {
      maxMeas = (sample.adc_t() - pedestal);
      maxSample = iADC;
    }
    iADC++;
  }

  // time w.r.t to the peak
  double toa = (maxSample - toaSample) * clock_cycle_ - toaRelStartBX;

  // time w.r.t to the SOI
  toa += ((int)iSOI - maxSample) * clock_cycle_;

  return toa;
}

void HcalRecProducer::produce(framework::Event& event) {
  // get the Hcal Geometry
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  // get the reconstruction parameters
  const auto& the_conditions{
      getCondition<HcalReconConditions>(HcalReconConditions::CONDITIONS_NAME)};

  std::vector<ldmx::HcalHit> hcalRecHits;
  auto hcalDigis =
      event.getObject<ldmx::HgcrocDigiCollection>(digiCollName_, digiPassName_);
  int numDigiHits = hcalDigis.getNumDigis();

  // get sample of interest index
  unsigned int iSOI = hcalDigis.getSampleOfInterestIndex();

  // loop through digis
  int iDigi = 0;
  while (iDigi < numDigiHits) {
    auto digi_posend = hcalDigis.getDigi(iDigi);

    // ID from first digi sample (which should be in positive end)
    ldmx::HcalDigiID id_posend(digi_posend.id());
    ldmx::HcalID id(id_posend.section(), id_posend.layer(), id_posend.strip());

    // position from ID
    auto position = hcalGeometry.getStripCenterPosition(id);
    double half_total_width =
        hcalGeometry.getHalfTotalWidth(id.section(), id.layer());
    double ecal_dx = hcalGeometry.getEcalDx();
    double ecal_dy = hcalGeometry.getEcalDy();

    // compute distance to the end of the bar
    // for back Hcal, we take the half of the bar
    // for side Hcal, we take the length of the bar (2*half-width)-Ecal_dxy as
    // an approximation
    float distance_posend, distance_negend, distance_ecal;
    if (id.section() == ldmx::HcalID::HcalSection::BACK) {
      distance_posend = half_total_width;
      distance_negend = half_total_width;
    } else {
      if ((id.section() == ldmx::HcalID::HcalSection::TOP) ||
          (id.section() == ldmx::HcalID::HcalSection::BOTTOM))
        distance_ecal = ecal_dx;
      else
        distance_ecal = ecal_dy;
      distance_posend = 2 * half_total_width - distance_ecal / 2.;
      distance_negend = distance_ecal / 2.;
    }

    // get the estimated voltage and time from digi samples
    double voltage(0.);
    double voltage_min(0.);
    double hitTime(0.);

    double amplT(0.);
    double amplT_posend(0.), amplTm1_posend(0.);
    double amplT_negend(0.), amplTm1_negend(0.);

    // double readout
    if (id.section() == ldmx::HcalID::HcalSection::BACK) {
      auto digi_negend = hcalDigis.getDigi(iDigi + 1);
      ldmx::HcalDigiID id_negend(digi_negend.id());

      double voltage_posend, voltage_negend;
      double amplT_posend_copy, amplT_negend_copy;
      if (digi_posend.isTOT()) {
        voltage_posend =
            (digi_posend.tot() - the_conditions.totCalib(id_posend, 0)) *
            the_conditions.totCalib(id_posend, 1);
        voltage_negend =
            (digi_negend.tot() - the_conditions.totCalib(id_negend, 0)) *
            the_conditions.totCalib(id_negend, 1);
      } else {
        amplT_posend =
            digi_posend.soi().adc_t() - the_conditions.adcPedestal(id_posend);
        amplTm1_posend =
            digi_posend.soi().adc_tm1() - the_conditions.adcPedestal(id_posend);
        amplT_negend =
            digi_negend.soi().adc_t() - the_conditions.adcPedestal(id_negend);
        amplTm1_negend =
            digi_negend.soi().adc_tm1() - the_conditions.adcPedestal(id_negend);

        amplT_posend_copy = amplT_posend;
        amplT_negend_copy = amplT_negend;

        // correct amplitude (amplitude fractions from both ends need to be
        // above the boundary of the correction)
        if (amplTm1_posend / amplT_posend > minAmplFraction_ &&
            amplTm1_negend / amplT_negend > minAmplFraction_) {
          amplT_posend *= correctionAmpl_.Eval(amplTm1_posend / amplT_posend);
          amplT_negend *= correctionAmpl_.Eval(amplTm1_negend / amplT_negend);
        }

        // set voltage
	voltage_posend = amplT_posend * the_conditions.adcGain(id_posend, 0);
        voltage_negend = amplT_negend * the_conditions.adcGain(id_negend, 0);
      }

      // get TOA
      double TOA_posend =
          getTOA(digi_posend, the_conditions.adcPedestal(id_posend), iSOI);
      double TOA_negend =
          getTOA(digi_negend, the_conditions.adcPedestal(id_negend), iSOI);

      // get sign of position along the bar
      int position_bar_sign = (TOA_posend - TOA_negend) > 0 ? 1 : -1;

      double TOA_posend_copy = TOA_posend;
      double TOA_negend_copy = TOA_negend;

      // correct TOA
      // amplitudes from both ends need to be above the boundary of the
      // correction otherwise, one TOA gets corrected and the other does not,
      // which results in a large TOA difference and an out-of-bounds position
      if (amplT_posend > minAmpl_ && amplT_negend > minAmpl_) {
        TOA_posend = correctionTOA_.Eval(amplT_posend) - TOA_posend;
        TOA_negend = correctionTOA_.Eval(amplT_negend) - TOA_negend;
      }

      // get x(y) coordinate from TOA measurement = (dt*v/2)
      // if time_posend < time_negend: position is positive
      double v =
          299.792 / 1.6;  // velocity of light in polystyrene, n = 1.6 = c/v
      double position_bar =
          position_bar_sign * fabs(TOA_posend - TOA_negend) * v / 2;

      // reverse voltage attenuation
      // if position along the bar is positive, then the positive end will have
      // less attenuation than the negative end
      // NOTE: For now, reverse attenuation is not applied to the energy
      // deposited since both ends of the bar are summed.
      double att_posend =
          exp(-1. * ((distance_posend - position_bar) / 1000.) / attlength_);
      double att_negend =
          exp(-1. * ((distance_negend + position_bar) / 1000.) / attlength_);

      // set voltage as the sum of both bars
      voltage = (voltage_posend + voltage_negend);
      voltage_min = std::min(voltage_posend, voltage_negend);

      // set amplitude as the average of both bars (reverse attenuated)
      amplT = (amplT_posend / att_posend + amplT_negend / att_negend) / 2;

      // set position along the bar
      if (hcalGeometry.layerIsHorizontal(id_posend.layer())) {
        position.SetX(position_bar);
      } else {
        position.SetY(position_bar);
      }

      // set hit time
      // TODO: does this need to revert shift because of propagation of light in
      // polysterene?
      hitTime = fabs(TOA_posend + TOA_negend) / 2;  // ns

      iDigi += 2;
    }       // end double readout loop
    else {  // single readout

      double voltage_i;
      if (digi_posend.isTOT()) {
        // TOT - number of clock ticks that pulse was over threshold
        // this is related to the amplitude of the pulse approximately through a
        // linear drain rate the amplitude of the pulse is related to the energy
        // deposited

        // convert the time over threshold into a total energy deposited in the
        // bar (time over threshold [ns] - pedestal) * gain

        voltage_i = (digi_posend.tot() - the_conditions.totCalib(id_posend)) *
                    the_conditions.totCalib(id_posend);

      } else {
        // ADC mode of readout
        // ADC - voltage measurement at a specific time of the pulse
        amplT_posend =
            digi_posend.soi().adc_t() - the_conditions.adcPedestal(id_posend);
        amplTm1_posend =
            digi_posend.soi().adc_tm1() - the_conditions.adcPedestal(id_posend);
        voltage_i = amplT_posend * the_conditions.adcGain(id_posend);
      }

      // reverse voltage attenuation
      // for now, assume that position along the bar is the half_total_width
      double distance_end =
          id_posend.isNegativeEnd() ? distance_negend : distance_posend;
      double att = exp(-1. * ((distance_end - fabs(half_total_width)) / 1000.) /
                       attlength_);

      // set voltage
      voltage = voltage_i;
      voltage_min = voltage_i;

      // set amplitude (reverse attenuated)
      amplT = amplT_posend / att;

      // get TOA
      double TOA =
          getTOA(digi_posend, the_conditions.adcPedestal(id_posend), iSOI);

      // correct TOA
      TOA = correctionTOA_.Eval(amplT) - TOA;

      // set hit time
      hitTime = TOA;  // ns

      iDigi++;
    }  // end single readout loop

    double num_mips_equivalent = voltage / voltage_per_mip_;
    double energy_deposited = num_mips_equivalent * mip_energy_;

    // reconstructed energy in the layer (approximate)
    // TODO: need to incorporate corrections if necessary
    /**
     * Simple calculation of sampling fraction:
     * Thickness per layer: scintillator (0.2cm) + steel (0.25cm)
     * Radiation length and nuclear interaction length:
     *  scintillator: X0 = 41.31cm, Lambda = 77.07cm
     *https://pdg.lbl.gov/2017/AtomicNuclearProperties/HTML/polystyrene.html
     *  steel X0 = 1.757cm, Lambda = 16.77cm
     *https://pdg.lbl.gov/2012/AtomicNuclearProperties/HTML_PAGES/026.html Prob
     *of EM interaction (e.g. pi0): thickness/X0*100 = (0.2/41.31)/ ((0.2/41.31)
     *+ (0.25/1.757)) ~ 3.3% Prob of Had interaction (e.g. pi+/pi-): =
     *(0.2/77.07)/ ((0.2/77.07) + (0.25/16.77)) ~ 15% Then e.g. for neutron
     *assuming 1/3 pi0 and 2/3 pi+/- composition: sampling fraction ~
     *(1/3)*3.3.% + (2/3)*15% ~ 11% energy of neutron = energy_deposited / 0.11;
     *
     * NOTE: For now, sampling fraction is not applied.
     **/
    double reconstructed_energy = energy_deposited;

    int PEs = num_mips_equivalent * pe_per_mip_;
    int minPEs = (voltage_min / voltage_per_mip_) * pe_per_mip_;

    // copy over information to rec hit structure in new collection
    ldmx::HcalHit recHit;
    recHit.setID(id.raw());
    recHit.setXPos(position.X());
    recHit.setYPos(position.Y());
    recHit.setZPos(position.Z());
    recHit.setSection(id.section());
    recHit.setStrip(id.strip());
    recHit.setLayer(id.layer());
    recHit.setPE(PEs);
    recHit.setMinPE(minPEs);
    recHit.setAmplitude((amplT / voltage_per_mip_) * mip_energy_);
    recHit.setEnergy(reconstructed_energy);
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
