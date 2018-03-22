/**
 * @file ClusterAnalyzer.cxx
 * @brief Class that defines a Cluster Analyzer that does stuff 
 * @author Josh Hiltbrand, University of Minnesota
 */

#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"
#include <iostream>
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "Event/Event.h"
#include "Event/EcalCluster.h"
#include "Event/ClusterAlgoResult.h"
#include "Event/SimParticle.h"
#include "TClonesArray.h"

namespace ldmx {

    /**
     * @class ClusterAnalyzer
     * @brief Analyzes clusters and cluster algo results
     */
    class ClusterAnalyzer : public ldmx::Analyzer {

        public:

            ClusterAnalyzer(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name, process) {}

            virtual void configure(const ldmx::ParameterSet& ps) {

                hexReadout_ = new EcalHexReadout();
                passName_ = ps.getString("passName");
                clusterCollName_ = ps.getString("clusterCollName");
                algoCollName_ = ps.getString("algoCollName");
                minHits_ = ps.getInteger("minHits");
            }

            double getSimSeparation(TClonesArray* simPs) {

                int numSims = simPs->GetEntriesFast();
                std::vector<double> simXY1;
                std::vector<double> simXY2;

                int found = 0;
                for (int iS = 0; iS < numSims; iS++) {
               
                    SimParticle* aS = (SimParticle*) simPs->At(iS);
                    if (aS->getPdgID() == 11 && aS->getParentCount() == 0) {
                        if (found == 0) { 
                            simXY1 = aS->getVertex(); 
                            found = 1;
                        } else if (found == 1) {
                            simXY2 = aS->getVertex();
                            found = 2;
                            break;
                        }
                    }
                }

                double separation;
                if (found < 2) { 
                    separation = 0.0; 
                } else {
                    separation = pow(pow(simXY1[0]-simXY2[0],2) + pow(simXY1[1]-simXY2[1],2), 0.5);
                }

                return separation;
            }

            double getClusterRSep(EcalCluster* ac, EcalCluster* bc) {

                double ax = ac->getCentroidX();
                double ay = ac->getCentroidY();
                double bx = bc->getCentroidX();
                double by = bc->getCentroidY();

                double separation = pow(pow(ax-bx,2) + pow(ay-by,2), 0.5);

                return separation;
            }

            double getClusterZSep(EcalCluster* ac, EcalCluster* bc) {

                double az = ac->getCentroidZ();
                double bz = bc->getCentroidZ();

                double separation = az-bz; 

                return separation;
            }

            double getClusterEmptyLayers(EcalCluster* ac) {

                std::vector<int> hitLayers;
                std::vector<unsigned int> clusterHitIDs = ac->getHitIDs();
                for (int iHit = 0; iHit < clusterHitIDs.size(); iHit++) {
                    int layer = (clusterHitIDs[iHit]<<20)>>24;
                    hitLayers.push_back(layer);
                }

                std::sort(hitLayers.begin(), hitLayers.end()); 

                int largestDiff = 0;
                for (int iL = 0; iL < hitLayers.size()-1; iL++) {
                    double diff = hitLayers[iL] - hitLayers[iL+1];
                    if (diff > largestDiff) {
                        largestDiff = diff;
                    }
                }

                return largestDiff;
            }


            virtual void analyze(const ldmx::Event& event) {

                std::cout << "ClusterAnalyzer: Analyzing an event!" << std::endl;

                const TClonesArray* clusters = event.getCollection(clusterCollName_, passName_);
                const TClonesArray* car = event.getCollection(algoCollName_, passName_);
                const ldmx::ClusterAlgoResult* algoRes = (const ldmx::ClusterAlgoResult*)(car->At(0));
                const TClonesArray* simPs = event.getCollection("SimParticles");

                double separation = getSimSeparation(simPs);

                int numClusters = clusters->GetEntries();
                int numClustersCut = numClusters; //Count clusters above hits thresh and energy
                double energyLO = 0.0;
                EcalCluster* clusterLO;
                for (int iC = 0; iC < numClusters; iC++) {

                    EcalCluster* aC = (EcalCluster*) clusters->At(iC);
                    int cHits = aC->getNHits();
                    double cEnergy = aC->getEnergy();

                    if (cHits < minHits_) {
                        numClustersCut--;
                    }

                    if (cEnergy > energyLO) {
                        energyLO = cEnergy;
                        clusterLO = aC;
                    }
                }

                numClusters_->Fill(separation, numClustersCut);
                nSeedsHisto_->Fill(separation, algoRes->getAlgoVar2());

                for (int iC = 0; iC < numClusters; iC++) {
                
                    EcalCluster* aC = (EcalCluster*) clusters->At(iC);

                    int cHits = aC->getNHits();

                    if (cHits < minHits_) { continue; }

                    double cEnergy = aC->getEnergy(); //MeV
                    double cXY = pow(pow(aC->getCentroidY(),2) + pow(aC->getCentroidX(),2),0.5); 
                    double cZ = aC->getCentroidZ();
                    std::vector<unsigned int> hitIDs = aC->getHitIDs()
                
                    energyPerCluster_->Fill(separation, cEnergy);
                    hitsPerCluster_->Fill(separation, cHits);

                    double rSep = getClusterRSep(clusterLO, aC);
                    double zSep = getClusterZSep(clusterLO, aC);

                    if (cEnergy < energy) {
                        energyPerClusterNLO_->Fill(cEnergy, numClustersCut);
                        clusterZSepNLO_->Fill(zSep, numClustersCut);
                        clusterTSepNLO_->Fill(rSep, numClustersCut);
                        hitsPerClusterNLO_->Fill(cHits, numClustersCut);
                    } else {
                        energyPerClusterLO_->Fill(cEnergy, numClustersCut);
                        hitsPerClusterLO_->Fill(cHits, numClustersCut);
                    }
                }

                //for (int iCW = 1; iCW < 20; iCW++) {
                //    weightsHisto_->Fill(iCW, algoRes->getWeight(iCW), separation);
                //}
            }

