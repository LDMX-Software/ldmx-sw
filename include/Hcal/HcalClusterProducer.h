/**
 * @file HcalClusterProducer.h
 * @brief Class that performs clustering of HCal hits
 * @author Sophie Middleton, Caltech
 */

#ifndef HCAL_HCALCLUSTERPRODUCER_H_
#define HCAL_HCALCLUSTERPRODUCER_H_

// ROOT
#include "TRandom3.h"
#include "TString.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"

// Hcal
#include "DetDescr/HcalGeometry.h"
#include "Hcal/Event/HcalCluster.h"
#include "Hcal/Event/HcalHit.h"
#include "Hcal/MyClusterWeight.h"
#include "Hcal/TemplatedClusterFinder.h"
#include "Hcal/WorkingCluster.h"
namespace hcal {

/**
 * @class HcalClusterProducer
 * @brief Make clusters from hits in the HCAL
 */
class HcalClusterProducer : public framework::Producer {
 public:
  HcalClusterProducer(const std::string& name, framework::Process& process);

  virtual ~HcalClusterProducer() { ; }

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  virtual void produce(framework::Event& event);

 private:
  bool verbose_{false};
  // double     EminSeed_{0.};
  double EnoiseCut_{0.};
  double deltaTime_{0};
  double deltaR_{0};
  double EminCluster_{0.};
  double cutOff_{0.};
  std::string clusterCollName_;
};

}  // namespace hcal

#endif
