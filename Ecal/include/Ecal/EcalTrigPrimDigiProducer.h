/**
 * @file EcalTrigPrimDigiProducer.h
 * @brief Class that performs emulation of the EcalTriggerPrimitives
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef ECAL_ECALTRIGPRIMDIGIPRODUCER_H_
#define ECAL_ECALTRIGPRIMDIGIPRODUCER_H_

//----------------//
//   LDMX Core    //
//----------------//
#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "Tools/HgcrocTriggerCalculations.h"

namespace ecal {

/**
 * @class EcalRecProducer
 * @brief Performs basic ECal reconstruction
 *
 * Reconstruction is done from the EcalDigi samples.
 * Some hard-coded parameters are used for position and energy calculation.
 */
class EcalTrigPrimDigiProducer : public framework::Producer {
 public:
  /**
   * Constructor
   */
  EcalTrigPrimDigiProducer(const std::string& name,
                           framework::Process& process);

  /**
   * Grabs configure parameters from the python config file.
   *
   * Parameter        Default
   * inputDigiCollName     EcalDigis
   * inputDigiPassName     "" <-- blank means take any pass if only one
   * collection exists
   */
  virtual void configure(framework::config::Parameters&);

  /**
   * Produce EcalHits and put them into the event bus using the
   * EcalDigis as input.
   */
  virtual void produce(framework::Event& event);

 private:
  /** Digi Collection Name to use as input */
  std::string digi_coll_name_;

  /** Digi Pass Name to use as input */
  std::string digi_pass_name_;

  /** Conditions object for the calibration information */
  std::string cond_obj_name_;
};
}  // namespace ecal

#endif  // EVENTPROC_ECALTRIGPRIMDIGIPRODUCER_H_INC
