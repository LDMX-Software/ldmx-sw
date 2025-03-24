/**
 * @file EcalWABResult.h
 * @brief Class used to encapsulate the results obtained from
 * EcalWABRecProcessor
 * @author Sanjit Masanam, UCSB
 */

#ifndef EVENT_ECALWABRESULT_H_
#define EVENT_ECALWABRESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>  //For ClassDef

namespace ldmx {

class EcalWABResult {
 public:
  EcalWABResult() = default;

  /**
   * Destructor.
   *
   * Currently, the destructor does nothing.
   */
  virtual ~EcalWABResult() = default;

  /**
   * Print the string representation of this object.
   *
   * This method is needed by ROOT when building the dictionary.
   */
  void Print() const;

  // To match the Framework Bus clear. It's doing nothing.
  void Clear() {}

  void setVariables(
      float true_theta_electron, float true_theta_photon,
      float true_phi_electron, float true_phi_photon, float rec_theta_electron,
      float rec_theta_photon, float rec_phi_electron, float rec_phi_photon,
      float true_theta_diff_electron_photon,
      float true_phi_diff_electron_photon, float rec_theta_diff_electron_photon,
      float rec_phi_diff_electron_photon, float true_rec_theta_diff_electron,
      float true_rec_phi_diff_electron, float true_rec_theta_diff_photon,
      float true_rec_phi_diff_photon, float true_electron_shower_energy,
      float true_photon_shower_energy, float rec_electron_shower_energy,
      float rec_photon_shower_energy, int progress_num) {
    true_theta_electron_ = true_theta_electron;
    true_theta_photon_ = true_theta_photon;
    true_phi_electron_ = true_phi_electron;
    true_phi_photon_ = true_phi_photon;
    rec_theta_electron_ = rec_theta_electron;
    rec_theta_photon_ = rec_theta_photon;
    rec_phi_electron_ = rec_phi_electron;
    rec_phi_photon_ = rec_phi_photon;
    true_theta_diff_electron_photon_ = true_theta_diff_electron_photon;
    true_phi_diff_electron_photon_ = true_phi_diff_electron_photon;
    rec_theta_diff_electron_photon_ = rec_theta_diff_electron_photon;
    rec_phi_diff_electron_photon_ = rec_phi_diff_electron_photon;
    true_rec_theta_diff_electron_ = true_rec_theta_diff_electron;
    true_rec_phi_diff_electron_ = true_rec_phi_diff_electron;
    true_rec_theta_diff_photon_ = true_rec_theta_diff_photon;
    true_rec_phi_diff_photon_ = true_rec_phi_diff_photon;
    true_electron_shower_energy_ = true_electron_shower_energy;
    true_photon_shower_energy_ = true_photon_shower_energy;
    rec_electron_shower_energy_ = rec_electron_shower_energy;
    rec_photon_shower_energy_ = rec_photon_shower_energy;
    progress_num_ = progress_num;
  }

  void setTrueThetaElectron(float true_theta_electron) {
    true_theta_electron_ = true_theta_electron;
  }
  float getTrueThetaElectron() const { return true_theta_electron_; }

  void setTrueThetaPhoton(float true_theta_photon) {
    true_theta_photon_ = true_theta_photon;
  }
  float getTrueThetaPhoton() const { return true_theta_photon_; }

  void setTruePhiElectron(float true_phi_electron) {
    true_phi_electron_ = true_phi_electron;
  }
  float getTruePhiElectron() const { return true_phi_electron_; }

  void setTruePhiPhoton(float true_phi_photon) {
    true_phi_photon_ = true_phi_photon;
  }
  float getTruePhiPhoton() const { return true_phi_photon_; }

  void setRecThetaElectron(float rec_theta_electron) {
    rec_theta_electron_ = rec_theta_electron;
  }
  float getRecThetaElectron() const { return rec_theta_electron_; }

  void setRecThetaPhoton(float rec_theta_photon) {
    rec_theta_photon_ = rec_theta_photon;
  }
  float getRecThetaPhoton() const { return rec_theta_photon_; }

  void setRecPhiElectron(float rec_phi_electron) {
    rec_phi_electron_ = rec_phi_electron;
  }
  float getRecPhiElectron() const { return rec_phi_electron_; }

  void setRecPhiPhoton(float rec_phi_photon) {
    rec_phi_photon_ = rec_phi_photon;
  }
  float getRecPhiPhoton() const { return rec_phi_photon_; }

