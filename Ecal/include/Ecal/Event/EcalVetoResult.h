/**
 * @file EcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from
 *        EcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_ECALVETORESULT_H_
#define EVENT_ECALVETORESULT_H_

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

class EcalVetoResult {
 public:
  /** Constructor */
  EcalVetoResult();

  /** Destructor */
  ~EcalVetoResult();

  /**
   * Set the sim particle and 'is findable' flag.
   */
  void setVariables(int nReadoutHits, int deepestLayerHit, float summedDet,
                    float summedTightIso, float maxCellDep, float showerRMS,
                    float xStd, float yStd, float avgLayerHit,
                    float stdLayerHit, float ecalBackEnergy,
                    int nStraightTracks, int nLinregTracks,
                    int firstNearPhLayer, int nNearPhHits,
                    int photonTerritoryHits, float epAng, float epSep,
                    float epDot,

                    std::vector<float> electronContainmentEnergy,
                    std::vector<float> photonContainmentEnergy,
                    std::vector<float> outsideContainmentEnergy,
                    std::vector<int> outsideContainmentNHits,
                    std::vector<float> outsideContainmentXStd,
                    std::vector<float> outsideContainmentYStd,

                    std::vector<float> energySeg, std::vector<float> xMeanSeg,
                    std::vector<float> yMeanSeg, std::vector<float> xStdSeg,
                    std::vector<float> yStdSeg, std::vector<float> layerMeanSeg,
                    std::vector<float> layerStdSeg,

                    std::vector<std::vector<float>> eContEnergy,
                    std::vector<std::vector<float>> eContXMean,
                    std::vector<std::vector<float>> eContYMean,
                    std::vector<std::vector<float>> gContEnergy,
                    std::vector<std::vector<int>> gContNHits,
                    std::vector<std::vector<float>> gContXMean,
                    std::vector<std::vector<float>> gContYMean,
                    std::vector<std::vector<float>> oContEnergy,
                    std::vector<std::vector<int>> oContNHits,
                    std::vector<std::vector<float>> oContXMean,
                    std::vector<std::vector<float>> oContYMean,
                    std::vector<std::vector<float>> oContXStd,
                    std::vector<std::vector<float>> oContYStd,
                    std::vector<std::vector<float>> oContLayerMean,
                    std::vector<std::vector<float>> oContLayerStd,

                    std::vector<float> EcalLayerEdepReadout,
                    std::vector<double> recoilP, std::vector<float> recoilPos);

  /** Reset the object. */
  void Clear();

  /** Print the object */
  void Print() const;

  /** Checks if the event passes the Ecal veto. */
  bool passesVeto() const { return passesVeto_; }
  float getDisc() const { return discValue_; }

  int getDeepestLayerHit() const { return deepestLayerHit_; }

  int getNReadoutHits() const { return nReadoutHits_; }

  float getSummedDet() const { return summedDet_; }

  float getSummedTightIso() const { return summedTightIso_; }

  float getMaxCellDep() const { return maxCellDep_; }

  float getShowerRMS() const { return showerRMS_; }

  float getXStd() const { return xStd_; }

  float getYStd() const { return yStd_; }

  float getAvgLayerHit() const { return avgLayerHit_; }

  float getStdLayerHit() const { return stdLayerHit_; }

  float getEcalBackEnergy() const { return ecalBackEnergy_; }

  const std::vector<float>& getElectronContainmentEnergy() const {
    return electronContainmentEnergy_;
  }

  const std::vector<float>& getPhotonContainmentEnergy() const {
    return photonContainmentEnergy_;
  }

  const std::vector<float>& getOutsideContainmentEnergy() const {
    return outsideContainmentEnergy_;
  }

  const std::vector<int>& getOutsideContainmentNHits() const {
    return outsideContainmentNHits_;
  }

  const std::vector<float>& getOutsideContainmentXStd() const {
    return outsideContainmentXStd_;
  }

  const std::vector<float>& getOutsideContainmentYStd() const {
    return outsideContainmentYStd_;
  }

  const std::vector<float>& getEcalLayerEdepReadout() const {
    return ecalLayerEdepReadout_;
  }

  const std::vector<float>& getEnergySeg() const { return energySeg_; }

  const std::vector<float>& getXMeanSeg() const { return xMeanSeg_; }

  const std::vector<float>& getYMeanSeg() const { return yMeanSeg_; }

  const std::vector<float>& getXStdSeg() const { return xStdSeg_; }

  const std::vector<float>& getYStdSeg() const { return yStdSeg_; }

  const std::vector<float>& getLayerMeanSeg() const { return layerMeanSeg_; }

  const std::vector<float>& getLayerStdSeg() const { return layerStdSeg_; }

  const std::vector<std::vector<float>>& getEleContEnergy() const {
    return eContEnergy_;
  }

  const std::vector<std::vector<float>>& getEleContXMean() const {
    return eContXMean_;
  }

  const std::vector<std::vector<float>>& getEleContYMean() const {
    return eContYMean_;
  }

  const std::vector<std::vector<float>>& getPhContEnergy() const {
    return gContEnergy_;
  }

  const std::vector<std::vector<int>>& getPhContNHits() const {
    return gContNHits_;
  }

  const std::vector<std::vector<float>>& getPhContXMean() const {
    return gContXMean_;
  }

  const std::vector<std::vector<float>>& getPhContYMean() const {
    return gContYMean_;
  }

  const std::vector<std::vector<float>>& getOutContEnergy() const {
    return oContEnergy_;
  }

  const std::vector<std::vector<int>>& getOutContNHits() const {
    return oContNHits_;
  }

  const std::vector<std::vector<float>>& getOutContXMean() const {
    return oContXMean_;
  }

  const std::vector<std::vector<float>>& getOutContYMean() const {
    return oContYMean_;
  }

  const std::vector<std::vector<float>>& getOutContXStd() const {
    return oContXStd_;
  }

  const std::vector<std::vector<float>>& getOutContYStd() const {
    return oContYStd_;
  }

  const std::vector<std::vector<float>>& getOutContLayerMean() const {
    return oContLayerMean_;
  }

  const std::vector<std::vector<float>>& getOutContLayerStd() const {
    return oContLayerStd_;
  }

  void setVetoResult(bool passesVeto) { passesVeto_ = passesVeto; }
  void setDiscValue(float discValue) { discValue_ = discValue; }

  /** Return the momentum of the recoil at the Ecal face. */
  const std::vector<double> getRecoilMomentum() const {
    return {recoilPx_, recoilPy_, recoilPz_};
  };

  /** Return the x position of the recoil at the Ecal face. */
  const double getRecoilX() const { return recoilX_; };

  /** Return the y position of the recoil at the Ecal face. */
  const double getRecoilY() const { return recoilY_; };

  /// Number of straight tracks found
  const int getNStraightTracks() const { return nStraightTracks_; }

  /// Number of linear-regression tracks found
  const int getNLinRegTracks() const { return nLinregTracks_; }

  const int getFirstNearPhLayer() const { return firstNearPhLayer_; }
  const int getNNearPhHits() const { return nNearPhHits_; }
  const int getPhotonTerritoryHits() const { return photonTerritoryHits_; }
  const float getEPAng() const { return epAng_; }
  const float getEPSep() const { return epSep_; }
  const float getEPDot() const { return epDot_; }

 private:
  /** Flag indicating whether the event is vetoed by the Ecal. */
  bool passesVeto_{false};

  int nReadoutHits_{0};
  int deepestLayerHit_{0};

  float summedDet_{0};
  float summedTightIso_{0};
  float maxCellDep_{0};
  float showerRMS_{0};
  float xStd_{0};
  float yStd_{0};
  float avgLayerHit_{0};
  float stdLayerHit_{0};
  float ecalBackEnergy_{0};
  // MIP tracking
  /// Number of "straight" tracks found in the event
  int nStraightTracks_{0};
  /// Number of "linreg" tracks found in the event
  int nLinregTracks_{0};
  /// Earliest ECal layer in which a hit is found near the projected photon
  /// trajectory
  int firstNearPhLayer_{0};
  /// Number of hits near the photon trajectory
  int nNearPhHits_{0};
  /// Number of hits in the photon territory
  int photonTerritoryHits_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// (currently unused)
  float epAng_{0};
  /// Distance between the projected photon and electron trajectories at the
  /// ECal face
  float epSep_{0};
  /// Dot product of the photon and electron momenta unit vectors
  float epDot_{0};

  std::vector<float> electronContainmentEnergy_;
  std::vector<float> photonContainmentEnergy_;
  std::vector<float> outsideContainmentEnergy_;
  std::vector<int> outsideContainmentNHits_;
  std::vector<float> outsideContainmentXStd_;
  std::vector<float> outsideContainmentYStd_;

  std::vector<float> energySeg_;
  std::vector<float> xMeanSeg_;
  std::vector<float> yMeanSeg_;
  std::vector<float> xStdSeg_;
  std::vector<float> yStdSeg_;
  std::vector<float> layerMeanSeg_;
  std::vector<float> layerStdSeg_;

  std::vector<std::vector<float>> eContEnergy_;
  std::vector<std::vector<float>> eContXMean_;
  std::vector<std::vector<float>> eContYMean_;
  std::vector<std::vector<float>> gContEnergy_;
  std::vector<std::vector<int>> gContNHits_;
  std::vector<std::vector<float>> gContXMean_;
  std::vector<std::vector<float>> gContYMean_;
  std::vector<std::vector<float>> oContEnergy_;
  std::vector<std::vector<int>> oContNHits_;
  std::vector<std::vector<float>> oContXMean_;
  std::vector<std::vector<float>> oContYMean_;
  std::vector<std::vector<float>> oContXStd_;
  std::vector<std::vector<float>> oContYStd_;
  std::vector<std::vector<float>> oContLayerMean_;
  std::vector<std::vector<float>> oContLayerStd_;

  float discValue_{0};

  /** px of recoil electron at the Ecal face. */
  double recoilPx_{-9999};

  /** py of recoil electron at the Ecal face. */
  double recoilPy_{-9999};

  /** py of recoil electron at the Ecal face. */
  double recoilPz_{-9999};

  /** x position of recoil electron at the Ecal face. */
  float recoilX_{-9999};

  /** y position of recoil electron at the Ecal face. */
  float recoilY_{-9999};

  std::vector<float> ecalLayerEdepReadout_;

  ClassDef(EcalVetoResult, 6);
};
}  // namespace ldmx

#endif