            virtual void onFileOpen() {
                std::cout << "ClusterAnalyzer: Opening a file!" << std::endl;
            }
            
            virtual void onFileClose() {
                std::cout << "ClusterAnalyzer: Closing a file!" << std::endl;
            }

            virtual void onProcessStart() {
                std::cout << "ClusterAnalyzer: Starting processing!" << std::endl;
                getHistoDirectory();

                numClusters_ = new TH2F("numClusters", "Clusters per Event;Sim Particle Separation [mm];Number of Clusters", 200, 0, 50, 20, -0.5, 19.5);
                hitsPerCluster_ = new TH2F("hitsPerCluster", "Hits per Cluster;Sim Particle Separation [mm];Number of Hits", 200, 0, 50, 100, -0.5, 99.5);
                nSeedsHisto_ = new TH2F("nSeedsHisto", "Seeds in Event;Sim Particle Separation;Number of Seeds", 200, 0, 50, 20, -0.5, 19.5);
                energyPerCluster_ = new TH2F("energyPerCluster", "Cluster Energy;Sim Particle Separation [mm];Energy [MeV]", 200, 0, 200, 500, 0, 10000);

                energyPerClusterLO_ = new TH2F("energyPerClusterLO", "Cluster Energy;Energy [MeV];Number of Clusters", 500, 0, 10000, 10, -0.5, 9.5);
                hitsPerClusterLO_ = new TH2F("hitsPerClusterLO", "Hits per Cluster;Number of Hits;Number of Clusters", 500, -0.5, 499.5, 10, -0.5, 9.5);

                energyPerClusterNLO_ = new TH2F("energyPerClusterNLO", "Cluster Energy;Energy [MeV];Number of Clusters", 500, 0, 10000, 10, -0.5, 9.5);
                clusterZSepNLO_ = new TH2F("clusterZNLO", "Sub-Leading Cluster Z Separation;Distance [mm];Number of Clusters", 150, -150, 150, 10, -0.5, 9.5);
                clusterTSepNLO_ = new TH2F("clusterTNLO", "Sub-Leading Cluster R Separation;Distance [mm];Number of Clusters", 130, 0, 260, 10, -0.5, 9.5);
                hitsPerClusterNLO_ = new TH2F("hitsPerClusterNLO", "Hits per Cluster;Number of Hits;Number of Clusters", 500, -0.5, 499.5, 10, -0.5, 9.5);

            }

            virtual void onProcessEnd() {
                std::cout << "ClusterAnalyzer: Finishing processing!" << std::endl;
            }

        private:

            TH2F* numClusters_;
            TH2F* hitsPerCluster_;
            TH2F* nSeedsHisto_;
            TH2F* energyPerCluster_;

            TH2F* energyPerClusterLO_;
            TH2F* clusterZLO_;
            TH2F* clusterTLO_;
            TH2F* hitsPerClusterLO_;

            TH2F* energyPerClusterNLO_;
            TH2F* clusterZNLO_;
            TH2F* clusterTNLO_;
            TH2F* hitsPerClusterNLO_;

            std::string passName_;
            std::string clusterCollName_;
            std::string algoCollName_;

            double minEnergy_;
            double minHits_;

            EcalHexReadout* hexReadout_{nullptr};

    };
}

DECLARE_ANALYZER_NS(ldmx, ClusterAnalyzer);
