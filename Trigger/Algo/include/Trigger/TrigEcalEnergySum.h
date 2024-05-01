/**
 * @file TrigEcalEnergySum.h
 * @brief EcalEnergySum algo
 * @author Christian Herwig, Fermilab
 */

#ifndef TRIGGERECALENERGYSUM_H
#define TRIGGERECALENERGYSUM_H

// LDMX Framework
#include "TrigUtilities.h"
#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "ap_fixed.h"
#include "ap_int.h"

namespace trigger {

/**
 * @class TrigEcalEnergySum
 * @brief
 */
class TrigEcalEnergySum : public framework::Producer {
 public:
  TrigEcalEnergySum(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {
    //std::cout << "c++ initializing TrigEcalEnergySum" << std::endl;
  }

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

  typedef ap_ufixed<16, 14> e_t;  // [MeV] (Up to at least 8 GeV)

 private:
  // specific verbosity of this producer
  int verbose_{0};

  // name of collection for trigHits to be passed as input
  std::string hitCollName_;

  // From:
  // Tools/python/HgcrocEmulator.py
  // ECal/python/digi.py
  // ECal/src/EcalRecProducer.cxx
  /* float gain = 320. / 0.1 / 1024;                                    // mV/ADC */
  /* float mVtoMeV = 0.130 / (37000.0 * (0.162 / 1000.) * (1. / 0.1));  // MeV/mV */
  /* std::vector<float> layerWeights = { */
  /*     1.675,  2.724,  4.398,  6.039,  7.696,  9.077,  9.630,  9.630,  9.630, */
  /*     9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630, */
  /*     9.630,  9.630,  9.630,  9.630,  9.630,  13.497, 17.364, 17.364, 17.364, */
  /*     17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 8.990}; */
  /* float secondOrderEnergyCorrection = 4000. / 4010.; */
  /* float mipSiEnergy = 0.130; */

  // ClusterGeometry myGeo;
};
}  // namespace trigger

#endif /* TRIGGERECALENERGYSUM_H */
