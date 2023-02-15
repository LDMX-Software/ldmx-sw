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

//---------//
//  ROOT   //
//---------//
#include "TF1.h"
#include "TGraph.h"

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
  ~HcalRecProducer() = default;

  /**
   * Grabs configure parameters from the python config file.
   */
  void configure(framework::config::Parameters&) final override;

  /**
   * Gets Time of Arrival with respect to the SOI.
   */
  double getTOA(const ldmx::HgcrocDigiCollection::HgcrocDigi digi,
                double pedestal, unsigned int iSOI);

  /**
   * Produce HcalHits and put them into the event bus using the
   * HcalDigis as input.
   *
   * This function unfolds the digi samples taken by the HGC ROC
   * and reconstructs their energy using knowledge of how
   * the chip operates and the position using HcalGeometry.
   */
  void produce(framework::Event& event) final override;

 private:
  /// Digi Collection Name to use as input
  std::string digiCollName_;

  /// Digi Pass Name to use as input
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

  /// Strip attenuation length [m]
  double attlength_;

  /// Pulse function
  mutable TF1 pulseFunc_;

  /**
   * Correction to the pulse's measured amplitude at the peak.
   * The correction is calculated by comparing the amplitude at the sample time
   *(T) over its correct value (1.0) with the ratio between sample T and sample
   *T+25ns.
   **/
  mutable TGraph correctionAmpl_;

  /**
   * Correction to the measured TOA relative to the peak.
   * This corrects for the time-walk effect where the time that the front edge
   * of the pulse crosses the TOA threshold walks higher in time as the pulse's
   * amplitude gets smaller. The correction is calculated by comparing the TOA
   * measured relative to the peak (i.e. the time at which the pulse crosses the
   * TOA threshold) with the amplitude at the sample time (T) over its correct
   * value (1.0).
   */
  mutable TGraph correctionTOA_;

  /// Minimum amplitude fraction to apply amplitude correction
  double minAmplFraction_;

  /// Minimum amplitude to apply TOA correction
  double minAmpl_;

  /// Depth of ADC buffer.
  int nADCs_;

  /// Rate of Up Slope in Pulse Shape [1/ns]
  double rateUpSlope_;

  /// Time of Up Slope relative to Pulse Shape Fit [ns]
  double timeUpSlope_;

  /// Rate of Down Slope in Pulse Shape [1/ns]
  double rateDnSlope_;

  /// Time of Down Slope relative to Pulse Shape Fit [ns]
  double timeDnSlope_;

  /// Time of Peak relative to pulse shape fit [ns]
  double timePeak_;
};
}  // namespace hcal

#endif
