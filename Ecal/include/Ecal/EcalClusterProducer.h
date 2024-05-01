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
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"

//----------//
//   LDMX   //
//----------//
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
#include "Ecal/Event/ClusterAlgoResult.h"
#include "Ecal/Event/EcalCluster.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/MyClusterWeight.h"
#include "Ecal/TemplatedClusterFinder.h"
#include "Ecal/WorkingCluster.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//----------//
//    STL   //
//----------//
#include <memory>
#include <tuple>

namespace ecal {

/**
 * @class EcalClusterProducer
 * @brief Simple algorithm that does clustering in the ECal
 */
class EcalClusterProducer : public framework::Producer {
 public:
  EcalClusterProducer(const std::string& name, framework::Process& process);

  virtual ~EcalClusterProducer();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  virtual void produce(framework::Event& event);

 private:
  double seedThreshold_{0};
  double cutoff_{0};
  std::string digisPassName_;
  std::string algoCollName_;
  std::string clusterCollName_;

  /** The name of the cluster algorithm used. */
  TString algoName_;
};
}  // namespace ecal

#endif
