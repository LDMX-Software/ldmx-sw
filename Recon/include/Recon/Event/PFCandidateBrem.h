/**
 * @file PFCandidate.h
 * @brief Class that represents a reconstructed particle candidate
 * @author Christian Herwig, Fermilab
 */

#ifndef RECON_EVENT_PFCANDIDATEBREM_H_
#define RECON_EVENT_PFCANDIDATEBREM_H_

// ROOT
#include "TObject.h"  //For ClassDef

namespace ldmx {

/**
 * @class PFCandidateBrem
 * @brief Represents a reconstructed particle, including brem products
 */
class PFCandidateBrem {
 public:
  PFCandidateBrem() {}
  virtual ~PFCandidateBrem() {}

  void print() const;
  bool operator<(const PFCandidateBrem &rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

  /*
     Setters
   */
  void setPID(int x) { pid_ = x; }
  void setPDGId(int x) { pdgid_ = x; }
  void setMass(float x) { mass_ = x; }
  void setEnergy(float x) { energy_ = x; }

  void setClusterIdx(int i) { cluster_idx_ = i; }
  void setTrackIdx(int i) { track_idx_ = i; }

  void setTargetTagPositionXYZ(float x, float y, float z) {
    posTargTagX_ = x;
    posTargTagY_ = y;
    posTargTagZ_ = z;
  }
  void setTargetRecPositionXYZ(float x, float y, float z) {
    posTargRecX_ = x;
    posTargRecY_ = y;
    posTargRecZ_ = z;
  }
  void setEcalPositionXYZ(float x, float y, float z) {
    posEcalX_ = x;
    posEcalY_ = y;
    posEcalZ_ = z;
  }
  void setHcalPositionXYZ(float x, float y, float z) {
    posHcalX_ = x;
    posHcalY_ = y;
    posHcalZ_ = z;
  }

  void setTaggerTrackPxPyPz(float x, float y, float z) {
    trackTaggerPx_ = x;
    trackTaggerPy_ = y;
    trackTaggerPz_ = z;
  }

  void setRecoilTrackPxPyPz(float x, float y, float z) {
    trackRecoilPx_ = x;
    trackRecoilPy_ = y;
    trackRecoilPz_ = z;
  }

// void setTargetMomentumXYZ(float x, float y, float z) {
//    momTargX_ = x;
//    momTargY_ = y;
//    momTargZ_ = z;
//  }
  void setEcalMomentumXYZ(float x, float y, float z) {
    momEcalX_ = x;
    momEcalY_ = y;
    momEcalZ_ = z;
  }

  void addBremProduct(int id) {
    bremProductsID_.push_back(id);
    bremProductsEnergy_.push_back(-999);
    bremProductsVertex_.push_back("unknown");
    bremProductsEcalX_.push_back(-999);
    bremProductsEcalY_.push_back(-999);
    bremProductsEcalZ_.push_back(-999);
    bremClusterIdx_.push_back(-999);
    std::vector<unsigned int> hits;
    bremProductsECalHits_.push_back(hits);

  }

  void setBremProductEnergy(int idx, double energy) {bremProductsEnergy_[idx] = energy;}
  void setBremClusterIdx(int idx, int i) {bremClusterIdx_[idx] = i;}
  void setBremProductVertex(int idx, std::string vertex) {bremProductsVertex_[idx] = vertex;}
  void setBremProductECalPositionXYZ(int idx, double x, double y, double z) {
    bremProductsEcalX_[idx] = x;
    bremProductsEcalY_[idx] = y;
    bremProductsEcalZ_[idx] = z;
  }
  void setBremProductECalHits(int idx,  std::vector<unsigned int> hits) {bremProductsECalHits_[idx] = hits;}
  void addBremProductECalHit(int idx,  int hit) {bremProductsECalHits_[idx].push_back(hit);}

  void setEcalEnergy(float x) { ecalEnergy_ = x; }
  void setEcalRawEnergy(float x) { ecalRawEnergy_ = x; }
  void setEcalClusterXYZ(float x, float y, float z) {
    ecalClusterX_ = x;
    ecalClusterY_ = y;
    ecalClusterZ_ = z;
  }
  void setEcalClusterEXYZ(float x, float y, float z) {
    ecalClusterEX_ = x;
    ecalClusterEY_ = y;
    ecalClusterEZ_ = z;
  }
  void setEcalClusterDXDZ(float x) { ecalClusterDXDZ_ = x; }
  void setEcalClusterDYDZ(float x) { ecalClusterDYDZ_ = x; }
  void setEcalClusterEDXDZ(float x) { ecalClusterEDXDZ_ = x; }
  void setEcalClusterEDYDZ(float x) { ecalClusterEDYDZ_ = x; }

  void setECalClusterHits(std::vector<unsigned int> hits) {ecalClusterHits_ = hits;}
  
  void addECalClusterHit(int hit) {ecalClusterHits_.push_back(hit);}


  void setHcalEnergy(float x) { hcalEnergy_ = x; }
  void setHcalRawEnergy(float x) { hcalRawEnergy_ = x; }
  void setHcalClusterXYZ(float x, float y, float z) {
    hcalClusterX_ = x;
    hcalClusterY_ = y;
    hcalClusterZ_ = z;
  }
  void setHcalClusterEXYZ(float x, float y, float z) {
    hcalClusterEX_ = x;
    hcalClusterEY_ = y;
    hcalClusterEZ_ = z;
  }
  void setHcalClusterDXDZ(float x) { hcalClusterDXDZ_ = x; }
  void setHcalClusterDYDZ(float x) { hcalClusterDYDZ_ = x; }
  void setHcalClusterEDXDZ(float x) { hcalClusterEDXDZ_ = x; }
  void setHcalClusterEDYDZ(float x) { hcalClusterEDYDZ_ = x; }

  /*
  void setTruthEcalXYZ(double x, double y, double z) {
    truthEcalX_ = x;
    truthEcalY_ = y;
    truthEcalZ_ = z;
  }
  void setTruthPxPyPz(double x, double y, double z) {
    truthPx_ = x;
    truthPy_ = y;
    truthPz_ = z;
  }
  void setTruthMass(double x) { truthMass_ = x; }
  void setTruthEnergy(double x) { truthEnergy_ = x; }
  void setTruthPdgId(int x) { truthPdgId_ = x; }

  /*

  /*
     Getters
   */
  int getPID() const { return pid_; }
  float getMass() const { return mass_; }
  float getEnergy() const { return energy_; }
  
  int getClusterIdx() const { return cluster_idx_; }
  int getTrackIdx() const { return track_idx_; }

  std::vector<float> getTargetTagPositionXYZ() const {
    return {posTargTagX_, posTargTagY_, posTargTagZ_};
  }
  std::vector<float> getTargetRecPositionXYZ() const {
    return {posTargRecX_, posTargRecY_, posTargRecZ_};
  }
  std::vector<float> getEcalPositionXYZ() const {
    return {posEcalX_, posEcalY_, posEcalZ_};
  }
  std::vector<float> getHcalPositionXYZ() const {
    return {posHcalX_, posHcalY_, posHcalZ_};
  }
  std::vector<float> getTaggerTrackPxPyPz() const {
    return {trackTaggerPx_, trackTaggerPy_, trackTaggerPz_};
  }
  std::vector<float> getRecoilTrackPxPyPz() const {
    return {trackRecoilPx_, trackRecoilPy_, trackRecoilPz_};
  }

  std::vector<int> getBremProductIDs() const {
    return bremProductsID_;
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

  /*
  std::vector<double> const getTruthEcalXYZ() {
    return {truthEcalX_, truthEcalY_, truthEcalZ_};
  }
  std::vector<double> const getTruthPxPyPz() {
    return {truthPx_, truthPy_, truthPz_};
  }
  double getTruthMass() { return truthMass_; }
  double getTruthEnergy() { return truthEnergy_; }
  int getTruthPdgId() { return truthPdgId_; }

  */

 private:
  /* Particle ID enum */
  int pid_{-999};
  int pdgid_{-999};
  float mass_{-999};
  float energy_{-999};
  int cluster_idx_{-999};
  int track_idx_{-999};

  /* Position Before Target */
  float posTargTagX_{-999};
  float posTargTagY_{-999};
  float posTargTagZ_{-999};
   /* Position After Target */
  float posTargRecX_{-999};
  float posTargRecY_{-999};
  float posTargRecZ_{-999};
  /* Position at the ECal face */
  float posEcalX_{-999};
  float posEcalY_{-999};
  float posEcalZ_{-999};
  /* Position at the HCal face */
  float posHcalX_{-999};
  float posHcalY_{-999};
  float posHcalZ_{-999};

  /* tagger track momenta at target */
  float trackTaggerPx_{-999};
  float trackTaggerPy_{-999};
  float trackTaggerPz_{-999};

  /* reccoil track momenta at target */
  float trackRecoilPx_{-999};
  float trackRecoilPy_{-999};
  float trackRecoilPz_{-999};

  /* Momentum at the Target */
  //float momTargX_{0};
  //float momTargY_{0};
  //float momTargZ_{0};
  /* Momentum at the ECal face */
  float momEcalX_{-999};
  float momEcalY_{-999};
  float momEcalZ_{-999}; 

  /* Brem products */
  std::vector<int> bremProductsID_;
  std::vector<int> bremClusterIdx_;
  std::vector<double> bremProductsEnergy_;
  std::vector<std::string> bremProductsVertex_;
  std::vector<double> bremProductsEcalX_;
  std::vector<double> bremProductsEcalY_;
  std::vector<double> bremProductsEcalZ_;
  std::vector<std::vector<unsigned int>> bremProductsECalHits_;

  /* Ecal energy, cluster info */
  float ecalEnergy_{-999};
  float ecalRawEnergy_{-999};
  float ecalClusterX_{-999};
  float ecalClusterY_{-999};
  float ecalClusterZ_{-999};
  float ecalClusterEX_{-999};
  float ecalClusterEY_{-999};
  float ecalClusterEZ_{-999};
  float ecalClusterDXDZ_{-999};
  float ecalClusterDYDZ_{-999};
  float ecalClusterEDXDZ_{-999};
  float ecalClusterEDYDZ_{-999};
  std::vector<unsigned int> ecalClusterHits_;

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
  //double truthEcalX_{0};
  //double truthEcalY_{0};
  //double truthEcalZ_{0};
  //double truthPx_{0};
  //double truthPy_{0};
  //double truthPz_{0};
  //double truthMass_{0};
  //double truthEnergy_{0};
  //int truthPdgId_{0};

  /* The ROOT class definition. */
  ClassDef(PFCandidateBrem, 1);
};
}  // namespace ldmx

#endif /* RECON_EVENT_PFCANDIDATEBREM_H_ */
