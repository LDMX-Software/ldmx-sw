/**
 * @file HcalWABVetoProcessor.h
 * @brief Processor that determines if an event is vetoed by the Hcal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __HCAL_HCAL_VETO_PROCESSOR_H__
#define __HCAL_HCAL_VETO_PROCESSOR_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <string>

//----------//
//   LDMX   //
//----------//
#include "Event/HcalHit.h"
#include "Ecal/Event/EcalHit.h"
#include "Event/HcalCluster.h"
#include "Event/HcalVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace hcal {

class HcalWABVetoProcessor : public framework::Producer {
 public:
  /** Constructor */
  HcalWABVetoProcessor(const std::string &name, framework::Process &process);

  /** Destructor */
  ~HcalWABVetoProcessor();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Run the processor and create a collection of results which
   * indicate if the event passes/fails the Hcal veto.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event);

 private:
     double maxtotalEnergyCompare_{0.};
     double maxnClusters_{0.};
     double maxMeanHitsPerCluster_{0.};
     double maxMeanEnergyPerCluster_{0.};

};  // HcalWABVetoProcessor
}  // namespace hcal

#endif  // HCAL_HcalWABVetoProcessor_H_
