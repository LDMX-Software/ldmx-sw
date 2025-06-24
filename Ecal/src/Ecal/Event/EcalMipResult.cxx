#include "Ecal/Event/EcalMipResult.h"

ClassImp(ldmx::EcalMipResult);

namespace ldmx {
EcalMipResult::EcalMipResult() {}

EcalMipResult::~EcalMipResult() { Clear(); }

void EcalMipResult::Print() const {
    std::cout << "[ EcalMipResult ]:\n"
              << "\t nStraightTracks : " << nStraightTracks_ << "\n"
              << "\t nLinregTracks : " << nLinregTracks_ << "\n"
              << "\t firstNearPhLayer : " << firstNearPhLayer_ << "\n"
              << "\t nNearPhHits : " << nNearPhHits_ << "\n"
              << "\t photonTerritoryHits : " << photonTerritoryHits_ << "\n"
              << "\t epAng : " << epAng_ << "\n"
              << "\t epAngAtTarget : " << epAngAtTarget_ << "\n"
              << "\t epSep : " << epSep_ << "\n"
              << "\t epDot : " << epDot_ << "\n"
              << "\t epDotAtTarget : " << epDotAtTarget_ << std::endl;
}

void EcalMipResult::Clear() {
    nStraightTracks_ = 0;
    nLinregTracks_ = 0;
    firstNearPhLayer_ = 0;
    nNearPhHits_ = 0;
    photonTerritoryHits_ = 0;
    epAng_ = 0;
    epAngAtTarget_ = 0;
    epSep_ = 0;
    epDot_ = 0;
    epDotAtTarget_ = 0;
}

void EcalMipResult::setVariables(
    int nStraightTracks, int nLinregTracks, int firstNearPhLayer,
    int nNearPhHits, int photonTerritoryHits, float epAng, float epAngAtTarget,
    float epSep, float epDot, float epDotAtTarget) {
    nStraightTracks_ = nStraightTracks;
    nLinregTracks_ = nLinregTracks;
    firstNearPhLayer_ = firstNearPhLayer;
    nNearPhHits_ = nNearPhHits;
    photonTerritoryHits_ = photonTerritoryHits;
    epAng_ = epAng;
    epAngAtTarget_ = epAngAtTarget;
    epSep_ = epSep;
    epDot_ = epDot;
    epDotAtTarget_ = epDotAtTarget;
}


} // namespace ldmx