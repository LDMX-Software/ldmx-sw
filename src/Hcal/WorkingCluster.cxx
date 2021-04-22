
#include "Hcal/WorkingCluster.h"
#include <iostream>

namespace hcal {

    WorkingCluster::WorkingCluster(const ldmx::HcalHit* eh, const ldmx::HcalGeometry& hex) {
       
        add(eh, hex);
    }

    void WorkingCluster::add(const ldmx::HcalHit* eh, const ldmx::HcalGeometry& hex) {
        
        double hitE = eh->getEnergy();
        TVector3 hitpos = hex.getStripCenterPosition( eh->getID() );
        double hitX = hitpos.x();
        double hitY = hitpos.y();
        double hitZ = hitpos.z();
        double hitT = eh->getTime();
        // Based on weight for  Center-of-Gravity by hitpos*hiE/totalE
        double newE = hitE + centroid_.E();
        double newCentroidX = (centroid_.Px()*centroid_.E() + hitE*hitX) / newE;
        double newCentroidY = (centroid_.Py()*centroid_.E() + hitE*hitY) / newE;
        double newCentroidZ = (centroid_.Pz()*centroid_.E() + hitE*hitZ) / newE;
        
        if(time_ < hitT){
            time_ = hitT;
        }
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
        /*if(wc.GetTime() > time_){
            time_ = wc.GetTime();
        }*/
        
        std::vector<const ldmx::HcalHit*> clusterHits = wc.getHits();
    
        for (unsigned int i = 0; i < clusterHits.size(); i++) {  
            hits_.push_back(clusterHits[i]);
        }
    }
}
