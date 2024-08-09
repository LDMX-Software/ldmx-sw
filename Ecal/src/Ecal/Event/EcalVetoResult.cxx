
#include "Ecal/Event/EcalVetoResult.h"

ClassImp(ldmx::EcalVetoResult)

    namespace ldmx {
  EcalVetoResult::EcalVetoResult() {}

  EcalVetoResult::~EcalVetoResult() { Clear(); }

  void EcalVetoResult::Clear() {
    passesVeto_ = false;

    nReadoutHits_ = 0;
    summedDet_ = 0;
    summedTightIso_ = 0;
    maxCellDep_ = 0;
    showerRMS_ = 0;
    xStd_ = 0;
    yStd_ = 0;
    avgLayerHit_ = 0;
    stdLayerHit_ = 0;
    deepestLayerHit_ = 0;
    ecalBackEnergy_ = 0;
    // MIP tracking
    nStraightTracks_ = 0;
    nLinregTracks_ = 0;
    firstNearPhLayer_ = 0;
    nNearPhHits_ = 0;
    photonTerritoryHits_ = 0;
    epAng_ = 0;
    epSep_ = 0;
    epDot_ = 0;

    electronContainmentEnergy_.clear();
    photonContainmentEnergy_.clear();
    outsideContainmentEnergy_.clear();
    outsideContainmentNHits_.clear();
    outsideContainmentXStd_.clear();
    outsideContainmentYStd_.clear();

    energySeg_.clear();
    xMeanSeg_.clear();
    yMeanSeg_.clear();
    xStdSeg_.clear();
    yStdSeg_.clear();
    layerMeanSeg_.clear();
    layerStdSeg_.clear();

    eContEnergy_.clear();
    eContXMean_.clear();
    eContYMean_.clear();
    gContEnergy_.clear();
    gContNHits_.clear();
    gContXMean_.clear();
    gContYMean_.clear();
    oContEnergy_.clear();
    oContNHits_.clear();
    oContXMean_.clear();
    oContYMean_.clear();
    oContXStd_.clear();
    oContYStd_.clear();
    oContLayerMean_.clear();
    oContLayerStd_.clear();

    discValue_ = 0;

    recoilPx_ = -9999;
    recoilPy_ = -9999;
    recoilPz_ = -9999;
    recoilX_ = -9999;
    recoilY_ = -9999;

    ecalLayerEdepReadout_.clear();
  }

  void EcalVetoResult::setVariables(
      int nReadoutHits, int deepestLayerHit, float summedDet,
      float summedTightIso, float maxCellDep, float showerRMS, float xStd,
      float yStd, float avgLayerHit, float stdLayerHit, float ecalBackEnergy,
      int nStraightTracks, int nLinregTracks, int firstNearPhLayer,
      int nNearPhHits, int photonTerritoryHits, float epAng, float epSep,
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

      std::vector<float> EcalLayerEdepReadout, std::vector<double> recoilP,
      std::vector<float> recoilPos) {
    nReadoutHits_ = nReadoutHits;
    summedDet_ = summedDet;
    summedTightIso_ = summedTightIso;
    maxCellDep_ = maxCellDep;
    showerRMS_ = showerRMS;
    xStd_ = xStd;
    yStd_ = yStd;
    avgLayerHit_ = avgLayerHit;
    stdLayerHit_ = stdLayerHit;
    deepestLayerHit_ = deepestLayerHit;
    ecalBackEnergy_ = ecalBackEnergy;
    // MIP tracking
    nStraightTracks_ = nStraightTracks;
    nLinregTracks_ = nLinregTracks;
    firstNearPhLayer_ = firstNearPhLayer;
    nNearPhHits_ = nNearPhHits;
    photonTerritoryHits_ = photonTerritoryHits;
    epAng_ = epAng;
    epSep_ = epSep;
    epDot_ = epDot;

    electronContainmentEnergy_ = electronContainmentEnergy;
    photonContainmentEnergy_ = photonContainmentEnergy;
    outsideContainmentEnergy_ = outsideContainmentEnergy;
    outsideContainmentNHits_ = outsideContainmentNHits;
    outsideContainmentXStd_ = outsideContainmentXStd;
    outsideContainmentYStd_ = outsideContainmentYStd;

    energySeg_ = energySeg;
    xMeanSeg_ = xMeanSeg;
    yMeanSeg_ = yMeanSeg;
    xStdSeg_ = xStdSeg;
    yStdSeg_ = yStdSeg;
    layerMeanSeg_ = layerMeanSeg;
    layerStdSeg_ = layerStdSeg;

    eContEnergy_ = eContEnergy;
    eContXMean_ = eContXMean;
    eContYMean_ = eContYMean;
    gContEnergy_ = gContEnergy;
    gContNHits_ = gContNHits;
    gContXMean_ = gContXMean;
    gContYMean_ = gContYMean;
    oContEnergy_ = oContEnergy;
    oContNHits_ = oContNHits;
    oContXMean_ = oContXMean;
    oContYMean_ = oContYMean;
    oContXStd_ = oContXStd;
    oContYStd_ = oContYStd;
    oContLayerMean_ = oContLayerMean;
    oContLayerStd_ = oContLayerStd;

    // discvalue not set here

    if (!recoilP.empty()) {
      recoilPx_ = recoilP[0];
      recoilPy_ = recoilP[1];
      recoilPz_ = recoilP[2];
      recoilX_ = recoilPos[0];
      recoilY_ = recoilPos[1];
    }

    ecalLayerEdepReadout_ = EcalLayerEdepReadout;
  }

  void EcalVetoResult::Print() const {
    std::cout << "[ EcalVetoResult ]:\n"
              << "\t Passes veto : " << passesVeto_ << "\n"
              << std::endl;
  }
}  // namespace ldmx
