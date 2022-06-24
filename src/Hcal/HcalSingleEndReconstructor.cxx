
#include "Conditions/SimpleTableCondition.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace hcal {

class HcalSingleEndReconstructor : public framework::Producer {
  /// name of pass of digis to use
  std::string pass_name_{""};
  /// name of digis to reconstruct
  std::string coll_name_{"HcalDigis"};
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
    double linearize(const ldmx::HgcrocDigiCollection::HgcrocDigi& digi) const;
  };
  static int sum_adc(const ldmx::HgcrocDigiCollection::HgcrocDigi& digi);
  static int sum_tot(const ldmx::HgcrocDigiCollection::HgcrocDigi& digi);
 public:
  HcalSingleEndReconstructor(const std::string& n, framework::Process& p)
    : Producer(n,p) {}
  virtual void configure(framework::config::Parameters& p) final override;
  virtual void produce(framework::Event& event) final override;
}; // HcalSingleEndReconstructor

double HcalSingleEndReconstructor::TOTLinearizer::linearize(
    const ldmx::HgcrocDigiCollection::HgcrocDigi& digi) const {
  // calculate the two measurements from a single digi we
  // use in linearization process: sum_adc and sum_tot
  //  sum_adc = total of all but first in-time adc measurments
  //  sum_tot = total of all tot measurements
  int sum_adc{0}, sum_tot{0};
  for (std::size_t i_sample{0}; i_sample < digi.size(); i_sample++) {
    if (i_sample > 0) sum_adc += digi.at(i_sample).adc_t();
    sum_tot += digi.at(i_sample).tot();
  }

  // check if the linearization has been done correctly
  //  a non-zero flag value is implicitly converted to true
  if (table_.get(digi.id(), i_flagged)) {
    return sum_adc;
  } 

  // if we are in ADC range (which was used as a reference in linearization),
  // we just return it
  if (sum_tot < table_.get(digi.id(), i_lower_offset)) {
    return sum_adc;
  }

  // we know we have a linearization fit and are in TOT range,
  //  the lower side of TOT needs to be linearized with a specialized power law
  if (sum_tot < table_.get(digi.id(), i_cut_point_tot)) {
    return pow(
        (sum_tot - table_.get(digi.id(), i_lower_offset)) 
          / table_.get(digi.id(), i_low_slope),
        1/table_.get(digi.id(),i_low_power)
       ) + table_.get(digi.id(), i_tot_not);
  }

  // we know sum_tot is >= lower offset and >= tot cut
  //  higher tot, linearized with adc using a simple linear mapping
  return (sum_tot - table_.get(digi.id(), i_high_offset))*table_.get(digi.id(), i_high_slope);
  
}

void HcalSingleEndReconstructor::configure(framework::config::Parameters& p) {
  pass_name_ = p.getParameter("pass_name",pass_name_);
  coll_name_ = p.getParameter("coll_name",coll_name_);
}

void HcalSingleEndReconstructor::produce(framework::Event& event) {
  TOTLinearizer linearizer{
    getCondition<conditions::DoubleTableCondition>("hcal_tot_calibration")};
}

}

DECLARE_PRODUCER_NS(hcal,HcalSingleEndReconstructor);
