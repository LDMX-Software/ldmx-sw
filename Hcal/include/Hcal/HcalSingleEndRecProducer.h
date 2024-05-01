#ifndef HCALSINGLEENDRECPRODUCER_H
#define HCALSINGLEENDRECPRODUCER_H

#include "Conditions/SimpleTableCondition.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalDigiID.h"
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/EventProcessor.h"
#include "Hcal/HcalReconConditions.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Hcal/Event/HcalHit.h"

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
   * in the far future, we can make these member functions ofthe HgcrocDigi
   * class; however, right now as we develop our reconstruction method it is
   * helpful to have more flexible control on how we extract these measurements
   *
   * with C++17 structured bindings, this tuple return can be bound to separate
   * variables:
   * ```cpp
   * auto [ toa, sum_adc, sum_tot ] =
   * extract_measurements(digi,pedestal,bx_shift);
   * ```
   * giving us the dual benefit of separate variable names while only having to
   * loop over the samples within a single digi once
   *
   * Uses isoi_ and clock_cycle_ member variables to convert TOA into ns since
   * beginning of Sample Of Interest (SOI)
   *
   * @param[in] digi handle to HgcrocDigi to extract from
   * @param[in] pedestal pedestal for this channel
   * @param[in] shift in BX associated to TOA for this channel
   * @return tuple of (toa [ns since SOI], sum_adc, sum_tot)
   */
  std::tuple<double, double, int> extract_measurements(
      const ldmx::HgcrocDigiCollection::HgcrocDigi& digi, double pedestal,
      double bx_shift);

 public:
  HcalSingleEndRecProducer(const std::string& n, framework::Process& p)
      : Producer(n, p) {}
  virtual ~HcalSingleEndRecProducer() = default;

  void configure(framework::config::Parameters& p) override;
  void produce(framework::Event& event) override;

};  // HcalSingleEndRecProducer

}  // namespace hcal

#endif /* HCALSINGLEENDRECPRODUCER_H */
