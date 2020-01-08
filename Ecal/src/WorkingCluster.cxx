/*
   WorkingCluster -- In-memory tool for working on clusters
   */

#include "Ecal/WorkingCluster.h"
#include <iostream>

namespace ldmx {

    WorkingCluster::WorkingCluster(const EcalHit* eh, const std::shared_ptr<EcalHexReadout> hex, double zPos) {
        add(eh, hex, zPos);
    }

    void WorkingCluster::add(const EcalHit* eh, const std::shared_ptr<EcalHexReadout> hex, double zPos) {
    
        double hitE = eh->getEnergy();
        unsigned int hitID = eh->getID();

        unsigned int cellID = hitID>>15;
        unsigned int moduleID = (hitID<<17)>>29;
        unsigned int combinedID = 10*cellID + moduleID;

        std::pair<double, double> hitXY = hex->getCellCenterAbsolute(combinedID);
    
        double newE = hitE + centroid_.E();
        double newCentroidX = (centroid_.Px()*centroid_.E() + hitE*hitXY.first) / newE;
        double newCentroidY = (centroid_.Py()*centroid_.E() + hitE*hitXY.second) / newE;
        double newCentroidZ = (centroid_.Pz()*centroid_.E() + hitE*zPos) / newE;

        centroid_.SetPxPyPzE(newCentroidX, newCentroidY, newCentroidZ, newE);

        hits_.push_back(eh); 
    }
    
    void WorkingCluster::add(const WorkingCluster& wc) {
    
        double clusterE = wc.centroid().E();
        double centroidX = wc.centroid().Px();
        double centroidY = wc.centroid().Py();
        double centroidZ = wc.centroid().Pz();
    
        double newE = clusterE + centroid_.E();
        double newCentroidX = (centroid_.Px()*centroid_.E() + clusterE*centroidX) / newE;
        double newCentroidY = (centroid_.Py()*centroid_.E() + clusterE*centroidY) / newE;
        double newCentroidZ = (centroid_.Pz()*centroid_.E() + clusterE*centroidZ) / newE;
    
        centroid_.SetPxPyPzE(newCentroidX, newCentroidY, newCentroidZ, newE);

        std::vector<const EcalHit*> clusterHits = wc.getHits();
    
        for (size_t i = 0; i < clusterHits.size(); i++) {
    
            hits_.push_back(clusterHits[i]);
        }
    }
}
