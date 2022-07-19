#include "Conditions/SimpleTableCondition.h"
#include "Framework/EventProcessor.h"
#include "Framework/EventDef.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalDigiID.h"
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Hcal/HcalReconConditions.h"

namespace hcal {

class HcalSingleEndRecProducer : public framework::Producer {
  /// name of pass of digis to use
  std::string pass_name_{""};
  /// name of digis to reconstruct
  std::string coll_name_{"HcalDigis"};
  /// name of pass of rechits to use
  std::string rec_pass_name_{""};
  /// name of rechits to reconstruct
  std::string rec_coll_name_{"HcalRecHits"};
  
  /// number of PEs per MIP
  double pe_per_mip_;
  /// energy per MIP [MeV]
  double mip_energy_;
  /// length of clock cycle [ns]
  double clock_cycle_;
  /// sample of interest index
  unsigned int isoi_;
  
private:

  /**
   * extract toa, sum adc, and sum tot from the input raw digi
   *
   * in the far future, we can make these member functions ofthe HgcrocDigi class;
   * however, right now as we develop our reconstruction method it is helpful to have
   * more flexible control on how we extract these measurements
   *
   * with C++17 structured bindings, this tuple return can be bound to separate variables:
   * ```cpp
   * auto [ toa, sum_adc, sum_tot ] = extract_measurments(digi,pedestal,isoi);
   * ```
   * giving us the dual benefit of separate variable names while only having to loop over
   * the samples within a single digi once
   *
   * Uses isoi_ and clock_cycle_ member variables to convert TOA into ns since beginning
   * of Sample Of Interest (SOI)
   *
   * @param[in] digi handle to HgcrocDigi to extract from
   * @param[in] pedestal pedestal for this channel
   * @return tuple of (toa [ns since SOI], sum_adc, sum_tot)
   */
  std::tuple<double, double, int> extract_measurments(
      const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal);

public:

  HcalSingleEndRecProducer(const std::string& n, framework::Process& p)
    : Producer(n,p) {}
  
  virtual void configure(framework::config::Parameters& p) final override;
  virtual void produce(framework::Event& event) final override;
  
}; // HcalSingleEndRecProducer

std::tuple<double,double,int> HcalSingleEndRecProducer::extract_measurments(
    const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal) {
  // sum_adc = total of all but first in-time adc measurements
  double sum_adc{0};
  // sum_tot = total of all tot measurements
  int sum_tot{0};
  // first, get time of arrival w.r.t to start BX
  int toa_sample{0},toa_startbx{0};
  // and figure out sample of maximum amplitude
  int max_sample{0};
  double max_meas{0};
  for (std::size_t i_sample{0}; i_sample < digi.size(); i_sample++) {
    // adc logic
    if (i_sample > 0) sum_adc += (digi.at(i_sample).adc_t() - pedestal);

    // tot logic
    sum_tot += digi.at(i_sample).tot();
    
    // toa logic
    if (digi.at(i_sample).toa() > 0) {
      toa_startbx = digi.at(i_sample).toa() * (clock_cycle_ / 1024);
      toa_sample = i_sample;
    }
    if (digi.at(i_sample).adc_t() - pedestal > max_meas){
      max_meas = digi.at(i_sample).adc_t() - pedestal;
      max_sample = i_sample;
    }
  }
  // get toa w.r.t the peak
  double toa = (max_sample - toa_sample) * clock_cycle_ - toa_startbx;
  // get toa w.r.t the SOI
  toa += ((int)isoi_ - max_sample) * clock_cycle_;
  
  return std::make_tuple(toa, sum_adc, sum_tot);
}
  
void HcalSingleEndRecProducer::configure(framework::config::Parameters& p) {
  pass_name_ = p.getParameter("pass_name",pass_name_);
  coll_name_ = p.getParameter("coll_name",coll_name_);

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
    auto [toa, sum_adc, sum_tot] = extract_measurments(digi, conditions.adcPedestal(digi.id()));
    auto is_adc = conditions.is_adc(digi.id(),sum_tot);
    if(is_adc){
      double adc_calib = sum_adc / conditions.adcGain(digi.id(), 0) ;
      num_mips_equivalent = adc_calib;
    }
    else{
      double tot_calib = conditions.linearize(digi.id(),sum_tot);
      num_mips_equivalent = tot_calib;
    }
    int PEs = num_mips_equivalent * pe_per_mip_;
    double reconstructed_energy = num_mips_equivalent * pe_per_mip_ * mip_energy_;

    // time reconstruction 
    //    right now, just using the decoded TOA converted to ns (done above)
    //    we could attempt to correct for time walk here
    double hitTime = toa;

    // position single ended
    auto position = hcalGeometry.getStripCenterPosition(id);
    
    // reconstructed Hit
    ldmx::HcalHit recHit;
    recHit.setID(id.raw());
    recHit.setXPos(0);
    recHit.setYPos(0);
    recHit.setZPos(0);
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
  
}
DECLARE_PRODUCER_NS(hcal,HcalSingleEndRecProducer);
