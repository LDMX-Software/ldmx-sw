#include "Ecal/Event/EcalCluster.h"

ClassImp(ldmx::EcalCluster)

    namespace ldmx {
  EcalCluster::EcalCluster() {}

  EcalCluster::~EcalCluster() { Clear(); }

  void EcalCluster::Print() const {
    std::cout << "EcalCluster { "
              << "Energy: " << energy_ << ", "
              << "Number of hits: " << nHits_ << " }" << std::endl;

    // for (int iHit = 0; iHit < hits_->GetEntries(); ++iHit) {
    //    ldmx::EcalHit* aHit = (ldmx::EcalHit*) hits_->At(iHit);
    //    std::cout << "Hit " << iHit << " : " << "with energy " <<
    //    aHit->getEnergy() << std::endl;
    //}
  }

  void EcalCluster::Clear() {
    hitIDs_.clear();

    energy_ = 0;
    nHits_ = 0;
    centroidX_ = 0;
    centroidY_ = 0;
    centroidZ_ = 0;
    rmsX_ = 0;
    rmsY_ = 0;
    rmsZ_ = 0;
    DXDZ_ = 0;
    DYDZ_ = 0;
    errDXDZ_ = 0;
    errDYDZ_ = 0;
  }

  void EcalCluster::addHits(const std::vector<const EcalHit *> hitsVec) {
    // double clusterE = 0;
    // int nHits = 0;
    // double centroidX = 0;
    // double centroidY = 0;
    std::vector<unsigned int> vecIDs;
    for (int iHit = 0; iHit < hitsVec.size(); iHit++) {
      vecIDs.push_back(hitsVec[iHit]->getID());
    }

    // setEnergy(clusterE);
    // setNHits(nHits);
    setIDs(vecIDs);
    // setCentroidXYZ(centroidX, centroidY, 0);
  }
}  // namespace ldmx
