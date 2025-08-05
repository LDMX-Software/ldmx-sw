/**
 * @file PFCandidate.h
 * @brief Class that represents a reconstructed particle candidate
 * @author Christian Herwig, Fermilab
 */

#ifndef RECON_EVENT_PFCANDIDATE_H_
#define RECON_EVENT_PFCANDIDATE_H_

// ROOT
#include "TObject.h"  //For ClassDef

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
    posTargX_ = x_;
    posTargY_ = y_;
    posTargZ_ = z_;
  }
  void setEcalPositionXYZ(float x_, float y_, float z_) {
    posEcalX_ = x_;
    posEcalY_ = y_;
    posEcalZ_ = z_;
  }
  void setHcalPositionXYZ(float x_, float y_, float z_) {
    posHcalX_ = x_;
    posHcalY_ = y_;
    posHcalZ_ = z_;
  }

  void setTrackPxPyPz(float x_, float y_, float z_) {
    trackPx_ = x_;
    trackPy_ = y_;
    trackPz_ = z_;
  }

  void setEcalEnergy(float x_) { ecalEnergy_ = x_; }
  void setEcalRawEnergy(float x_) { ecalRawEnergy_ = x_; }
  void setEcalClusterXYZ(float x_, float y_, float z_) {
    ecalClusterX_ = x_;
    ecalClusterY_ = y_;
    ecalClusterZ_ = z_;
  }
  void setEcalClusterEXYZ(float x_, float y_, float z_) {
    ecalClusterEX_ = x_;
    ecalClusterEY_ = y_;
    ecalClusterEZ_ = z_;
  }
  void setEcalClusterDXDZ(float x_) { ecalClusterDXDZ_ = x_; }
  void setEcalClusterDYDZ(float x_) { ecalClusterDYDZ_ = x_; }
  void setEcalClusterEDXDZ(float x_) { ecalClusterEDXDZ_ = x_; }
  void setEcalClusterEDYDZ(float x_) { ecalClusterEDYDZ_ = x_; }

  void setHcalEnergy(float x_) { hcalEnergy_ = x_; }
  void setHcalRawEnergy(float x_) { hcalRawEnergy_ = x_; }
  void setHcalClusterXYZ(float x_, float y_, float z_) {
    hcalClusterX_ = x_;
    hcalClusterY_ = y_;
    hcalClusterZ_ = z_;
  }
  void setHcalClusterEXYZ(float x_, float y_, float z_) {
    hcalClusterEX_ = x_;
    hcalClusterEY_ = y_;
    hcalClusterEZ_ = z_;
  }
  void setHcalClusterDXDZ(float x_) { hcalClusterDXDZ_ = x_; }
  void setHcalClusterDYDZ(float x_) { hcalClusterDYDZ_ = x_; }
  void setHcalClusterEDXDZ(float x_) { hcalClusterEDXDZ_ = x_; }
  void setHcalClusterEDYDZ(float x_) { hcalClusterEDYDZ_ = x_; }

  void setTruthEcalXYZ(double x_, double y_, double z_) {
    truthEcalX_ = x_;
    truthEcalY_ = y_;
    truthEcalZ_ = z_;
  }
  void setTruthPxPyPz(double x_, double y_, double z_) {
    truthPx_ = x_;
    truthPy_ = y_;
    truthPz_ = z_;
  }
  void setTruthMass(double x_) { truthMass_ = x_; }
  void setTruthEnergy(double x_) { truthEnergy_ = x_; }
  void setTruthPdgId(int x_) { truthPdgId_ = x_; }

  /*
     Getters
   */
  int getPID() const { return pid_; }
  float getMass() const { return mass_; }
  float getEnergy() const { return energy_; }

  std::vector<float> getTargetPositionXYZ() const {
    return {posTargX_, posTargY_, posTargZ_};
  }
  std::vector<float> getEcalPositionXYZ() const {
    return {posEcalX_, posEcalY_, posEcalZ_};
  }
  std::vector<float> getHcalPositionXYZ() const {
    return {posHcalX_, posHcalY_, posHcalZ_};
  }

  std::vector<float> getTrackPxPyPz() const {
    return {trackPx_, trackPy_, trackPz_};
  }

  float getEcalEnergy() const { return ecalEnergy_; }
  float getEcalRawEnergy() const { return ecalRawEnergy_; }
  std::vector<float> getEcalClusterXYZ() const {
    return {ecalClusterX_, ecalClusterY_, ecalClusterZ_};
  }
  std::vector<float> getEcalClusterEXYZ() const {
    return {ecalClusterEX_, ecalClusterEY_, ecalClusterEZ_};
  }
  float getEcalClusterDXDZ() const { return ecalClusterDXDZ_; }
  float getEcalClusterDYDZ() const { return ecalClusterDYDZ_; }
  float getEcalClusterEDXDZ() const { return ecalClusterEDXDZ_; }
  float getEcalClusterEDYDZ() const { return ecalClusterEDYDZ_; }

  float getHcalEnergy() const { return hcalEnergy_; }
  float getHcalRawEnergy() const { return hcalRawEnergy_; }
  std::vector<float> getHcalClusterXYZ() const {
    return {hcalClusterX_, hcalClusterY_, hcalClusterZ_};
  }
  std::vector<float> getHcalClusterEXYZ() const {
    return {hcalClusterEX_, hcalClusterEY_, hcalClusterEZ_};
  }
  float getHcalClusterDXDZ() const { return hcalClusterDXDZ_; }
  float getHcalClusterDYDZ() const { return hcalClusterDYDZ_; }
  float getHcalClusterEDXDZ() const { return hcalClusterEDXDZ_; }
  float getHcalClusterEDYDZ() const { return hcalClusterEDYDZ_; }

  std::vector<double> const getTruthEcalXYZ() {
    return {truthEcalX_, truthEcalY_, truthEcalZ_};
  }
  std::vector<double> const getTruthPxPyPz() {
    return {truthPx_, truthPy_, truthPz_};
  }
  double getTruthMass() { return truthMass_; }
  double getTruthEnergy() { return truthEnergy_; }
  int getTruthPdgId() { return truthPdgId_; }

 private:
  /* Particle ID enum */
  int pid_{0};
  float mass_{0};
  float energy_{0};

  /* Position at the Target */
  float posTargX_{0};
  float posTargY_{0};
  float posTargZ_{0};
  /* Position at the ECal face */
  float posEcalX_{0};
  float posEcalY_{0};
  float posEcalZ_{0};
  /* Position at the HCal face */
  float posHcalX_{0};
  float posHcalY_{0};
  float posHcalZ_{0};

  /* track momenta */
  float trackPx_{0};
  float trackPy_{0};
  float trackPz_{0};

  /* Ecal energy, cluster info */
  float ecalEnergy_{0};
  float ecalRawEnergy_{0};
  float ecalClusterX_{0};
  float ecalClusterY_{0};
  float ecalClusterZ_{0};
  float ecalClusterEX_{0};
  float ecalClusterEY_{0};
  float ecalClusterEZ_{0};
  float ecalClusterDXDZ_{0};
  float ecalClusterDYDZ_{0};
  float ecalClusterEDXDZ_{0};
  float ecalClusterEDYDZ_{0};

  /* Hcal energy, cluster info */
  float hcalEnergy_{0};
  float hcalRawEnergy_{0};
  float hcalClusterX_{0};
  float hcalClusterY_{0};
  float hcalClusterZ_{0};
  float hcalClusterEX_{0};
  float hcalClusterEY_{0};
  float hcalClusterEZ_{0};
  float hcalClusterDXDZ_{0};
  float hcalClusterDYDZ_{0};
  float hcalClusterEDXDZ_{0};
  float hcalClusterEDYDZ_{0};

  /* Information for truth matched particles */
  double truthEcalX_{0};
  double truthEcalY_{0};
  double truthEcalZ_{0};
  double truthPx_{0};
  double truthPy_{0};
  double truthPz_{0};
  double truthMass_{0};
  double truthEnergy_{0};
  int truthPdgId_{0};

  /* The ROOT class definition. */
  ClassDef(PFCandidate, 2);
};
}  // namespace ldmx

#endif /* RECON_EVENT_PFCANDIDATE_H_ */
