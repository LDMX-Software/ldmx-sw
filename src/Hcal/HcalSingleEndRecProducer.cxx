#include "Hcal/HcalSingleEndRecProducer.h"
namespace hcal {

std::tuple<double, double, int> HcalSingleEndRecProducer::extract_measurements(
    const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal,
    double bx_shift) {
  // sum_adc = total of all but first in-time adc measurements
  double sum_adc{0};
  // sum_tot = total of all tot measurements
  int sum_tot{0};
  // first, get time of arrival w.r.t to start BX
  int toa_sample{0}, toa_startbx{0};
  // get the correction for the wrong BX assignment
  // TODO: Currently not used anywhere
  [[maybe_unused]] int bx_to_time{0};
  // and figure out sample of maximum amplitude
  // TODO: Currently not used anywhere
  [[maybe_unused]] int max_sample{0};
  double max_meas{0};
  for (std::size_t i_sample{0}; i_sample < digi.size(); i_sample++) {
    // adc logic
    if (i_sample > 0) sum_adc += (digi.at(i_sample).adc_t() - pedestal);

    // tot logic
    sum_tot += digi.at(i_sample).tot();

    // toa logic
    if (digi.at(i_sample).toa() > 0) {
      if (digi.at(i_sample).toa() >= bx_shift) {
        toa_sample = i_sample;
      } else {
        toa_sample = i_sample + 1;
      }

      // sum toa in given bx - given that multiple bx may be associated with the
      // TOA measurement
      toa_startbx += digi.at(i_sample).toa() * (clock_cycle_ / 1024) +
                     (clock_cycle_ * toa_sample);
    }

    if (digi.at(i_sample).adc_t() - pedestal > max_meas) {
      max_meas = digi.at(i_sample).adc_t() - pedestal;
      max_sample = i_sample;
    }
  }
  // get toa
  double toa = toa_startbx;

  // get toa w.r.t the peak
  // double toa = (max_sample - toa_sample) * clock_cycle_ - toa_startbx;
  // get toa w.r.t the SOI
  // toa += ((int)isoi_ - max_sample) * clock_cycle_;

  return std::make_tuple(toa, sum_adc, sum_tot);
}

void HcalSingleEndRecProducer::configure(framework::config::Parameters& p) {
  pass_name_ = p.getParameter("pass_name", pass_name_);
  coll_name_ = p.getParameter("coll_name", coll_name_);

  rec_pass_name_ = p.getParameter("rec_pass_name", rec_pass_name_);
  rec_coll_name_ = p.getParameter("rec_coll_name", rec_coll_name_);

  pe_per_mip_ = p.getParameter<double>("pe_per_mip");
  mip_energy_ = p.getParameter<double>("mip_energy");
  clock_cycle_ = p.getParameter<double>("clock_cycle");
}

void HcalSingleEndRecProducer::produce(framework::Event& event) {
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  const auto& conditions{
      getCondition<HcalReconConditions>(HcalReconConditions::CONDITIONS_NAME)};

  auto hcalDigis =
      event.getObject<ldmx::HgcrocDigiCollection>(coll_name_, pass_name_);

  std::vector<ldmx::HcalHit> hcalRecHits;

  isoi_ = hcalDigis.getSampleOfInterestIndex();

  for (auto const& digi : hcalDigis) {
    ldmx::HcalDigiID id_digi(digi.id());
    ldmx::HcalID id(id_digi.section(), id_digi.layer(), id_digi.strip());

    // amplitude/TOT reconstruction
    double num_mips_equivalent{0};
    auto [toa, sum_adc, sum_tot] =
        extract_measurements(digi, conditions.adcPedestal(digi.id()),
                             conditions.toaCalib(digi.id(), 0));
    auto is_adc = conditions.is_adc(digi.id(), sum_tot);
    if (is_adc) {
      double adc_calib = sum_adc / conditions.adcGain(digi.id(), 0);
      num_mips_equivalent = adc_calib;
    } else {
      double tot_calib = conditions.linearize(digi.id(), sum_tot);
      num_mips_equivalent = tot_calib / conditions.adcGain(digi.id(), 0);
    }
    int PEs = num_mips_equivalent * pe_per_mip_;
    double reconstructed_energy =
        num_mips_equivalent * pe_per_mip_ * mip_energy_;

    // time reconstruction
    double hitTime = toa;

    // position single ended (taken directly from geometry)
    auto position = hcalGeometry.getStripCenterPosition(id);

    // reconstructed Hit
    ldmx::HcalHit recHit;
    recHit.setID(id.raw());
    recHit.setXPos(position.X());
    recHit.setYPos(position.Y());
    recHit.setZPos(position.Z());
    recHit.setSection(id.section());
    recHit.setStrip(id.strip());
    recHit.setLayer(id.layer());
    recHit.setEnd(id_digi.end());
    recHit.setPE(PEs);
    recHit.setMinPE(PEs);
    recHit.setAmplitude(num_mips_equivalent);
    recHit.setEnergy(reconstructed_energy);
    recHit.setTime(hitTime);
    recHit.setIsADC(is_adc);
    hcalRecHits.push_back(recHit);
  }

  // add collection to event bus
  event.add(rec_coll_name_, hcalRecHits);
}

}  // namespace hcal
DECLARE_PRODUCER_NS(hcal, HcalSingleEndRecProducer);
