/**
 * @file TrigEcalClusterProducer.h
 * @brief ECal clustering algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef TRIGECALCLUSTERPRODUCER_H
#define TRIGECALCLUSTERPRODUCER_H

// LDMX Framework
#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "ap_fixed.h"
#include "ap_int.h"

namespace trigger {

/**
 * @class TrigEcalClusterProducer
 * @brief
 */
class TrigEcalClusterProducer : public framework::Producer {
 public:
  TrigEcalClusterProducer(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

 private:
  // specific verbosity of this producer
  int verbose_{0};

  // name of collection for trigHits to be passed as input
  std::string hitCollName_;
  // name of collection for trigCluster to be output
  std::string clusterCollName_;

  // From:
  // Tools/python/HgcrocEmulator.py
  // ECal/python/digi.py
  // ECal/src/EcalRecProducer.cxx
  float gain_ = 320. / 0.1 / 1024;                                    // mV/ADC
  float mVtoMeV_ = 0.130 / (37000.0 * (0.162 / 1000.) * (1. / 0.1));  // MeV/mV
  std::vector<float> layerWeights = {
      1.675,  2.724,  4.398,  6.039,  7.696,  9.077,  9.630,  9.630,  9.630,
      9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,
      9.630,  9.630,  9.630,  9.630,  9.630,  13.497, 17.364, 17.364, 17.364,
      17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 8.990};
  float secondOrderEnergyCorrection_ = 4000. / 4010.;
  float mipSiEnergy_ = 0.130;
  int hgc_compression_factor_ = 8;

  // ClusterGeometry myGeo;
};
}  // namespace trigger

#endif /* TRIGECALCLUSTERPRODUCER_H */
