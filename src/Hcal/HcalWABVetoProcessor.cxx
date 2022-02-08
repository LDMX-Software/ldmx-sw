/**
 * @file HcalWABVetoProcessor.cxx
 * @brief Processor that determines if an event is vetoed by the Hcal.
 * @author Sophie Middleton, Caltech
 */

#include "Hcal/HcalWABVetoProcessor.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "DetDescr/HcalID.h"
#include <numeric>
using namespace std;
namespace hcal {

    HcalWABVetoProcessor::HcalWABVetoProcessor(const std::string &name,
                                         framework::Process &process)
        : Producer(name, process) {}

    HcalWABVetoProcessor::~HcalWABVetoProcessor() {}

    void HcalWABVetoProcessor::configure(framework::config::Parameters &parameters) {
        maxtotalEnergyCompare_ = parameters.getParameter<double>("max_total_energy_compare");
        mintotalEnergyCompare_ = parameters.getParameter<double>("min_total_energy_compare");
        maxnClusters_ = parameters.getParameter<double>("n_clusters");
        maxMeanHitsPerCluster_ = parameters.getParameter<double>("mean_hits_per_cluster");
        maxMeanEnergyPerCluster_ = parameters.getParameter<double>("mean_energy_per_cluster");
        outputCollName_ = parameters.getParameter<std::string>("outputCollName");
        inputHCALClusterCollName_ = parameters.getParameter<std::string>("inputHCALClusterCollName");
        inputHCALHitCollName_ = parameters.getParameter<std::string>("inputHCALHitCollName");
        inputECALHitCollName_  = parameters.getParameter<std::string>("inputECALHitCollName");
    }

    void HcalWABVetoProcessor::produce(framework::Event &event) {

        // Get the collection of sim particles from the event
        //HCAL:
        const std::vector<ldmx::HcalHit> hcalRecHits =
        event.getCollection<ldmx::HcalHit>(inputHCALHitCollName_);
        //ECAL:
        const std::vector<ldmx::EcalHit> ecalRecHits =
        event.getCollection<ldmx::EcalHit>(inputECALHitCollName_);
        //Clusters:
        const std::vector<ldmx::HcalCluster> hcalClusters =
        event.getCollection<ldmx::HcalCluster>(inputHCALClusterCollName_);

        // Loop over all of the Hcal hits and calculate to total photoelectrons
        // in the event.
        float totalHCALEnergy{0};
        float totalECALEnergy{0};
        float maxPE{-1000};
        const ldmx::HcalHit *maxPEHit;
        for (const ldmx::HcalHit &hcalHit : hcalRecHits) {
                if (hcalHit.isNoise()==0){
                        totalHCALEnergy += hcalHit.getPE();
                }

                // Find the maximum PE in the list
                if (maxPE < hcalHit.getPE()) {
                  maxPE = hcalHit.getPE();
                  maxPEHit = &hcalHit;
                }
            }

        for (const ldmx::EcalHit &ecalHit : ecalRecHits) {
                if (ecalHit.isNoise()==0){
                        totalECALEnergy += ecalHit.getEnergy();
                }
            }
        std::vector<double> nhits;
        std::vector<double> energies;
        unsigned int nClusters = 0;
        for (const ldmx::HcalCluster &hcalCluster : hcalClusters) {
                nClusters += 1;
                energies.push_back(hcalCluster.getEnergy());
                nhits.push_back(hcalCluster.getNHits());
            }
        double meanEnergy = std::accumulate(energies.begin(), energies.end(), 0.0) / energies.size();
        double meanNhits = std::accumulate(nhits.begin(), nhits.end(), 0.0) / nhits.size();
        std::cout<<totalECALEnergy + totalHCALEnergy<<" "<<nClusters<<" "<<meanEnergy<<" "<<meanNhits<<std::endl;
        bool passesEnergyCombo = (totalECALEnergy + totalHCALEnergy < maxtotalEnergyCompare_  and totalECALEnergy + totalHCALEnergy > mintotalEnergyCompare_);
        bool passesnClusters = true ;//(nClusters < maxnClusters_);
        bool passesNHits = true;//(meanNhits <  maxMeanHitsPerCluster_ );
        bool passesEnergy =  true;//(meanEnergy < maxMeanEnergyPerCluster_);

        //total veto:
        bool passesVeto = (passesEnergyCombo and passesnClusters and passesNHits and passesEnergy);
        ldmx::HcalVetoResult result;
        result.setVetoResult(passesVeto);
        result.setMaxPEHit(*maxPEHit);
        if (passesVeto) {
            std::cout<<"passes "<<std::endl;
            setStorageHint(framework::hint_shouldKeep);

        } else {
           setStorageHint(framework::hint_shouldDrop);
        }

        event.add(outputCollName_, result);

        }
}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalWABVetoProcessor);
