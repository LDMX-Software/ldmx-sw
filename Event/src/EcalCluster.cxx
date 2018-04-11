#include "Event/EcalCluster.h"

ClassImp(ldmx::EcalCluster)

namespace ldmx {

    EcalCluster::EcalCluster() : TObject() {

    }

    EcalCluster::~EcalCluster() {
        Clear();
    }

    void EcalCluster::Print(Option_t *option) const {

        std::cout << "EcalCluster { " << "Energy: " << energy_ << ", " << "Number of hits: " << nHits_ << " }" << std::endl;

        //for (int iHit = 0; iHit < hits_->GetEntries(); ++iHit) {
        //    EcalHit* aHit = (EcalHit*) hits_->At(iHit);
        //    std::cout << "Hit " << iHit << " : " << "with energy " << aHit->getEnergy() << std::endl;
        //}
    }

    void EcalCluster::Clear(Option_t*) {

        TObject::Clear();
        hitIDs_.clear();

        energy_ = 0;
        nHits_ = 0;
        centroidX_ = 0;
        centroidY_ = 0;
        centroidZ_ = 0;

    }

    void EcalCluster::Copy(TObject& ob) const {

        EcalCluster& tr = (EcalCluster&) (ob);
        tr.energy_ = energy_;
        tr.nHits_ = nHits_;
        tr.hitIDs_ = hitIDs_;
        tr.centroidX_ = centroidX_;
        tr.centroidY_ = centroidY_;
        tr.centroidZ_ = centroidZ_;
    }

    void EcalCluster::addHits(const std::vector<const EcalHit*> hitsVec) {

        //double clusterE = 0;
        //int nHits = 0;
        //double centroidX = 0;
        //double centroidY = 0;
        std::vector<unsigned int> vecIDs;
        for (int iHit = 0; iHit < hitsVec.size(); iHit++) {
            vecIDs.push_back(hitsVec[iHit]->getID());
        }

        //setEnergy(clusterE);
        //setNHits(nHits);
        setIDs(vecIDs);
        //setCentroidXYZ(centroidX, centroidY, 0);
    }
}
