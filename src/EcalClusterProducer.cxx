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

        // These are the v12 parameters
        //  all distances in mm
        double moduleRadius = 85.0; //same as default
        int    numCellsWide = 23; //same as default
        double moduleGap = 1.5;
        double ecalFrontZ = 220;
        std::vector<double> ecalSensLayersZ = {
             7.850,
            13.300,
            26.400,
            33.500,
            47.950,
            56.550,
            72.250,
            81.350,
            97.050,
            106.150,
            121.850,
            130.950,
            146.650,
            155.750,
            171.450,
            180.550,
            196.250,
            205.350,
            221.050,
            230.150,
            245.850,
            254.950,
            270.650,
            279.750,
            298.950,
            311.550,
            330.750,
            343.350,
            362.550,
            375.150,
            394.350,
            406.950,
            426.150,
            438.750
        };

        hexReadout_ = std::make_shared<EcalHexReadout>(
                moduleRadius,
                moduleGap,
                numCellsWide,
                ecalSensLayersZ,
                ecalFrontZ
                );

        cutoff_ = parameters.getParameter< double >("cutoff");
        seedThreshold_ = parameters.getParameter< double >("seedThreshold"); 
        digisPassName_ = parameters.getParameter< std::string >("digisPassName");
        algoCollName_ = parameters.getParameter< std::string >("algoCollName");
        algoName_ = parameters.getParameter< std::string >("algoName");
        clusterCollName_ = parameters.getParameter< std::string >("clusterCollName");

    }

    void EcalClusterProducer::produce(Event& event) {

        static const double layerZPos[] = {-137.2, -134.3, -127.95, -123.55, -115.7, -109.8, -100.7, -94.3, -85.2, -78.8, -69.7, -63.3, -54.2, -47.8, -38.7, -32.3, -23.2, -16.8, -7.7, -1.3, 7.8, 14.2, 23.3, 29.7, 42.3, 52.2, 64.8, 74.7, 87.3, 97.2, 109.8, 119.7, 132.3, 142.2};

        TemplatedClusterFinder<MyClusterWeight> cf;

        std::vector< EcalHit > ecalHits = event.getCollection< EcalHit >( "ecalDigis" , digisPassName_ );
        int nEcalDigis = ecalHits.size();

        // Don't do anything if there are no ECal digis!
        if (!(nEcalDigis > 0)) { return; }
        
        for (EcalHit &hit : ecalHits ) {

            //Skip zero energy digis.
            if (hit.getEnergy() == 0) { continue; }

            EcalID id(hit.getID());
            cf.add( &hit , hexReadout_, layerZPos[id.layer()]);
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
