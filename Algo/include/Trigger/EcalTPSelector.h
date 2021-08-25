/**
 * @file EcalTPSelector.h
 * @brief ECal clustering algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef ECALTPSELECTOR_H
#define ECALTPSELECTOR_H

// LDMX Framework
#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Trigger/Event/TrigCaloHit.h"
#include "Trigger/Event/TrigEnergySum.h"

#include "DetDescr/EcalHexReadout.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace trigger {

/**
 * @class EcalTPSelector
 * @brief
 */
class EcalTPSelector : public framework::Producer {
 public:
  EcalTPSelector(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

  // helpers
  void decodeTP(ldmx::HgcrocTrigDigi tp, double &x, double &y, double &z, double &e);
  double primitiveToEnergy(int tp, int layer);
    
 private:
  // specific verbosity of this producer
  int verbose_{0};

  // name of collection for EcalTPs to be passed as input
  std::string tpCollName_;
  // name of output collection
  std::string passCollName_;

  unsigned int maxCentralTPs_{12};
  unsigned int maxOuterTPs_{8};
  
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

};
}  // namespace trigger

#endif /* ECALTPSELECTOR_H */
