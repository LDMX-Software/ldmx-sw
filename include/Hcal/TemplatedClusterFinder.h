/*
   TemplatedClusterFinder
   */

#ifndef HCAL_TEMPLATEDCLUSTERFINDER_H_ 
#define HCAL_TEMPLATEDCLUSTERFINDER_H_

#include "Hcal/WorkingCluster.h"
#include "TH2F.h"

#include <math.h>
#include <map>

namespace hcal {

    template <class WeightClass>

    class TemplatedClusterFinder {
    
        public:

            void add(const ldmx::HcalHit* eh, const ldmx::HcalGeometry& hex) {
         
                clusters_.push_back(WorkingCluster(eh, hex));
            }
            
    

            static bool compClusters(const WorkingCluster& a, const WorkingCluster& b) {
                return a.centroid().E() > b.centroid().E();
            }

            void cluster(double seed_threshold, double cutoff) {
                int ncluster = clusters_.size();
                double minwgt = cutoff;

                std::sort(clusters_.begin(), clusters_.end(), compClusters);
                do {
                    bool any = false;
                    unsigned int mi(0),mj(0);
   
                    int nseeds = 0;

                    for (unsigned int i = 0; i < clusters_.size(); i++) {
    
                        if (clusters_[i].empty()) continue;

                        bool iseed = (clusters_[i].centroid().E() >= seed_threshold);
                        if (iseed) {
                            nseeds++;
                        } else {
                            // Since we sorted initially if we find a hit below seed threshold
                            // it is guaranteed that there will be no seeds after.
                            break;
                        }

                        for (unsigned int j = i + 1; j < clusters_.size(); j++) {
    
                            if (clusters_[j].empty() || (!iseed && clusters_[j].centroid().E() < seed_threshold)) continue;
                            double wgt = wgt_(clusters_[i],clusters_[j]);
                            if (!any || wgt < minwgt) {
                                any = true;
                                minwgt = wgt;
                                mi = i;
                                mj = j;
                            }
                        }
                    }

                    nseeds_ = nseeds;
                    transitionWeights_.insert(std::pair<int, double>(ncluster, minwgt));
    
                    if (any && minwgt < cutoff) {
                        // put the bigger one in mi
                        if (clusters_[mi].centroid().E() < clusters_[mj].centroid().E()) { std::swap(mi,mj); }
                        // now we have the smallest, merge
                        clusters_[mi].add(clusters_[mj]);
                        clusters_[mj].clear();
                        // decrement cluster count
                        ncluster--;
                    } 
    
                } while (minwgt < cutoff && ncluster > 1);
                finalwgt_ = minwgt;
            }

            double getYMax() const { return finalwgt_; }

            int getNSeeds() const { return nseeds_; }

            std::map<int, double> getWeights() const { return transitionWeights_; }

            std::vector<WorkingCluster> getClusters() const {
                return clusters_;
            }
    
        private:
    
            WeightClass wgt_;
            double finalwgt_;
            int nseeds_;
            std::map<int, double> transitionWeights_;
            std::vector<WorkingCluster> clusters_;
    };
}

#endif
