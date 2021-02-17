
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
    epAng_ = 0;
    epSep_ = 0;

    electronContainmentEnergy_.clear();
    photonContainmentEnergy_.clear();
    outsideContainmentEnergy_.clear();
    outsideContainmentNHits_.clear();
    outsideContainmentXStd_.clear();
    outsideContainmentYStd_.clear();

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
      float epAng, float epSep,

      std::vector<float> electronContainmentEnergy,
      std::vector<float> photonContainmentEnergy,
      std::vector<float> outsideContainmentEnergy,
      std::vector<int> outsideContainmentNHits,
      std::vector<float> outsideContainmentXStd,
      std::vector<float> outsideContainmentYStd,

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
    epAng_ = epAng;
    epSep_ = epSep;

    electronContainmentEnergy_ = electronContainmentEnergy;
    photonContainmentEnergy_ = photonContainmentEnergy;
    outsideContainmentEnergy_ = outsideContainmentEnergy;
    outsideContainmentNHits_ = outsideContainmentNHits;
    outsideContainmentXStd_ = outsideContainmentXStd;
    outsideContainmentYStd_ = outsideContainmentYStd;

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
