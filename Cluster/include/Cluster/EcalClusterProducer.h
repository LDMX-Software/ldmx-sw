/**
 * @file EcalClusterProducer.h
 * @brief Simple algorithm that does clustering in the ECal 
 * @author Josh Hiltbrand, University of Minnesota 
 */

#ifndef EVENTPROC_ECALCLUSTERPRODUCER_H_
#define EVENTPROC_ECALCLUSTERPRODUCER_H_

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TFile.h"

//----------//
//   LDMX   //
//----------//
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"
#include "Event/EcalCluster.h"
#include "Event/ClusterAlgoResult.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Cluster/WorkingCluster.h"
#include "Cluster/ClusterWeight.h"
#include "Cluster/TemplatedClusterFinder.h"

//----------//
//    STL   //
//----------//
#include <tuple>

namespace ldmx {

    /**
     * @class EcalClusterProducer
     * @brief Simple algorithm that does clustering in the ECal 
     */
    class EcalClusterProducer : public Producer {

        public:

            EcalClusterProducer(const std::string& name, Process& process);

            virtual ~EcalClusterProducer() {
            }

            virtual void configure(const ParameterSet&);

            virtual void produce(Event& event);

        private:

            TClonesArray* ecalClusters_{nullptr};
            EcalHexReadout* hexReadout_{nullptr};
            double seedThreshold_{0};
            double cutoff_{0};
            std::string digisPassName_;
            std::string algoCollName_;
            std::string clusterCollName_;

            /** Object to hold cluster algo variables */
            ClusterAlgoResult algoResult_;

            /** The name of the cluster algorithm used. */
            TString algoName_;

    };
}

#endif
