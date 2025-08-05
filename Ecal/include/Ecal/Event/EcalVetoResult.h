/**
 * @file EcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from
 *        EcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Danyi Zhang, Tamas Almos Vami (UCSB)
 */

#ifndef EVENT_ECALVETORESULT_H_
#define EVENT_ECALVETORESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <array>
#include <iostream>
#include <map>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>  //For ClassDef

namespace ldmx {

class EcalVetoResult {
 public:
  /** Constructor */
  EcalVetoResult() = default;

  /** Destructor */
  virtual ~EcalVetoResult();

  /**
   * Set the sim particle and 'is findable' flag.
   */
  void setVariables(
      int n_readout_hits, int deepest_layer_hit, int n_tracking_hits,
      float summed_det, float summed_tight_iso, float max_cell_dep,
      float shower_rms, float x_std, float y_std, float avg_layer_hit,
      float std_layer_hit, float ecal_back_energy, float ep_ang,
      float ep_ang_at_target, float ep_sep, float ep_dot,
      float ep_dot_at_target,

      std::vector<float> electron_containment_energy,
      std::vector<float> photon_containment_energy,
      std::vector<float> outside_containment_energy,
      std::vector<int> outside_containment_n_hits,
      std::vector<float> outside_containment_x_std,
      std::vector<float> outside_containment_y_std,

      std::vector<float> energy_seg, std::vector<float> x_mean_seg,
      std::vector<float> y_mean_seg, std::vector<float> x_std_seg,
      std::vector<float> y_std_seg, std::vector<float> layer_mean_seg,
      std::vector<float> layer_std_seg,

      std::vector<std::vector<float>> e_cont_energy,
      std::vector<std::vector<float>> e_cont_x_mean,
      std::vector<std::vector<float>> e_cont_y_mean,
      std::vector<std::vector<float>> g_cont_energy,
      std::vector<std::vector<int>> g_cont_n_hits,
      std::vector<std::vector<float>> g_cont_x_mean,
      std::vector<std::vector<float>> g_cont_y_mean,
      std::vector<std::vector<float>> o_cont_energy,
      std::vector<std::vector<int>> o_cont_n_hits,
      std::vector<std::vector<float>> o_cont_x_mean,
      std::vector<std::vector<float>> o_cont_y_mean,
      std::vector<std::vector<float>> o_cont_x_std,
      std::vector<std::vector<float>> o_cont_y_std,
      std::vector<std::vector<float>> o_cont_layer_mean,
      std::vector<std::vector<float>> o_cont_layer_std,

      std::vector<float> ecal_layer_edep_readout, std::array<float, 3> recoil_p,
      std::array<float, 3> recoil_pos);

  /** Reset the object. */
  void clear();

  /** Print the object */
  friend std::ostream& operator<<(std::ostream& o, const EcalVetoResult& d);

  /** Checks if the event passes the Ecal veto. */
  bool passesVeto() const { return passes_veto_; }

  float getDisc() const { return disc_value_; }

  bool getFiducial() const { return fiducial_; }

  int getDeepestLayerHit() const { return deepest_layer_hit_; }

  // Did ACTS find a recoil track in the tracker?
  bool getTrackingFiducial() const { return tracking_fiducial_; }

  int getNReadoutHits() const { return n_readout_hits_; }

  float getSummedDet() const { return summed_det_; }

  float getSummedTightIso() const { return summed_tight_iso_; }

  float getMaxCellDep() const { return max_cell_dep_; }

  float getShowerRMS() const { return shower_rms_; }

  float getXStd() const { return x_std_; }

  float getYStd() const { return y_std_; }

  float getAvgLayerHit() const { return avg_layer_hit_; }

  float getStdLayerHit() const { return std_layer_hit_; }

  float getEcalBackEnergy() const { return ecal_back_energy_; }

  int getNTrackingHits() const { return n_tracking_hits_; }

  float getEPAng() const { return ep_ang_; }

  float getEPAngAtTarget() const { return ep_ang_at_target_; }

  float getEPSep() const { return ep_sep_; }

  float getEPDot() const { return ep_dot_; }

  float getEPDotAtTarget() const { return ep_dot_at_target_; }

  const std::vector<float>& getElectronContainmentEnergy() const {
    return electron_containment_energy_;
  }

  const std::vector<float>& getPhotonContainmentEnergy() const {
    return photon_containment_energy_;
  }

  const std::vector<float>& getOutsideContainmentEnergy() const {
    return outside_containment_energy_;
  }

  const std::vector<int>& getOutsideContainmentNHits() const {
    return outside_containment_n_hits_;
  }

  const std::vector<float>& getOutsideContainmentXStd() const {
    return outside_containment_x_std_;
  }

  const std::vector<float>& getOutsideContainmentYStd() const {
    return outside_containment_y_std_;
  }

  const std::vector<float>& getEcalLayerEdepReadout() const {
    return ecal_layer_edep_readout_;
  }

  const std::vector<float>& getEnergySeg() const { return energy_seg_; }

  const std::vector<float>& getXMeanSeg() const { return x_mean_seg_; }

  const std::vector<float>& getYMeanSeg() const { return y_mean_seg_; }

  const std::vector<float>& getXStdSeg() const { return x_std_seg_; }

  const std::vector<float>& getYStdSeg() const { return y_std_seg_; }

  const std::vector<float>& getLayerMeanSeg() const { return layer_mean_seg_; }

  const std::vector<float>& getLayerStdSeg() const { return layer_std_seg_; }

  const std::vector<std::vector<float>>& getEleContEnergy() const {
    return e_cont_energy_;
  }

  const std::vector<std::vector<float>>& getEleContXMean() const {
    return e_cont_x_mean_;
  }

