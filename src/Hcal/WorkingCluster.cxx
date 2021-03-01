
#include "Hcal/WorkingCluster.h"
#include <iostream>

namespace hcal {

    WorkingCluster::WorkingCluster(const ldmx::HcalHit* eh, const ldmx::HcalGeometry& hex) {
       
        add(eh, hex);
    }

    void WorkingCluster::add(const ldmx::HcalHit* eh, const ldmx::HcalGeometry& hex) {
        
        double hitE = eh->getEnergy();

        double hitX, hitY, hitZ;
        std::cout<<"[WorkingCluster::hit poisition...]"<<eh->getXPos()<<" "<<eh->getYPos()<<" "<<eh->getZPos()<<std::endl;
        //hex.getStripAbsolutePosition( eh->getID() , hitX , hitY , hitZ );
        //std::cout<<"[WorkingCluster::strip poisition...]"<<hitX<<" "<<hitY<<" "<<hitZ<<std::endl;
        hitX = eh->getXPos();
        hitY = eh->getYPos();
        hitZ = eh->getZPos();
        double newE = hitE + centroid_.E();
        double newCentroidX = (centroid_.Px()*centroid_.E() + hitE*hitX) / newE;
        double newCentroidY = (centroid_.Py()*centroid_.E() + hitE*hitY) / newE;
        double newCentroidZ = (centroid_.Pz()*centroid_.E() + hitE*hitZ) / newE;
        std::cout<<"has new Center : "<<newCentroidX<<" "<<newCentroidY<<" "<<newCentroidZ<<std::endl;
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

        std::vector<const ldmx::HcalHit*> clusterHits = wc.getHits();
    
        for (size_t i = 0; i < clusterHits.size(); i++) {
    
            hits_.push_back(clusterHits[i]);
        }
    }
}
