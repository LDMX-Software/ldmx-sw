/**
 * @file DumpFileWriter.h
 * @brief Write objects to a file for standalone
 * @author Christian Herwig, Fermilab
 */

#ifndef DUMPFILEWRITER_H
#define DUMPFILEWRITER_H

// LDMX Framework
#include "DiscreteInputs.h"
#include "DiscreteInputs_IO.h"
#include "Ecal/EcalTriggerGeometry.h"
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "ap_fixed.h"
#include "ap_int.h"

namespace trigger {

/**
 * @class DumpFileWriter
 * @brief
 */
class DumpFileWriter : public framework::Analyzer {
 public:
  DumpFileWriter(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void analyze(const framework::Event& event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

  typedef ap_ufixed<16, 14> e_t;  // [MeV] (Up to at least 8 GeV)

 private:
  // specific verbosity
  int verbose_{0};

  // From:
  // Tools/python/HgcrocEmulator.py
  // ECal/python/digi.py
  // ECal/src/EcalRecProducer.cxx
  float gain = 320. / 0.1 / 1024;                                    // mV/ADC
  float mVtoMeV = 0.130 / (37000.0 * (0.162 / 1000.) * (1. / 0.1));  // MeV/mV
  std::vector<float> layerWeights = {
      1.675,  2.724,  4.398,  6.039,  7.696,  9.077,  9.630,  9.630,  9.630,
      9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,
      9.630,  9.630,  9.630,  9.630,  9.630,  13.497, 17.364, 17.364, 17.364,
      17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 8.990};
  float secondOrderEnergyCorrection = 4000. / 4010.;
  float mipSiEnergy = 0.130;

  // ClusterGeometry myGeo;

  std::string dumpFileName = "dummy.dump";
  EventDump myEvent;
  FILE* file = 0;
  unsigned long evtNo = 0;
};
}  // namespace trigger

#endif /* DUMPFILEWRITER_H */