  void setTrueThetaDiffElectronPhoton(float true_theta_diff_electron_photon) {
    true_theta_diff_electron_photon_ = true_theta_diff_electron_photon;
  }
  float getTrueThetaDiffElectronPhoton() const {
    return true_theta_diff_electron_photon_;
  }

  void setTruePhiDiffElectronPhoton(float true_phi_diff_electron_photon) {
    true_phi_diff_electron_photon_ = true_phi_diff_electron_photon;
  }
  float getTruePhiDiffElectronPhoton() const {
    return true_phi_diff_electron_photon_;
  }

  void setRecThetaDiffElectronPhoton(float rec_theta_diff_electron_photon) {
    rec_theta_diff_electron_photon_ = rec_theta_diff_electron_photon;
  }
  float getRecThetaDiffElectronPhoton() const {
    return rec_theta_diff_electron_photon_;
  }

  void setRecPhiDiffElectronPhoton(float rec_phi_diff_electron_photon) {
    rec_phi_diff_electron_photon_ = rec_phi_diff_electron_photon;
  }
  float getRecPhiDiffElectronPhoton() const {
    return rec_phi_diff_electron_photon_;
  }

  void setTrueRecThetaDiffElectron(float true_rec_theta_diff_electron) {
    true_rec_theta_diff_electron_ = true_rec_theta_diff_electron;
  }
  float getTrueRecThetaDiffElectron() const {
    return true_rec_theta_diff_electron_;
  }

  void setTrueRecPhiDiffElectron(float true_rec_phi_diff_electron) {
    true_rec_phi_diff_electron_ = true_rec_phi_diff_electron;
  }
  float getTrueRecPhiDiffElectron() const {
    return true_rec_phi_diff_electron_;
  }

  void setTrueRecThetaDiffPhoton(float true_rec_theta_diff_photon) {
    true_rec_theta_diff_photon_ = true_rec_theta_diff_photon;
  }
  float getTrueRecThetaDiffPhoton() const {
    return true_rec_theta_diff_photon_;
  }

  void setTrueRecPhiDiffPhoton(float true_rec_phi_diff_photon) {
    true_rec_phi_diff_photon_ = true_rec_phi_diff_photon;
  }
  float getTrueRecPhiDiffPhoton() const { return true_rec_phi_diff_photon_; }

  void setTrueElectronShowerEnergy(float true_electron_shower_energy) {
    true_electron_shower_energy_ = true_electron_shower_energy;
  }
  float getTrueElectronShowerEnergy() const {
    return true_electron_shower_energy_;
  }

  void setTruePhotonShowerEnergy(float true_photon_shower_energy) {
    true_photon_shower_energy_ = true_photon_shower_energy;
  }
  float getTruePhotonShowerEnergy() const { return true_photon_shower_energy_; }

  void setRecElectronShowerEnergy(float rec_electron_shower_energy) {
    rec_electron_shower_energy_ = rec_electron_shower_energy;
  }
  float getRecElectronShowerEnergy() const {
    return rec_electron_shower_energy_;
  }

  void setRecPhotonShowerEnergy(float rec_photon_shower_energy) {
    rec_photon_shower_energy_ = rec_photon_shower_energy;
  }
  float getRecPhotonShowerEnergy() const { return rec_photon_shower_energy_; }

  void setProgressNum(int progress_num) { progress_num_ = progress_num; }
  float getProgressNum() const { return progress_num_; }

 protected:
  // Actual Kinematic Parameters
  float true_theta_electron_;
  float true_theta_photon_;
  float true_phi_electron_;
  float true_phi_photon_;
  float rec_theta_electron_;
  float rec_theta_photon_;
  float rec_phi_electron_;
  float rec_phi_photon_;
  float true_theta_diff_electron_photon_;
  float true_phi_diff_electron_photon_;
  float rec_theta_diff_electron_photon_;
  float rec_phi_diff_electron_photon_;
  float true_rec_theta_diff_electron_;
  float true_rec_phi_diff_electron_;
  float true_rec_theta_diff_photon_;
  float true_rec_phi_diff_photon_;
  float true_electron_shower_energy_;
  float true_photon_shower_energy_;
  float rec_electron_shower_energy_;
  float rec_photon_shower_energy_;
  int progress_num_;

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(EcalWABResult, 1);
};  // EcalWABResult

typedef std::vector<ldmx::EcalWABResult> EcalWABResults;

}  // namespace ldmx

#endif  // EVENT_ECALWABRESULT_H_