  const std::vector<std::vector<float>>& getEleContYMean() const {
    return e_cont_y_mean_;
  }

  const std::vector<std::vector<float>>& getPhContEnergy() const {
    return g_cont_energy_;
  }

  const std::vector<std::vector<int>>& getPhContNHits() const {
    return g_cont_n_hits_;
  }

  const std::vector<std::vector<float>>& getPhContXMean() const {
    return g_cont_x_mean_;
  }

  const std::vector<std::vector<float>>& getPhContYMean() const {
    return g_cont_y_mean_;
  }

  const std::vector<std::vector<float>>& getOutContEnergy() const {
    return o_cont_energy_;
  }

  const std::vector<std::vector<int>>& getOutContNHits() const {
    return o_cont_n_hits_;
  }

  const std::vector<std::vector<float>>& getOutContXMean() const {
    return o_cont_x_mean_;
  }

  const std::vector<std::vector<float>>& getOutContYMean() const {
    return o_cont_y_mean_;
  }

  const std::vector<std::vector<float>>& getOutContXStd() const {
    return o_cont_x_std_;
  }

  const std::vector<std::vector<float>>& getOutContYStd() const {
    return o_cont_y_std_;
  }

  const std::vector<std::vector<float>>& getOutContLayerMean() const {
    return o_cont_layer_mean_;
  }

  const std::vector<std::vector<float>>& getOutContLayerStd() const {
    return o_cont_layer_std_;
  }

  void setVetoResult(bool passes_veto) { passes_veto_ = passes_veto; }
  void setDiscValue(float disc_value) { disc_value_ = disc_value; }
  void setFiducial(bool fiducial) { fiducial_ = fiducial; }

  // Fiducial from the recoil tracking point of view
  void setTrackingFiducial(bool tracking_fiducial) {
    tracking_fiducial_ = tracking_fiducial;
  }

  /** Return the momentum of the recoil at the Ecal face. */
  const std::vector<float> getRecoilMomentum() const {
    return {recoil_px_, recoil_py_, recoil_pz_};
  };

  /** Return the x_ position of the recoil at the Ecal face. */
  float getRecoilX() const { return recoil_x_; };

  /** Return the y_ position of the recoil at the Ecal face. */
  float getRecoilY() const { return recoil_y_; };

 private:
  /** Flag indicating whether the event is vetoed by the Ecal. */
  bool passes_veto_{false};

  int n_readout_hits_{0};
  int deepest_layer_hit_{0};

  float summed_det_{0};
  float summed_tight_iso_{0};
  float max_cell_dep_{0};
  float shower_rms_{0};
  float x_std_{0};
  float y_std_{0};
  float avg_layer_hit_{0};
  float std_layer_hit_{0};
  float ecal_back_energy_{0};

  std::vector<float> electron_containment_energy_;
  std::vector<float> photon_containment_energy_;
  std::vector<float> outside_containment_energy_;
  std::vector<int> outside_containment_n_hits_;
  std::vector<float> outside_containment_x_std_;
  std::vector<float> outside_containment_y_std_;

  std::vector<float> energy_seg_;
  std::vector<float> x_mean_seg_;
  std::vector<float> y_mean_seg_;
  std::vector<float> x_std_seg_;
  std::vector<float> y_std_seg_;
  std::vector<float> layer_mean_seg_;
  std::vector<float> layer_std_seg_;

  std::vector<std::vector<float>> e_cont_energy_;
  std::vector<std::vector<float>> e_cont_x_mean_;
  std::vector<std::vector<float>> e_cont_y_mean_;
  std::vector<std::vector<float>> g_cont_energy_;
  std::vector<std::vector<int>> g_cont_n_hits_;
  std::vector<std::vector<float>> g_cont_x_mean_;
  std::vector<std::vector<float>> g_cont_y_mean_;
  std::vector<std::vector<float>> o_cont_energy_;
  std::vector<std::vector<int>> o_cont_n_hits_;
  std::vector<std::vector<float>> o_cont_x_mean_;
  std::vector<std::vector<float>> o_cont_y_mean_;
  std::vector<std::vector<float>> o_cont_x_std_;
  std::vector<std::vector<float>> o_cont_y_std_;
  std::vector<std::vector<float>> o_cont_layer_mean_;
  std::vector<std::vector<float>> o_cont_layer_std_;

  /** discriminator value from the BDT */
  float disc_value_{0};

  /** is the recoil electron fiducial in ECAL?*/
  bool fiducial_{false};

  /** is the recoil electron fiducial in Tracker?*/
  bool tracking_fiducial_{false};

  /** px of recoil electron at the Ecal face. */
  float recoil_px_{-9999};

  /** py of recoil electron at the Ecal face. */
  float recoil_py_{-9999};

  /** pz of recoil electron at the Ecal face. */
  float recoil_pz_{-9999};

  /** x_ position of recoil electron at the Ecal face. */
  float recoil_x_{-9999};

  /** y_ position of recoil electron at the Ecal face. */
  float recoil_y_{-9999};

  /// Number of hits_ outside of the electron roc in the Ecal
  /// or if the electron trajectory is missing, all the hits_ in the Ecal
  int n_tracking_hits_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as projected at the ECAL
  float ep_ang_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as projected at the target
  float ep_ang_at_target_{0};

  /// Distance between the projected photon and electron trajectories at the
  /// ECal face
  float ep_sep_{0};
  /// Dot product of the photon and electron momenta unit vectors as at ECAL
  float ep_dot_{0};
  /// Dot product of the photon and electron momenta unit vectors as at Target
  float ep_dot_at_target_{0};

  std::vector<float> ecal_layer_edep_readout_;

  ClassDef(EcalVetoResult, 12);
};
}  // namespace ldmx

#endif
