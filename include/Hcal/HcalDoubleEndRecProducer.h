#ifndef HCALDOUBLEENDRECPRODUCER_H
#define HCALDOUBLEENDRECPRODUCER_H

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

class HcalDoubleEndRecProducer : public framework::Producer {
 private:
  /// name of pass of rechits to use
  std::string pass_name_{""};
  /// name of rechits to use as input
  std::string coll_name_{"HcalRecHits"};
  /// name of pass of rechits to reconstruct
  std::string rec_pass_name_{""};
  /// name of rechits to reconstruct
  std::string rec_coll_name_{"HcalRecHitsDoubleEnd"};

  /// number of PEs per MIP
  double pe_per_mip_;
  /// energy per MIP [MeV]
  double mip_energy_;
  /// length of clock cycle [ns]
  double clock_cycle_;

 public:
  HcalDoubleEndRecProducer(const std::string& n, framework::Process& p)
      : Producer(n, p) {}

  virtual ~HcalDoubleEndRecProducer() = default;
  void configure(framework::config::Parameters& p) override;
  void produce(framework::Event& event) override;

};  // HcalDoubleEndRecProducer
}  // namespace hcal

#endif /* HCALDOUBLEENDRECPRODUCER_H */
