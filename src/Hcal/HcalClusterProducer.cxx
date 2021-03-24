#include "TString.h"
#include "TFile.h"
#include "TTree.h"

#include "Hcal/HcalClusterProducer.h"
#include "Hcal/WorkingCluster.h"
#include "Hcal/MyClusterWeight.h"
#include "Hcal/TemplatedClusterFinder.h"
#include <iostream>
#include <exception>

namespace hcal {

    HcalClusterProducer::HcalClusterProducer(const std::string& name, framework::Process& process) :
        Producer(name, process){}

    void HcalClusterProducer::configure(framework::config::Parameters& parameters) {
         //EminSeed_ = parameters.getParameter< double >("EminSeed");
         EnoiseCut_ = parameters.getParameter< double >("EnoiseCut");
         deltaTime_ = parameters.getParameter< double >("deltaTime");
         deltaR_ = parameters.getParameter< double >("deltaR");
         EminCluster_ = parameters.getParameter< double >("EminCluster");
         cutOff_ = parameters.getParameter< double >("cutOff");
        
    }


    void HcalClusterProducer::produce(framework::Event& event)
    {
        
        const ldmx::HcalGeometry& hcalGeom = getCondition<ldmx::HcalGeometry>(ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
        
        TemplatedClusterFinder<MyClusterWeight> finder;
        
        std::vector<ldmx::HcalCluster> hcalClusters;
        std::list<const ldmx::HcalHit*> seedList;
        std::vector< ldmx::HcalHit > hcalHits = event.getCollection< ldmx::HcalHit >("HcalRecHits");
        
  
        if (hcalHits.empty()) { return; }

        for (ldmx::HcalHit& hit : hcalHits) {
            if (hit.getEnergy() <  EnoiseCut_) continue;
            if (hit.getEnergy() == 0) continue;
            finder.add(&hit, hcalGeom);
        }
        
  
        //seedList.sort([](const ldmx::HcalHit* a, const ldmx::HcalHit* b) {return a->getEnergy() > b->getEnergy();});

        finder.cluster(EminCluster_,cutOff_,deltaTime_);
        //std::cout<<"[HcalClusterProducer::produce cluster made...]"<<std::endl;
        std::vector<WorkingCluster> wcVec = finder.getClusters();
        //std::cout<<"[HcalClusterProducer::produce ending...]"<<std::endl;
        for (unsigned int c = 0; c < wcVec.size(); c++) {
            if (wcVec[c].empty()) continue;
            ldmx::HcalCluster cluster;
    
            cluster.setEnergy(wcVec[c].centroid().E());
            cluster.setCentroidXYZ(wcVec[c].centroid().Px(), wcVec[c].centroid().Py(), wcVec[c].centroid().Pz());
            cluster.setNHits(wcVec[c].getHits().size());
            cluster.addHits(wcVec[c].getHits());
            cluster.setTime(wcVec[c].GetTime());
            //std::cout<<"[HcalClusterProducer::setting the cluster parameters...]"<<std::endl;
            hcalClusters.push_back( cluster );
        }
        //std::cout<<"[HcalClusterProducer::produce adding to event...]"<<std::endl;
        event.add( "HcalClusters", hcalClusters );
        //std::cout<<"[HcalClusterProducer::produce added...]"<<std::endl;
    }
   
}
DECLARE_PRODUCER_NS(hcal, HcalClusterProducer);

