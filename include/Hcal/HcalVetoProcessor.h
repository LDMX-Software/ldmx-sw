/**
 * @file HcalVetoProcessor.h
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
#include "Event/HcalVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace hcal {

class HcalVetoProcessor : public framework::Producer {
 public:
  /** Constructor */
  HcalVetoProcessor(const std::string &name, framework::Process &process);

  /** Destructor */
  ~HcalVetoProcessor();

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
  /** Total PE threshold. */
  double totalPEThreshold_{8};

  /** Maximum hit time that should be considered by the veto. */
  float maxTime_{50};  // ns

  /** Maximum z depth that a hit can have. */
  float maxDepth_{4000};  // mm

  /** The minimum number of PE needed for a hit. */
  float minPE_{1};

};  // HcalVetoProcessor
}  // namespace hcal

#endif  // HCAL_HCALVETOPROCESSOR_H_
