/*
   WorkingCluster -- In-memory tool for working on clusters
   */
#ifndef ECAL_WORKINGCLUSTER_H_
#define ECAL_WORKINGCLUSTER_H_

#include "Event/EcalHit.h"
#include <vector>
#include <iostream>
#include "TLorentzVector.h"
#include "DetDescr/EcalHexReadout.h"

namespace ldmx {

    class WorkingCluster {

        public:

            WorkingCluster(const EcalHit* eh, const std::shared_ptr<EcalHexReadout> hex, double zPos);

            ~WorkingCluster() {};

            void add(const EcalHit* eh, const std::shared_ptr<EcalHexReadout> hex, double zPos);
    
            void add(const WorkingCluster& wc);

            const TLorentzVector& centroid() const { 
                return centroid_; 
            } 

            std::vector<const EcalHit*> getHits() const {
                return hits_;
            }

            bool empty() const { return hits_.empty(); }

            void clear() { hits_.clear(); }

        private:


            std::vector<const EcalHit*> hits_;
            TLorentzVector centroid_;
    };
}

#endif
