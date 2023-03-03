#ifndef TRIGGER_EVENT_TRIGPARTICLE_H
#define TRIGGER_EVENT_TRIGPARTICLE_H

// ROOT
#include "TObject.h"  //For ClassDef
// #include "TLorentzVector.h"
#include "Math/GenVector/LorentzVector.h"
#include "Math/GenVector/PositionVector3D.h"

namespace trigger {

// Forward declaration needed by typedef
class TrigParticle;
typedef std::vector<TrigParticle> TrigParticleCollection;

// move to a central location?
typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > XYZTLorentzVector;
typedef XYZTLorentzVector LorentzVector;
typedef ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<double> > Point;

/**
 * @class TrigParticle
 * @brief Class for particles reconstructed by the trigger system
 */
class TrigParticle {
 public:

  TrigParticle() = default;
  TrigParticle(LorentzVector p4, Point vtx);
  TrigParticle(LorentzVector p4, Point vtx, int pdgId);

  virtual ~TrigParticle() = default;

  // momenta accessors
  double p() const { return p4_.P(); }
  double energy() const { return p4_.E(); }
  double mass() const { return p4_.mass(); }
  double px() const { return p4_.Px(); }
  double py() const { return p4_.Py(); }
  double pz() const { return p4_.Pz(); }
  double pt() const { return p4_.pt(); }
  double phi() const { return p4_.phi(); }
  double theta() const { return p4_.Theta(); }

  // vertex accessors
  const Point& vertex() const { return vtx_; }
  double vx() const { return vtx_.X(); }
  double vy() const { return vtx_.Y(); }
  double vz() const { return vtx_.Z(); }

  // setters
  void setP4(const LorentzVector& p4) {
      p4_ = p4;
  }
  void setVertex(const Point& v) {
      vtx_ = v;
  }

  // set HW values
  void setHwPt(int pt) { hwPt_ = pt; }
  void setHwEta(int eta) { hwEta_ = eta; }
  void setHwPhi(int phi) { hwPhi_ = phi; }
  void setHwQual(int qual) { hwQual_ = qual; }
  void setHwIso(int iso) { hwIso_ = iso; }

  // retrieve HW values
  int hwPt() const { return hwPt_; }
  int hwEta() const { return hwEta_; }
  int hwPhi() const { return hwPhi_; }
  int hwQual() const { return hwQual_; }
  int hwIso() const { return hwIso_; }
  
 private:

  Point vtx_;
  XYZTLorentzVector p4_;
  int pdgId_;

  int hwPt_;
  int hwEta_;
  int hwPhi_;
  int hwQual_;
  int hwIso_;
  
  /// ROOT Dictionary class definition macro
  ClassDef(TrigParticle, 1);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGPARTICLE_H
