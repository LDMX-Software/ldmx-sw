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
  
private:
  /**
   * Class encapsulating the TOT linearization procedure
   *
   * in order to improve performance, we cache the column-index mapping
   * and we perform the linearization procedure on an input digi of 
   * multiple samples
   *
   * the 'DetID' column is required and not included in the column index
   * by the conditions system so you'll notice that these column indices
   * are off by one relative to a raw reading of the CSV
   */
  class TOTLinearizer {
    const conditions::DoubleTableCondition& table_;
    static const int i_m_adc_i       = 0;
    static const int i_cut_point_tot = 1;
    static const int i_high_slope    = 2;
    static const int i_high_offset   = 3;
    static const int i_low_slope     = 4;
    static const int i_low_power     = 5;
    static const int i_lower_offset  = 6;
    static const int i_tot_not       = 7;
    static const int i_channel       = 8;
    static const int i_flagged       = 9;
   public:
    TOTLinearizer(const conditions::DoubleTableCondition& t)
      : table_{t} {}
    bool is_adc(int digi_id, double sum_tot) const;
    double linearize(int digi_id, double sum_tot) const;
  };
  
  static double get_sum_adc(const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal);
  static int get_sum_tot(const ldmx::HgcrocDigiCollection::HgcrocDigi& digi);
  double get_toa(const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal);

public:

  HcalSingleEndRecProducer(const std::string& n, framework::Process& p)
    : Producer(n,p) {}
  
  virtual void configure(framework::config::Parameters& p) final override;
  virtual void produce(framework::Event& event) final override;
  
}; // HcalSingleEndRecProducer

double HcalSingleEndRecProducer::get_sum_adc(
    const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal) {
  // sum_adc = total of all but first in-time adc measurements
  double sum_adc{0};
  for (std::size_t i_sample{0}; i_sample < digi.size(); i_sample++) {
    if (i_sample > 0) sum_adc += (digi.at(i_sample).adc_t() - pedestal);
  }
}

int HcalSingleEndRecProducer::get_sum_tot(
    const ldmx::HgcrocDigiCollection::HgcrocDigi& digi) {
  // sum_tot = total of all tot measurements
  int sum_tot{0};
  for (std::size_t i_sample{0}; i_sample < digi.size(); i_sample++) {
    sum_tot += digi.at(i_sample).tot();
  }
}

double HcalSingleEndRecProducer::get_toa(
    const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal) {
  // first, get time of arrival w.r.t to start BX
  int toa_sample{0},toa_startbx{0};
  // and figure out sample of maximum amplitude
  int max_sample{0};
  double max_meas{0};
  for (std::size_t i_sample{0}; i_sample < digi.size(); i_sample++) {
    if (digi.at(i_sample).toa() > 0) {
      toa_startbx = digi.at(i_sample).toa() * (clock_cycle_ / 1024);
      toa_sample = i_sample;
    }
    if (digi.at(i_sample).adc_t() - pedestal > max_meas){
      max_sample = i_sample;
      max_meas = digi.at(i_sample).adc_t() - pedestal;
    }
  }

  // get toa w.r.t the peak
  int toa{0};
  double toa = (max_sample - toa_sample) * clock_cycle_ - toa_startbx;

  // get toa w.r.t the SOI
  toa += ((int)iSOI - max_sample) * clock_cycle_;

  return toa;
}
  
bool HcalSingleEndRecProducer::TOTLinearizer::is_adc(
    int digi_id, double sum_tot) const {
  // check if the linearization has been done correctly
  //  a non-zero flag value is implicitly converted to true
  if (table_.get(digi_id, i_flagged)) {
    return true;
  }
  
  // if we are in ADC range (which was used as a reference in linearization),
  // we use ADC
  if (sum_tot < table_.get(digi_id, i_lower_offset)) {
    return true;
  }

  return false;
}
double HcalSingleEndRecProducer::TOTLinearizer::linearize(
    int digi_id, double sum_tot) const {
  
  // we know we have a linearization fit and are in TOT range,
  //  the lower side of TOT needs to be linearized with a specialized power law
  if (sum_tot < table_.get(digi_id, i_cut_point_tot)) {
    return pow(
        (sum_tot - table_.get(digi_id, i_lower_offset)) 
          / table_.get(digi_id, i_low_slope),
        1/table_.get(digi_id,i_low_power)
       ) + table_.get(digi_id, i_tot_not);
  }

  // we know sum_tot is >= lower offset and >= tot cut
  //  higher tot, linearized with adc using a simple linear mapping
  return (sum_tot - table_.get(digi_id, i_high_offset))*table_.get(digi_id, i_high_slope);
  
}

void HcalSingleEndRecProducer::configure(framework::config::Parameters& p) {
  pass_name_ = p.getParameter("pass_name",pass_name_);
  coll_name_ = p.getParameter("coll_name",coll_name_);

  pe_per_mip_ = p.getParameter<double>("pe_per_mip");
  mip_energy_ = p.getParameter<double>("mip_enegy");
  clock_cycle_ = p.getParameter<double>("clock_cycle");
}

void HcalSingleEndRecProducer::produce(framework::Event& event) {

  const auto& conditions{
    getCondition<HcalReconConditions>(HcalReconConditions::CONDITIONS_NAME)};
    
  TOTLinearizer linearizer{
    getCondition<conditions::DoubleTableCondition>("hcal_tot_calibration")};

  auto hcalDigis =
    event.getObject<ldmx::HgcrocDigiCollection>(rec_coll_name_, rec_pass_name_);
  std::vector<ldmx::HcalHit> hcalRecHits;
  
  for (auto const& digi : hcalDigis) {
    ldmx::HcalDigiID id_digi(digi.id());
    ldmx::HcalID id(id_digi.section(), id_digi.layer(), id_digi.strip());
    
    // amplitude/TOT reconstruction
    double num_mips_equivalent{0};
    auto sum_adc = get_sum_adc(digi,
			       conditions.adcPedestal(digi.id()));
    auto sum_tot = get_sum_tot(digi);
    auto is_adc = linearizer.is_adc(digi.id(),sum_tot);
    if(is_adc){
      double adc_calib = sum_adc / conditions.adcGain(digi.id(), 0) ;
      num_mips_equivalent = adc_calib;
    }
    else{
      double tot_calib = linearizer.linearize(digi.id(),sum_tot);
      num_mips_equivalent = tot_calib;
    }
    int PEs = num_mips_equivalent * pe_per_mip_;
    double reconstructed_energy = num_mips_equivalent * pe_per_mip_ * mip_energy_;

    // time reconstruction 
    double hitTime = get_toa(digi.id(),
			     conditions.adcPedestal(digi.id()));

    // position TODO
    
    // reconstructed Hit
    ldmx::HcalHit recHit;
    recHit.setID(id.raw());
    recHit.setXPos(0);
    recHit.setYPos(0);
    recHit.setZPos(0);
    recHit.setPE(PEs);
    recHit.setMinPE(PEs);
    recHit.setAmplitude(num_mips_equivalent);
    recHit.setEnergy(reconstructed_energy);
    recHit.setTime(hitTime);
    hcalRecHits.push_back(recHit);
  }

  // add collection to event bus
  event.add(rec_coll_name_, hcalRecHits);
}
  
}
DECLARE_PRODUCER_NS(hcal,HcalSingleEndRecProducer);
