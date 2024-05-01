/**
 * @file EcalTPSelector.h
 * @brief ECal clustering algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef ECALTPSELECTOR_H
#define ECALTPSELECTOR_H

// LDMX Framework
#include "TrigUtilities.h"
#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Trigger/Event/TrigCaloHit.h"
#include "Trigger/Event/TrigEnergySum.h"

#include "DetDescr/EcalGeometry.h"
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
  /* double primitiveToEnergy(int tp, int layer); */
    
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
  /* float gain_ = 320. / 0.1 / 1024;                                    // mV/ADC */
  /* float mVtoMeV_ = 0.130 / (37000.0 * (0.1602 / 1000.) * (1. / 0.1));  // MeV/mV */
  /* std::vector<float> layerWeights = { */
  /*     2.312, 4.312, 6.522, 7.490, 8.595, 10.253, 10.915, 10.915, 10.915, 10.915, 10.915, */
  /*     10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, */
  /*     10.915, 10.915, 14.783, 18.539, 18.539, 18.539, 18.539, 18.539, 18.539, 18.539, */
  /*     18.539, 18.539, 9.938}; */
  /* float secondOrderEnergyCorrection_ = 4000. / 3940.5; */
  /* float mipSiEnergy_ = 0.130; */
  /* float adHoc_ = 1.0; // my adhoc correction factor, to match v14 :( */
  /* int hgc_compression_factor_ = 8; */

};
}  // namespace trigger

#endif /* ECALTPSELECTOR_H */
