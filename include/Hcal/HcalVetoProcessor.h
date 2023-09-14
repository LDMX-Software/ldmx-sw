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

  /** The minimum number of PE in both bars needed for a hit to be considered in
   * double ended readout mode. */
  float backMinPE_{1};

  /*
   * A hit representing the case where we never reach the maxPE condition. This
   * is rare but can happen which previously would record uninitialized memory.
   * Stored inside of the producer so that it will have a valid lifetime when we
   * persist it.
   *
   * See https://github.com/LDMX-Software/Hcal/issues/58 for details
   *
   * Ideally, this would just be stored as a part of the HcalVetoResult, but
   * changing that would be breaking change so for now we work around it like
   * this.
   *
   * It contains nonsense values but since they are predictable, they are harder
   * to mistake for real hits. See constructor for the actual values.
   */
  ldmx::HcalHit defaultMaxHit_;

};  // HcalVetoProcessor
}  // namespace hcal

#endif  // HCAL_HCALVETOPROCESSOR_H_
