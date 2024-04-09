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
  PFCandidate() {}
  virtual ~PFCandidate() {}

  void Print() const;
  bool operator<(const PFCandidate &rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

  /* 
     Setters
   */
  void setPID(int x) {pid_ = x;}
  void setMass(float x) {mass_ = x;}
  void setEnergy(float x) {energy_ = x;}

  void setTargetPositionXYZ(float x, float y, float z){
      posTargX_=x; posTargY_=y; posTargZ_=z;
  }
  void setEcalPositionXYZ(float x, float y, float z){
      posEcalX_=x; posEcalY_=y; posEcalZ_=z;
  }
  void setHcalPositionXYZ(float x, float y, float z){
      posHcalX_=x; posHcalY_=y; posHcalZ_=z;
  }

  void setTrackPxPyPz(float x, float y, float z){
      trackPx_=x; trackPy_=y; trackPz_=z;
  }

  void setEcalEnergy(float x) {ecalEnergy_ = x;}
  void setEcalRawEnergy(float x) {ecalRawEnergy_ = x;}
  void setEcalClusterXYZ(float x, float y, float z){
      ecalClusterX_=x; ecalClusterY_=y; ecalClusterZ_=z;
  }
  void setEcalClusterEXYZ(float x, float y, float z){
      ecalClusterEX_=x; ecalClusterEY_=y; ecalClusterEZ_=z;
  }
  void setEcalClusterDXDZ(float x){ ecalClusterDXDZ_=x; }
  void setEcalClusterDYDZ(float x){ ecalClusterDYDZ_=x; }
  void setEcalClusterEDXDZ(float x){ ecalClusterEDXDZ_=x; }
  void setEcalClusterEDYDZ(float x){ ecalClusterEDYDZ_=x; }

  void setHcalEnergy(float x) {hcalEnergy_ = x;}
  void setHcalRawEnergy(float x) {hcalRawEnergy_ = x;}
  void setHcalClusterXYZ(float x, float y, float z){
      hcalClusterX_=x; hcalClusterY_=y; hcalClusterZ_=z;
  }
  void setHcalClusterEXYZ(float x, float y, float z){
      hcalClusterEX_=x; hcalClusterEY_=y; hcalClusterEZ_=z;
  }
  void setHcalClusterDXDZ(float x){ hcalClusterDXDZ_=x; }
  void setHcalClusterDYDZ(float x){ hcalClusterDYDZ_=x; }
  void setHcalClusterEDXDZ(float x){ hcalClusterEDXDZ_=x; }
  void setHcalClusterEDYDZ(float x){ hcalClusterEDYDZ_=x; }

  void setTruthEcalXYZ(double x, double y, double z){
      truthEcalX_=x; truthEcalY_=y; truthEcalZ_=z;
  }
  void setTruthPxPyPz(double x, double y, double z){
      truthPx_=x; truthPy_=y; truthPz_=z;
  }
  void setTruthMass(double x){ truthMass_=x; }
  void setTruthEnergy(double x){ truthEnergy_=x; }
  void setTruthPdgId(int x){ truthPdgId_=x; }

  /* 
     Getters
   */
  int getPID() const {return pid_;}
  float getMass() const { return mass_;}
  float getEnergy() const { return energy_;}

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

  float getEcalEnergy() const { return ecalEnergy_;}
  float getEcalRawEnergy() const { return ecalRawEnergy_;}
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

  float getHcalEnergy() const { return hcalEnergy_;}
  float getHcalRawEnergy() const { return hcalRawEnergy_;}
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

  std::vector<double> const getTruthEcalXYZ(){
    return {truthEcalX_, truthEcalY_, truthEcalZ_};
  }
  std::vector<double> const getTruthPxPyPz(){
    return {truthPx_, truthPy_, truthPz_};
  }
  double getTruthMass(){ return truthMass_; }
  double getTruthEnergy(){ return truthEnergy_; }
  int getTruthPdgId(){ return truthPdgId_; }

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
  ClassDef(PFCandidate, 1);
};
}  // namespace ldmx

#endif /* RECON_EVENT_PFCANDIDATE_H_ */
