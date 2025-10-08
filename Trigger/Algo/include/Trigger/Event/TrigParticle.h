#ifndef TRIGGER_EVENT_TRIGPARTICLE_H
#define TRIGGER_EVENT_TRIGPARTICLE_H

// ROOT
#include "Math/GenVector/LorentzVector.h"
#include "Math/GenVector/PositionVector3D.h"
#include "TObject.h"  //For ClassDef

namespace trigger {

// Forward declaration needed by typedef
class TrigParticle;
typedef std::vector<TrigParticle> TrigParticleCollection;

// move to a central location?
typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >
    XYZTLorentzVector;
typedef XYZTLorentzVector LorentzVector;
typedef ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<double> > Point;

/**
 * @class TrigParticle
 * @brief Class for particles reconstructed by the trigger system
 */
class TrigParticle {
 public:
  TrigParticle() = default;
  TrigParticle(LorentzVector p4);
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
  const Point& endPoint() const { return end_; }
  double endx() const { return end_.X(); }
  double endy() const { return end_.Y(); }
  double endz() const { return end_.Z(); }

  // cluster access
  float getClusEnergy() const { return em_clus_e_; }
  int getClusTP() const { return em_clus_ntp_; }
  int getClusDepth() const { return em_clus_depth_; }
  /* const TrigCaloCluster& getCluster() const {return clus_;} */

  // setters
  void setP4(const LorentzVector& p4) { p4_ = p4; }
  void setVertex(const Point& v) { vtx_ = v; }
  void setEndPoint(const Point& v) { end_ = v; }
  void setClusEnergy(const float n) { em_clus_e_ = n; }
  void setClusTP(const int n) { em_clus_ntp_ = n; }
  void setClusDepth(const int n) { em_clus_depth_ = n; }

  /* void setCluster(const TrigCaloCluster& c) { */
  /*     clus_ = c; */
  /* } */

  // set HW values
  void setHwPt(int pt) { hw_pt_ = pt; }
  void setHwEta(int eta) { hw_eta_ = eta; }
  void setHwPhi(int phi) { hw_phi_ = phi; }
  void setHwQual(int qual) { hw_qual_ = qual; }
  void setHwIso(int iso) { hw_iso_ = iso; }

  // retrieve HW values
  int hwPt() const { return hw_pt_; }
  int hwEta() const { return hw_eta_; }
  int hwPhi() const { return hw_phi_; }
  int hwQual() const { return hw_qual_; }
  int hwIso() const { return hw_iso_; }

 private:
  XYZTLorentzVector p4_{};
  /* TrigCaloCluster clus_; */

  Point vtx_{};
  Point end_{};
  int pdg_id_{0};

  int hw_pt_{0};
  int hw_eta_{0};
  int hw_phi_{0};
  int hw_qual_{0};
  int hw_iso_{0};

  // cluster attributes
  float em_clus_e_{0};
  int em_clus_ntp_{0};
  int em_clus_depth_{0};

  /// ROOT Dictionary class definition macro
  ClassDef(TrigParticle, 2);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGPARTICLE_H
