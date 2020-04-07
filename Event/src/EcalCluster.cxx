#include "Event/EcalCluster.h"

ClassImp(ldmx::EcalCluster)

namespace ldmx {

    EcalCluster::EcalCluster() {

    }

    EcalCluster::~EcalCluster() {
        Clear();
    }

    void EcalCluster::Print(std::ostream& o) const {
        o << "EcalCluster { " << "Energy: " << energy_ << ", " << "Number of hits: " << nHits_ << " }";
    }

    void EcalCluster::Clear() {

        hitIDs_.clear();

        energy_ = 0;
        nHits_ = 0;
        centroidX_ = 0;
        centroidY_ = 0;
        centroidZ_ = 0;

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
