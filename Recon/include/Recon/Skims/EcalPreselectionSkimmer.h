/**
 * @file EcalPreselectionSkimmer.h
 * @brief Processor used to pre-select events for the ECAL studies
 * @author Tamas Almos Vami (UCSB)
 */

#ifndef RECON_SKIMS_EcalPreselectionSkimmer_H_
#define RECON_SKIMS_EcalPreselectionSkimmer_H_

//----------//
//   LDMX   //
//----------//
#include "Ecal/Event/EcalVetoResult.h"
#include "Framework/EventProcessor.h"

namespace recon {

class EcalPreselectionSkimmer : public framework::Producer {
 public:
  /** Constructor */
  EcalPreselectionSkimmer(const std::string &name, framework::Process &process);

  /** Destructor */
  virtual ~EcalPreselectionSkimmer() = default;

  // Configure this processor with a set of parameters passed
  virtual void configure(framework::config::Parameters &) final;

  /**
   * Run the processor and select events that pass pre-selection in ECAL
   *
   * @param event The event to process.
   */
  virtual void produce(framework::Event &event) final;

 private:
  /// Collection Name for veto object
  std::string ecal_veto_name_;
  /// Pass Name for veto object
  std::string ecal_veto_pass_;
  /// Max value for summed det
  double summed_det_max_;
  /// Max value for summed tigh iso
  double summed_tight_iso_max_;
  /// Max value for ecal back energy
  double ecal_back_energy_max_;
  /// Max value for num readout hits
  int n_readout_hits_max_;
  /// Max value for shower rms
  double shower_rms_max_;
  /// Max value for shower rms in Y
  double shower_y_std_max_;
  /// Max value for shower rms in X
  double shower_x_std_max_;
  /// Max value for maximal cell deposition
  double max_cell_dep_max_;
  /// Max value for std layer hits
  int std_layer_hit_max_;
  /// Max value for num straight tracks
  int n_straight_tracks_max_;
  /// Min value for the BDT disc variable
  double bdt_disc_min_;
  /**
   * Level of interest in fiducial events
   *  0: don't care if it's fiducial or not,
   *  1: keep fiducial events only,
   *  2: keep non-fid events only
   */
  int fiducial_level_;

};  // EcalPreselectionSkimmer
}  // namespace recon

#endif  // RECON_SKIMS_EcalPreselectionSkimmer_H_
