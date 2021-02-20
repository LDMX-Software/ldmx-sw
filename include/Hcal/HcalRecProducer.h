/**
 * @file HcalRecProducer.h
 * @brief Class that performs basic HCal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#ifndef HCAL_HCALRECPRODUCER_H_
#define HCAL_HCALRECPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory>  //for smart pointers

//----------//
//   LDMX   //
//----------//
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalDigiID.h"
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/EventDef.h"
#include "Framework/EventProcessor.h"

namespace hcal {

/**
 * @class HcalRecProducer
 * @brief Performs basic HCal reconstruction
 *
 * Reconstruction is done from the HcalDigi samples.
 * Some hard-coded parameters are used for position and energy calculation.
 */
class HcalRecProducer : public framework::Producer {
 public:
  /**
   * Constructor
   */
  HcalRecProducer(const std::string& name, framework::Process& process);

  /**
   * Destructor
   */
  virtual ~HcalRecProducer();

  /**
   * Grabs configure parameters from the python config file.
   */
  virtual void configure(framework::config::Parameters&);

  /**
   * Produce HcalHits and put them into the event bus using the
   * HcalDigis as input.
   *
   * This function unfolds the digi samples taken by the HGC ROC
   * and reconstructs their energy using knowledge of how
   * the chip operates and the position using HcalGeometry.
   */
  virtual void produce(framework::Event& event);

 private:
  /** Digi Collection Name to use as input */
  std::string digiCollName_;

  /** Digi Pass Name to use as input */
  std::string digiPassName_;

  /// simhit collection name
  std::string simHitCollName_;

  /// simhit pass name
  std::string simHitPassName_;

  /// output hit collection name
  std::string recHitCollName_;

  /// Energy [MeV] deposited by a MIP
  double mip_energy_;

  /// PEs per MIP
  double pe_per_mip_;

  /// Length of clock cycle [ns]
  double clock_cycle_;

  /// Voltage by average MIP
  double voltage_per_mip_;

  /// Gain [mv/ADC]
  double gain_;

  /// Pedestal [ADC units]
  double pedestal_;

  /// Strip attenuation length [m]
  double attlength_;
};
}  // namespace hcal

#endif
