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
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"

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
  virtual ~HcalRecProducer() = default;

  /**
   * Grabs configure parameters from the python config file.
   */
  void configure(framework::config::Parameters&) override;

  /**
   * Gets Time of Arrival with respect to the SOI.
   */
  double getTOA(const ldmx::HgcrocDigiCollection::HgcrocDigi digi,
                double pedestal, unsigned int iSOI) const;

  /**
   * Produce HcalHits and put them into the event bus using the
   * HcalDigis as input.
   *
   * This function unfolds the digi samples taken by the HGC ROC
   * and reconstructs their energy using knowledge of how
   * the chip operates and the position using HcalGeometry.
   */
  void produce(framework::Event& event) override;

 private:
  /// Digi Collection Name to use as input
  std::string digi_coll_name_;

  /// Digi Pass Name to use as input
  std::string digi_pass_name_;

  /// simhit collection name
  std::string sim_hit_coll_name_;

  /// simhit pass name
  std::string sim_hit_pass_name_;

  /// output hit collection name
  std::string rec_hit_coll_name_;

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
  mutable TF1 pulse_func_;

  /**
   * Correction to the pulse's measured amplitude at the peak.
   * The correction is calculated by comparing the amplitude at the sample time
   *(T) over its correct value (1.0) with the ratio between sample T and sample
   *T+25ns.
   **/
  mutable TGraph correction_ampl_;

  /**
   * Correction to the measured TOA relative to the peak.
   * This corrects for the time-walk effect where the time that the front edge
   * of the pulse crosses the TOA threshold walks higher in time as the pulse's
   * amplitude gets smaller. The correction is calculated by comparing the TOA
   * measured relative to the peak (i.e. the time at which the pulse crosses the
   * TOA threshold) with the amplitude at the sample time (T) over its correct
   * value (1.0).
   */
  mutable TGraph correction_toa_;

  /// Minimum amplitude fraction to apply amplitude correction
  double min_ampl_fraction_;

  /// Minimum amplitude to apply TOA correction
  double min_ampl_;

  /// Depth of ADC buffer.
  int n_ad_cs_;

  /// Rate of Up Slope in Pulse Shape [1/ns]
  double rate_up_slope_;

  /// Time of Up Slope relative to Pulse Shape Fit [ns]
  double time_up_slope_;

  /// Rate of Down Slope in Pulse Shape [1/ns]
  double rate_dn_slope_;

  /// Time of Down Slope relative to Pulse Shape Fit [ns]
  double time_dn_slope_;

  /// Time of Peak relative to pulse shape fit [ns]
  double time_peak_;
};
}  // namespace hcal

#endif
