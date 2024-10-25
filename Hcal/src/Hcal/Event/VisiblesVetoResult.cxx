#include "Hcal/Event/VisiblesVetoResult.h"

ClassImp(ldmx::VisiblesVetoResult);

namespace ldmx {
  VisiblesVetoResult::VisiblesVetoResult() {}

  VisiblesVetoResult::~VisiblesVetoResult() { Clear(); }

  void VisiblesVetoResult::Clear() {
    passesVeto_ = false;

    nLayersHit_ = 0;
    xStd_ = 0.;
    yStd_ = 0.;
    zStd_ = 0.;
    xMean_ = 0.;
    yMean_ = 0.;
    rMean_ = 0.;
    isoHits_ = 0;
    isoEnergy_ = 0.;
    nReadoutHits_ = 0;
    summedDet_ = 0.;
    rMeanFromPhotonProj_ = 0.;

    discValue_ = 0.;
  }

  void VisiblesVetoResult::setVariables(
					int nLayersHit,
					double xStd, double yStd, double zStd,
					double xMean, double yMean, double rMean,
					int isoHits, double isoEnergy,
					int nReadoutHits, double summedDet,
					double rMeanFromPhotonProj) {
    nLayersHit_ = nLayersHit;
    xStd_ = xStd;
    yStd_ = yStd;
    zStd_ = zStd;
    xMean_ = xMean;
    yMean_ = yMean;
    rMean_ = rMean;
    isoHits_ = isoHits;
    isoEnergy_ = isoEnergy;
    nReadoutHits_ = nReadoutHits;
    summedDet_ = summedDet;
    rMeanFromPhotonProj_ = rMeanFromPhotonProj;
  }

  void VisiblesVetoResult::Print() const {
    std::cout << "[ VisiblesVetoResult ]:\n"
	      << "\t Passes veto : " << passesVeto_ << "\n"
	      << std::endl;
  }
} // namespace ldmx
