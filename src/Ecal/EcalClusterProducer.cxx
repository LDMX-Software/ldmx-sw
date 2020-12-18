/**
 * @file EcalClusterProducer.cxx
 * @brief Producer that performs clustering in the ECal 
 * @author Josh Hiltbrand, University of Minnesota 
 */

#include "Ecal/EcalClusterProducer.h"

namespace ldmx {

    EcalClusterProducer::EcalClusterProducer(const std::string& name, Process& process) :
        Producer(name, process) { } 

    EcalClusterProducer::~EcalClusterProducer() { }

    void EcalClusterProducer::configure(Parameters& parameters) {
        cutoff_ = parameters.getParameter< double >("cutoff");
        seedThreshold_ = parameters.getParameter< double >("seedThreshold"); 
        digisPassName_ = parameters.getParameter< std::string >("digisPassName");
        algoCollName_ = parameters.getParameter< std::string >("algoCollName");
        algoName_ = parameters.getParameter< std::string >("algoName");
        clusterCollName_ = parameters.getParameter< std::string >("clusterCollName");

    }

    void EcalClusterProducer::produce(Event& event) {
	// Get the Ecal Geometry
	const EcalHexReadout& hexReadout = getCondition<EcalHexReadout>(EcalHexReadout::CONDITIONS_OBJECT_NAME);
    
        TemplatedClusterFinder<MyClusterWeight> cf;

        std::vector< EcalHit > ecalHits = event.getCollection< EcalHit >( "ecalDigis" , digisPassName_ );
        int nEcalDigis = ecalHits.size();

        // Don't do anything if there are no ECal digis!
        if (!(nEcalDigis > 0)) { return; }
        
        for (EcalHit &hit : ecalHits ) {

            //Skip zero energy digis.
            if (hit.getEnergy() == 0) { continue; }

            cf.add( &hit , hexReadout);
        }

        cf.cluster(seedThreshold_, cutoff_);
        std::vector<WorkingCluster> wcVec = cf.getClusters();
    
        std::map<int, double> cWeights = cf.getWeights();
    
        ClusterAlgoResult algoResult;
        algoResult.set(algoName_, 3, cWeights.rbegin()->first);
        algoResult.setAlgoVar(0, cutoff_);
        algoResult.setAlgoVar(1, seedThreshold_);
        algoResult.setAlgoVar(2, cf.getNSeeds());
    
        std::map<int, double>::iterator it = cWeights.begin();
        for (it = cWeights.begin(); it != cWeights.end(); it++) {
            algoResult.setWeight(it->first, it->second/100);
        }

        std::vector< EcalCluster > ecalClusters;
        for (int aWC = 0; aWC < wcVec.size(); aWC++) {
    
            EcalCluster cluster;
    
            cluster.setEnergy(wcVec[aWC].centroid().E());
            cluster.setCentroidXYZ(wcVec[aWC].centroid().Px(), wcVec[aWC].centroid().Py(), wcVec[aWC].centroid().Pz());
            cluster.setNHits(wcVec[aWC].getHits().size());
            cluster.addHits(wcVec[aWC].getHits());
    
            ecalClusters.push_back( cluster );
        }

        event.add(clusterCollName_, ecalClusters );
        event.add(algoCollName_, algoResult );
    } 
}

DECLARE_PRODUCER_NS(ldmx, EcalClusterProducer);
