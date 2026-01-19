/**
 * @file HcalRecProducer.cxx
 * @brief Class that performs basic HCal reconstruction
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Hcal/HcalRecProducer.h"

#include "Hcal/Event/HcalHit.h"
#include "Hcal/HcalReconConditions.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace hcal {

HcalRecProducer::HcalRecProducer(const std::string& name,
                                 framework::Process& process)
    : Producer(name, process) {}

void HcalRecProducer::configure(framework::config::Parameters& ps) {
  // collection names
  input_coll_name_ = ps.get<std::string>("input_coll_name");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  sim_hit_coll_name_ = ps.get<std::string>("sim_hit_coll_name");
  sim_hit_pass_name_ = ps.get<std::string>("sim_hit_pass_name");
  rec_hit_coll_name_ = ps.get<std::string>("rec_hit_coll_name");
  // parameters
  mip_energy_ = ps.get<double>("mip_energy");
  pe_per_mip_ = ps.get<double>("pe_per_mip");
  clock_cycle_ = ps.get<double>("clock_cycle");
  voltage_per_mip_ = ps.get<double>("voltage_per_mip");
  attlength_ = ps.get<double>("attenuation_length");
  n_adcs_ = ps.get<int>("n_adcs");

  // configuring corrections graphs derived on the fly
  // TODO: maybe we should save these as a graph instead?
  rate_up_slope_ = ps.get<double>("rate_up_slope");
  time_up_slope_ = ps.get<double>("time_up_slope");
  rate_dn_slope_ = ps.get<double>("rate_dn_slope");
  time_dn_slope_ = ps.get<double>("time_dn_slope");
  time_peak_ = ps.get<double>("time_peak");
  pulse_func_ =
      TF1("pulseFunc",
          "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/"
          "((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
          (double)n_adcs_ * clock_cycle_ * -1, (double)n_adcs_ * clock_cycle_);
  pulse_func_.FixParameter(1, rate_up_slope_);
  pulse_func_.FixParameter(2, time_up_slope_);
  pulse_func_.FixParameter(3, time_peak_);
  pulse_func_.FixParameter(5, rate_dn_slope_);
  pulse_func_.FixParameter(6, time_dn_slope_);
  pulse_func_.FixParameter(4, 0);
  pulse_func_.FixParameter(0, 1);

  // build amplitude correction (Ampl[t-1]/Ampl[t]) with pulse-shape
  int n = 0;
  for (double t = -clock_cycle_; t < clock_cycle_; t += 0.01) {
    double ampl_t = pulse_func_.Eval(t);
    double ampl_tm1 = pulse_func_.Eval(t - clock_cycle_);
    if (ampl_tm1 > ampl_t) continue;
    correction_ampl_.SetPoint(n, ampl_tm1 / ampl_t, ampl_t);
    if (n == 0) min_ampl_fraction_ = ampl_tm1 / ampl_t;
    n++;
  }

  // build TOA timewalk correction with pulse-shape
  double toa_threshold = ps.get<double>("avg_toa_threshold");
  double gain = ps.get<double>("avg_gain");
  double pedestal = ps.get<double>("avg_pedestal");
  n = 0;
  for (double ampl = toa_threshold + 0.1; ampl < 10000; ampl += 0.01) {
    pulse_func_.FixParameter(0, ampl);
    double ampl_t = gain * pedestal + pulse_func_.Eval(0);
    double toa = fabs(pulse_func_.GetX(toa_threshold,
                                       (double)n_adcs_ * clock_cycle_ * -1,
                                       (double)n_adcs_ * clock_cycle_));
    correction_toa_.SetPoint(n, ampl_t, toa);
    if (n == 0) min_ampl_ = ampl_t;
    n++;
  }
  correction_toa_.SetBit(TGraph::kIsSortedX);
}

double HcalRecProducer::getTOA(
    const ldmx::HgcrocDigiCollection::HgcrocDigi digi, double pedestal,
    unsigned int iSOI) const {
  // get toa relative to the startBX
  double toa_rel_start_bx(0.), max_meas{0.};
  int toa_sample(0), max_sample(0), i_adc(0);
  for (int i_sample{0}; i_sample < digi.size(); i_sample++) {
    auto sample{digi.at(i_sample)};
    if (sample.toa() > 0) {
      toa_rel_start_bx = sample.toa() * (clock_cycle_ / 1024);  // ns
      // find in which ADC sample the TOA was taken
      toa_sample = i_adc;
    }
    if ((sample.adcT() - pedestal) > max_meas) {
      max_meas = (sample.adcT() - pedestal);
      max_sample = i_adc;
    }
    i_adc++;
  }

  // time w.r.t to the peak
  double toa = (max_sample - toa_sample) * clock_cycle_ - toa_rel_start_bx;

  // time w.r.t to the SOI
  toa += (static_cast<int>(iSOI) - max_sample) * clock_cycle_;

  return toa;
}

void HcalRecProducer::produce(framework::Event& event) {
  // get the Hcal Geometry
  const auto& hcal_geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  // get the reconstruction parameters
  const auto& the_conditions{
      getCondition<HcalReconConditions>(HcalReconConditions::CONDITIONS_NAME)};

  std::vector<ldmx::HcalHit> hcal_rec_hits;
  auto hcal_digis = event.getObject<ldmx::HgcrocDigiCollection>(
      input_coll_name_, input_pass_name_);
  int num_digi_hits = hcal_digis.getNumDigis();

  // get sample of interest index
  unsigned int i_soi = hcal_digis.getSampleOfInterestIndex();

  // loop through digis
  int i_digi = 0;
  while (i_digi < num_digi_hits) {
    auto digi_posend = hcal_digis.getDigi(i_digi);

    // track readout mode for this hit (1 = ADC, 0 = TOT)
    bool is_adc_mode = false;

    // ID from first digi sample (which should be in positive end)
    ldmx::HcalDigiID id_posend(digi_posend.id());
    ldmx::HcalID id(id_posend.section(), id_posend.layer(), id_posend.strip());

    // position from ID
    auto position = hcal_geometry.getStripCenterPosition(id);
    double half_total_width =
        hcal_geometry.getHalfTotalWidth(id.section(), id.layer());
    double ecal_dx = hcal_geometry.getEcalDx();
    double ecal_dy = hcal_geometry.getEcalDy();

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
          (id.section() == ldmx::HcalID::HcalSection::BOTTOM)) {
        distance_ecal = ecal_dx;
      } else {
        distance_ecal = ecal_dy;
      }
      distance_posend = 2 * half_total_width - distance_ecal / 2.;
      distance_negend = distance_ecal / 2.;
    }

    // get the estimated voltage and time from digi samples
    double voltage(0.);
    double voltage_min(0.);
    double hit_time(0.);

    double ampl_t(0.);
    double ampl_t_posend(0.), ampl_tm1_posend(0.);
    double ampl_t_negend(0.), ampl_tm1_negend(0.);

    // Check if the bar is oriented in X or Y
    const auto orientation{hcal_geometry.getScintillatorOrientation(id)};
    int orientation_int = static_cast<int>(orientation);

    // double readout
    if (id.section() == ldmx::HcalID::HcalSection::BACK) {
      auto digi_negend = hcal_digis.getDigi(i_digi + 1);
      ldmx::HcalDigiID id_negend(digi_negend.id());

      double voltage_posend, voltage_negend;
      // Check if in TOT mode
      if (digi_posend.isTOT()) {
        is_adc_mode = false;
        voltage_posend =
            (digi_posend.tot() - the_conditions.totCalib(id_posend, 0)) *
            the_conditions.totCalib(id_posend, 1);
        voltage_negend =
            (digi_negend.tot() - the_conditions.totCalib(id_negend, 0)) *
            the_conditions.totCalib(id_negend, 1);
      }  // end TOT mode, go to ADC mode
      else {
        is_adc_mode = true;
        ampl_t_posend =
            digi_posend.soi().adcT() - the_conditions.adcPedestal(id_posend);
        ampl_tm1_posend =
            digi_posend.soi().adcTm1() - the_conditions.adcPedestal(id_posend);
        ampl_t_negend =
            digi_negend.soi().adcT() - the_conditions.adcPedestal(id_negend);
        ampl_tm1_negend =
            digi_negend.soi().adcTm1() - the_conditions.adcPedestal(id_negend);

        // correct amplitude (amplitude fractions from both ends need to be
        // above the boundary of the correction)
        if (ampl_tm1_posend / ampl_t_posend > min_ampl_fraction_ &&
            ampl_tm1_negend / ampl_t_negend > min_ampl_fraction_) {
          ampl_t_posend *=
              correction_ampl_.Eval(ampl_tm1_posend / ampl_t_posend);
          ampl_t_negend *=
              correction_ampl_.Eval(ampl_tm1_negend / ampl_t_negend);
        }

        // set voltage
        voltage_posend = ampl_t_posend * the_conditions.adcGain(id_posend, 0);
        voltage_negend = ampl_t_negend * the_conditions.adcGain(id_negend, 0);
      }  // end ADC mode

      // get TOA
      double toa_posend =
          getTOA(digi_posend, the_conditions.adcPedestal(id_posend), i_soi);
      double toa_negend =
          getTOA(digi_negend, the_conditions.adcPedestal(id_negend), i_soi);

      // get sign of position along the bar
      int position_bar_sign = (toa_posend - toa_negend) > 0 ? 1 : -1;

      // correct TOA
      // amplitudes from both ends need to be above the boundary of the
      // correction otherwise, one TOA gets corrected and the other does not,
      // which results in a large TOA difference and an out-of-bounds position
      if (ampl_t_posend > min_ampl_ && ampl_t_negend > min_ampl_) {
        toa_posend = correction_toa_.Eval(ampl_t_posend) - toa_posend;
        toa_negend = correction_toa_.Eval(ampl_t_negend) - toa_negend;
      }

      // get x(y) coordinate from TOA measurement = (dt*v/2)
      // if time_posend < time_negend: position is positive
      // velocity of light in polystyrene, n = 1.6 = c/v
      double v = 299.792 / 1.6;
      double position_bar =
          position_bar_sign * fabs(toa_posend - toa_negend) * v / 2;

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
      ampl_t = (ampl_t_posend / att_posend + ampl_t_negend / att_negend) / 2;

      // set position along the bar
      if (orientation ==
          ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
        position.SetX(position_bar);
      } else {
        position.SetY(position_bar);
      }

      // set hit time
      // TODO: does this need to revert shift because of propagation of light in
      // polysterene?
      hit_time = fabs(toa_posend + toa_negend) / 2;  // ns

      i_digi += 2;
    }  // end double readout loop
    else {  // single readout
      double voltage_i;
      // Check if in TOT mode (for single-ended readout)
      if (digi_posend.isTOT()) {
        is_adc_mode = false;
        // TOT - number of clock ticks that pulse was over threshold
        // this is related to the amplitude of the pulse approximately through a
        // linear drain rate the amplitude of the pulse is related to the energy
        // deposited

        // convert the time over threshold into a total energy deposited in the
        // bar (time over threshold [ns] - pedestal) * gain

        voltage_i = (digi_posend.tot() - the_conditions.totCalib(id_posend)) *
                    the_conditions.totCalib(id_posend);

      }  // end TOT mode, go to ADC mode (for single-ended readout)
      else {
        is_adc_mode = true;
        // ADC mode of readout
        // ADC - voltage measurement at a specific time of the pulse
        ampl_t_posend =
            digi_posend.soi().adcT() - the_conditions.adcPedestal(id_posend);
        ampl_tm1_posend =
            digi_posend.soi().adcTm1() - the_conditions.adcPedestal(id_posend);
        voltage_i = ampl_t_posend * the_conditions.adcGain(id_posend);
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
      ampl_t = ampl_t_posend / att;

      // get TOA
      double toa =
          getTOA(digi_posend, the_conditions.adcPedestal(id_posend), i_soi);

      // correct TOA
      toa = correction_toa_.Eval(ampl_t) - toa;

      // set hit time
      hit_time = toa;  // ns

      i_digi++;
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

    int pe_s = num_mips_equivalent * pe_per_mip_;
    int min_pe_s = (voltage_min / voltage_per_mip_) * pe_per_mip_;

    // copy over information to rec hit structure in new collection
    ldmx::HcalHit rec_hit;
    rec_hit.setID(id.raw());
    rec_hit.setXPos(position.X());
    rec_hit.setYPos(position.Y());
    rec_hit.setZPos(position.Z());
    rec_hit.setSection(id.section());
    rec_hit.setStrip(id.strip());
    rec_hit.setLayer(id.layer());
    rec_hit.setPE(pe_s);
    rec_hit.setMinPE(min_pe_s);
    rec_hit.setIsADC(is_adc_mode ? 1 : 0);
    rec_hit.setAmplitude((ampl_t / voltage_per_mip_) * mip_energy_);
    rec_hit.setEnergy(reconstructed_energy);
    rec_hit.setTime(hit_time);
    rec_hit.setOrientation(orientation_int);
    hcal_rec_hits.push_back(rec_hit);
  }  // end of digi loop

  // mark noise hits if sim hits are available
  if (event.exists(sim_hit_coll_name_, sim_hit_pass_name_)) {
    // hcal sim hits_ exist ==> label which hits_ are real and which are pure
    // noise
    auto hcal_sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(
        sim_hit_coll_name_, sim_hit_pass_name_)};
    std::set<int> real_hits;
    for (auto const& sim_hit : hcal_sim_hits) real_hits.insert(sim_hit.getID());
    for (auto& hit : hcal_rec_hits)
      hit.setNoise(real_hits.find(hit.getID()) == real_hits.end());
  }

  // add collection to event bus
  event.add(rec_hit_coll_name_, hcal_rec_hits);
}

}  // namespace hcal

DECLARE_PRODUCER(hcal::HcalRecProducer);
