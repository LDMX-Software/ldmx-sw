/**
 * @file EcalClusterProducer.cxx
 * @brief Producer that performs clustering in the ECal 
 * @author Josh Hiltbrand, University of Minnesota 
 */

#include "Ecal/EcalClusterProducer.h"

namespace ldmx {

    EcalClusterProducer::EcalClusterProducer(const std::string& name, Process& process) :
        Producer(name, process) {

    }

    EcalClusterProducer::~EcalClusterProducer() {
        if ( ecalClusters_ ) delete ecalClusters_;
    }

    void EcalClusterProducer::configure(const ParameterSet& ps) {

        hexReadout_ = std::make_shared<EcalHexReadout>();
        cutoff_ = ps.getDouble("cutoff");
        seedThreshold_ = ps.getDouble("seedThreshold"); 
        digisPassName_ = ps.getString("digisPassName");
        algoCollName_ = ps.getString("algoCollName");
        algoName_ = ps.getString("algoName");
        clusterCollName_ = ps.getString("clusterCollName");
        ecalClusters_ = new TClonesArray(EventConstants::ECAL_CLUSTER.c_str(), 10000);

    }

    void EcalClusterProducer::produce(Event& event) {

        static const double layerZPos[] = {-137.2, -134.3, -127.95, -123.55, -115.7, -109.8, -100.7, -94.3, -85.2, -78.8, -69.7, -63.3, -54.2, -47.8, -38.7, -32.3, -23.2, -16.8, -7.7, -1.3, 7.8, 14.2, 23.3, 29.7, 42.3, 52.2, 64.8, 74.7, 87.3, 97.2, 109.8, 119.7, 132.3, 142.2};

        TemplatedClusterFinder<MyClusterWeight> cf;

        TClonesArray* ecalDigiHits = (TClonesArray*) event.getCollection("ecalDigis", digisPassName_);
        int nEcalDigis = ecalDigiHits->GetEntries();

        // Don't do anything if there are no ECal digis!
        if (!(nEcalDigis > 0)) { return; }
        
        for (int iDigi = 0; iDigi < nEcalDigis; iDigi++) {

            EcalHit* aDigi = (EcalHit*) ecalDigiHits->At(iDigi);

            //Skip zero energy digis.
            if (aDigi->getEnergy() == 0) { continue; }

            cf.add(aDigi, hexReadout_, layerZPos[aDigi->getLayer()]);
        }

        cf.cluster(seedThreshold_, cutoff_);
        std::vector<WorkingCluster> wcVec = cf.getClusters();
    
        std::map<int, double> cWeights = cf.getWeights();
    
        algoResult_.set(algoName_, 3, cWeights.rbegin()->first);
        algoResult_.setAlgoVar(0, cutoff_);
        algoResult_.setAlgoVar(1, seedThreshold_);
        algoResult_.setAlgoVar(2, cf.getNSeeds());
    
        std::map<int, double>::iterator it = cWeights.begin();
        for (it = cWeights.begin(); it != cWeights.end(); it++) {
            algoResult_.setWeight(it->first, it->second/100);
        }

        int iC = 0;
        for (int aWC = 0; aWC < wcVec.size(); aWC++) {
    
            EcalCluster* cluster = (EcalCluster*) (ecalClusters_->ConstructedAt(iC));
    
            cluster->setEnergy(wcVec[aWC].centroid().E());
            cluster->setCentroidXYZ(wcVec[aWC].centroid().Px(), wcVec[aWC].centroid().Py(), wcVec[aWC].centroid().Pz());
            cluster->setNHits(wcVec[aWC].getHits().size());
            cluster->addHits(wcVec[aWC].getHits());
    
            iC++;
        }

        event.add(clusterCollName_, ecalClusters_);
        event.addToCollection(algoCollName_, algoResult_);
    } 
}

DECLARE_PRODUCER_NS(ldmx, EcalClusterProducer);
