/**
 * @file EcalClusterProducer.h
 * @brief Simple algorithm that does clustering in the ECal 
 * @author Josh Hiltbrand, University of Minnesota 
 */

#ifndef ECAL_ECALCLUSTERPRODUCER_H_
#define ECAL_ECALCLUSTERPRODUCER_H_

//----------//
//   ROOT   //
//----------//
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
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 
#include "Ecal/WorkingCluster.h"
#include "Ecal/MyClusterWeight.h"
#include "Ecal/TemplatedClusterFinder.h"

//----------//
//    STL   //
//----------//
#include <tuple>
#include <memory>

namespace ldmx {

    /**
     * @class EcalClusterProducer
     * @brief Simple algorithm that does clustering in the ECal 
     */
    class EcalClusterProducer : public Producer {

        public:

            EcalClusterProducer(const std::string& name, Process& process);

            virtual ~EcalClusterProducer();

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this processor.
             */
            void configure(Parameters& parameters) final override;

            virtual void produce(Event& event);

        private:

            std::shared_ptr<EcalHexReadout> hexReadout_;
            double seedThreshold_{0};
            double cutoff_{0};
            std::string digisPassName_;
            std::string algoCollName_;
            std::string clusterCollName_;

            /** The name of the cluster algorithm used. */
            TString algoName_;

    };
}

#endif
