/**
 * @file PFCandidate.h
 * @brief Class that represents a reconstructed particle candidate
 * @author Christian Herwig, Fermilab
 */

#ifndef RECON_EVENT_PFCANDIDATE_H_
#define RECON_EVENT_PFCANDIDATE_H_

// ROOT
#include "TObject.h"  //For ClassDef
// ldmx-sw objects
// #include "Ecal/Event/EcalHit.h"
// #include "Hcal/Event/HcalHit.h"

namespace ldmx {

/**
 * @class PFCandidate
 * @brief Represents a reconstructed particle
 */
class PFCandidate {
 public:
  PFCandidate() = default;
  virtual ~PFCandidate() {}

  friend std::ostream &operator<<(std::ostream &o, const PFCandidate &d);

  bool operator<(const PFCandidate &rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

  /*
     Setters
   */
  void setPID(int x_) { pid_ = x_; }
  void setMass(float x_) { mass_ = x_; }
  void setEnergy(float x_) { energy_ = x_; }

  void setTargetPositionXYZ(float x_, float y_, float z_) {
    pos_targ_x_ = x_;
    pos_targ_y_ = y_;
    pos_targ_z_ = z_;
  }
  void setEcalPositionXYZ(float x_, float y_, float z_) {
    pos_ecal_x_ = x_;
    pos_ecal_y_ = y_;
    pos_ecal_z_ = z_;
  }
  void setHcalPositionXYZ(float x_, float y_, float z_) {
    pos_hcal_x_ = x_;
    pos_hcal_y_ = y_;
    pos_hcal_z_ = z_;
  }
  void setTrackPxPyPz(float x_, float y_, float z_) {
    track_px_ = x_;
    track_py_ = y_;
    track_pz_ = z_;
  }
  // associate component indices to the pf candidate
  void setTrackIndex(int x) { track_idx_ = x; }
  void setEcalIndex(int x) { ecal_idx_ = x; }
  void setHcalIndex(int x) { hcal_idx_ = x; }

  void setEcalEnergy(float x_) { ecal_energy_ = x_; }
  void setEcalRawEnergy(float x_) { ecal_raw_energy_ = x_; }
  void setEcalClusterXYZ(float x_, float y_, float z_) {
    ecal_cluster_x_ = x_;
    ecal_cluster_y_ = y_;
    ecal_cluster_z_ = z_;
  }
  void setEcalClusterEXYZ(float x_, float y_, float z_) {
    ecal_cluster_ex_ = x_;
    ecal_cluster_ey_ = y_;
    ecal_cluster_ez_ = z_;
  }
  void setEcalClusterDXDZ(float x_) { ecal_cluster_dxdz_ = x_; }
  void setEcalClusterDYDZ(float x_) { ecal_cluster_dydz_ = x_; }
  void setEcalClusterEDXDZ(float x_) { ecal_cluster_edxdz_ = x_; }
  void setEcalClusterEDYDZ(float x_) { ecal_cluster_edydz_ = x_; }

  void setHcalEnergy(float x_) { hcal_energy_ = x_; }
  void setHcalRawEnergy(float x_) { hcal_raw_energy_ = x_; }
  void setHcalClusterXYZ(float x_, float y_, float z_) {
    hcal_cluster_x_ = x_;
    hcal_cluster_y_ = y_;
    hcal_cluster_z_ = z_;
  }
  void setHcalClusterEXYZ(float x_, float y_, float z_) {
    hcal_cluster_ex_ = x_;
    hcal_cluster_ey_ = y_;
    hcal_cluster_ez_ = z_;
  }
  void setHcalClusterDXDZ(float x_) { hcal_cluster_dxdz_ = x_; }
  void setHcalClusterDYDZ(float x_) { hcal_cluster_dydz_ = x_; }
  void setHcalClusterEDXDZ(float x_) { hcal_cluster_edxdz_ = x_; }
  void setHcalClusterEDYDZ(float x_) { hcal_cluster_edydz_ = x_; }

  void setTruthEcalXYZ(double x_, double y_, double z_) {
    truth_ecal_x_ = x_;
    truth_ecal_y_ = y_;
    truth_ecal_z_ = z_;
  }
  void setTruthPxPyPz(double x_, double y_, double z_) {
    truth_px_ = x_;
    truth_py_ = y_;
    truth_pz_ = z_;
  }
  void setTruthMass(double x_) { truth_mass_ = x_; }
  void setTruthEnergy(double x_) { truth_energy_ = x_; }
  void setTruthPdgId(int x_) { truth_pdg_id_ = x_; }

  /**
   * Take in the ecal hits that make up the candidate.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  // void setEcalHits(const std::vector<const ldmx::EcalHit*> hits) { ecal_hits_
  // = hits; }

  /**
   * Take in the hcal hits that make up the candidate.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  //  void setHcalHits(const std::vector<const ldmx::HcalHit*> hits) {
  //  hcal_hits_ = hits; }
  /*
     Getters
   */
  int getPID() const { return pid_; }
  float getMass() const { return mass_; }
  float getEnergy() const { return energy_; }

  std::vector<float> getTargetPositionXYZ() const {
    return {pos_targ_x_, pos_targ_y_, pos_targ_z_};
  }
  std::vector<float> getEcalPositionXYZ() const {
    return {pos_ecal_x_, pos_ecal_y_, pos_ecal_z_};
  }
  std::vector<float> getHcalPositionXYZ() const {
    return {pos_hcal_x_, pos_hcal_y_, pos_hcal_z_};
  }
  // associate component indices to the pf candidate
  int getTrackIndex() const { return track_idx_; }
  int getEcalIndex() const { return ecal_idx_; }
  int getHcalIndex() const { return hcal_idx_; }

  std::vector<float> getTrackPxPyPz() const {
    return {track_px_, track_py_, track_pz_};
  }

  float getEcalEnergy() const { return ecal_energy_; }
  float getEcalRawEnergy() const { return ecal_raw_energy_; }
  std::vector<float> getEcalClusterXYZ() const {
    return {ecal_cluster_x_, ecal_cluster_y_, ecal_cluster_z_};
  }
  std::vector<float> getEcalClusterEXYZ() const {
    return {ecal_cluster_ex_, ecal_cluster_ey_, ecal_cluster_ez_};
  }
  float getEcalClusterDXDZ() const { return ecal_cluster_dxdz_; }
  float getEcalClusterDYDZ() const { return ecal_cluster_dydz_; }
  float getEcalClusterEDXDZ() const { return ecal_cluster_edxdz_; }
  float getEcalClusterEDYDZ() const { return ecal_cluster_edydz_; }

  float getHcalEnergy() const { return hcal_energy_; }
  float getHcalRawEnergy() const { return hcal_raw_energy_; }
  std::vector<float> getHcalClusterXYZ() const {
    return {hcal_cluster_x_, hcal_cluster_y_, hcal_cluster_z_};
  }
  std::vector<float> getHcalClusterEXYZ() const {
    return {hcal_cluster_ex_, hcal_cluster_ey_, hcal_cluster_ez_};
  }
  float getHcalClusterDXDZ() const { return hcal_cluster_dxdz_; }
  float getHcalClusterDYDZ() const { return hcal_cluster_dydz_; }
  float getHcalClusterEDXDZ() const { return hcal_cluster_edxdz_; }
  float getHcalClusterEDYDZ() const { return hcal_cluster_edydz_; }

  std::vector<double> const getTruthEcalXYZ() {
    return {truth_ecal_x_, truth_ecal_y_, truth_ecal_z_};
  }
  std::vector<double> const getTruthPxPyPz() {
    return {truth_px_, truth_py_, truth_pz_};
  }
  double getTruthMass() { return truth_mass_; }
  double getTruthEnergy() { return truth_energy_; }
  int getTruthPdgId() { return truth_pdg_id_; }

  /**
   * Take in the ecal hits that make up the candidate.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  // std::vector<const ldmx::EcalHit*> getEcalHits() { return ecal_hits_; }

  /**
   * Take in the hcal hits that make up the candidate.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  // std::vector<const ldmx::HcalHit*> getHcalHits() { return hcal_hits_; }

 private:
  /* Particle ID enum */
  int pid_{0};
  float mass_{0};
  float energy_{0};

  /* Position at the Target */
  float pos_targ_x_{0};
  float pos_targ_y_{0};
  float pos_targ_z_{0};
  /* Position at the ECal face */
  float pos_ecal_x_{0};
  float pos_ecal_y_{0};
  float pos_ecal_z_{0};
  /* Position at the HCal face */
  float pos_hcal_x_{0};
  float pos_hcal_y_{0};
  float pos_hcal_z_{0};

  /* track momenta */
  float track_px_{0};
  float track_py_{0};
  float track_pz_{0};

  /* Ecal energy, cluster info */
  float ecal_energy_{0};
  float ecal_raw_energy_{0};
  float ecal_cluster_x_{0};
  float ecal_cluster_y_{0};
  float ecal_cluster_z_{0};
  float ecal_cluster_ex_{0};
  float ecal_cluster_ey_{0};
  float ecal_cluster_ez_{0};
  float ecal_cluster_dxdz_{0};
  float ecal_cluster_dydz_{0};
  float ecal_cluster_edxdz_{0};
  float ecal_cluster_edydz_{0};

  /* Hcal energy, cluster info */
  float hcal_energy_{0};
  float hcal_raw_energy_{0};
  float hcal_cluster_x_{0};
  float hcal_cluster_y_{0};
  float hcal_cluster_z_{0};
  float hcal_cluster_ex_{0};
  float hcal_cluster_ey_{0};
  float hcal_cluster_ez_{0};
  float hcal_cluster_dxdz_{0};
  float hcal_cluster_dydz_{0};
  float hcal_cluster_edxdz_{0};
  float hcal_cluster_edydz_{0};

  /* Information for truth matched particles */
  double truth_ecal_x_{0};
  double truth_ecal_y_{0};
  double truth_ecal_z_{0};
  double truth_px_{0};
  double truth_py_{0};
  double truth_pz_{0};
  double truth_mass_{0};
  double truth_energy_{0};
  int truth_pdg_id_{0};

  /* Indices of the components making up the PFlow object */
  int track_idx_{-1};
  int ecal_idx_{-1};
  int hcal_idx_{-1};

  /* The ROOT class definition. */
  ClassDef(PFCandidate, 2);
};
}  // namespace ldmx

#endif /* RECON_EVENT_PFCANDIDATE_H_ */
