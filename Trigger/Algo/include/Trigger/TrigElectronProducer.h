/**
 * @file TrigElectronProducer.h
 * @brief Trigger electron reconstruction algorithm
 * @author Christian Herwig, Fermilab
 */

#ifndef TRIGECALCLUSTERPRODUCER_H
#define TRIGECALCLUSTERPRODUCER_H

#include <optional>

// LDMX Framework
/* #include "Ecal/EcalTriggerGeometry.h" */
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TF1.h"
#include "TProfile2D.h"
/* #include "ap_fixed.h" */
/* #include "ap_int.h" */

namespace trigger {

/**
 * @class TrigElectronProducer
 * @brief
 */
class TrigElectronProducer : public framework::Producer {
 public:
  TrigElectronProducer(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  virtual void configure(framework::config::Parameters& ps);

  virtual void produce(framework::Event& event);

  virtual void onProcessStart();

  virtual void onProcessEnd();

  void setupMaps(bool isX);
  float getP(bool isX, float e, float d);
  float getPx(float e, float d) { return getP(true, e, d); }
  float getPy(float e, float d) { return getP(false, e, d); }

 private:
  // y [mm] of the target point from the lowest-residual TS track
  std::optional<float> tsTargetY(const framework::Event& event);

  // TrigScint tracks giving the target y
  std::string ts_track_coll_name_;
  std::string ts_passname_;
  // y [mm] = ts_y0_ + ts_channel_pitch_ * centroid [channel]
  float ts_y0_{0};
  float ts_channel_pitch_{0};
  bool warned_missing_ts_{false};

  // name of collection for calo cluster inputs
  std::string cluster_coll_name_;
  // name of collection for electron outputs
  std::string ele_coll_name_;

  std::string cluster_coll_passname_;
  std::string cluster_collname_events_passname_;

  std::string prop_map_name_;
  // z [mm] of the ECal face
  double ecal_front_z_{0};
  TProfile2D* prop_mapx_{nullptr};
  TProfile2D* prop_mapy_{nullptr};
  std::vector<TF1*> fits_x_{};
  std::vector<TF1*> fits_y_{};

  // From:
  // Tools/python/HgcrocEmulator.py
  // ECal/python/digi.py
  // ECal/src/EcalRecProducer.cxx
  /* float gain_ = 320. / 0.1 / 1024;                                    //
   * mV/ADC */
  /* float mVtoMeV_ = 0.130 / (37000.0 * (0.162 / 1000.) * (1. / 0.1));  //
   * MeV/mV */
  /* std::vector<float> layerWeights = { */
  /*     1.675,  2.724,  4.398,  6.039,  7.696,  9.077,  9.630,  9.630,  9.630,
   */
  /*     9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,  9.630,
   */
  /*     9.630,  9.630,  9.630,  9.630,  9.630,  13.497, 17.364, 17.364, 17.364,
   */
  /*     17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 8.990}; */
  /* float secondOrderEnergyCorrection_ = 4000. / 4010.; */
  /* float mipSiEnergy_ = 0.130; */
  /* int hgc_compression_factor_ = 8; */

  // ClusterGeometry myGeo;
};
}  // namespace trigger

#endif /* TRIGECALCLUSTERPRODUCER_H */
