#pragma once

#include "Tracking/Digitization/ChargeCarrier.h"
#include "Tracking/Digitization/SiElectrodeDataCollection.h"
#include "Acts/Definitions/Algebra.hpp"
#include <map>

namespace tracking {
  namespace digitization {

class CDFSiSensorSim {

  // Static parameters - not intended to be user modifiable
  
  //10% of pitch or depleted thickness
  //static double deposition_granularity_   = 0.10;
  //static double distance_error_threshold_ = 0.001;
  
  
  //Field
  //SiSensor _sensor = null;

  CDFSiSensorSim();

  /*

  void setTrapping(double trapping) {trapping_ = trapping;}

  // Get charge map on electrodes
  // TODO::Add check that the carrier exists in the internal map
  SiElectrodeDataCollection getReadoutData(ChargeCarrier carrier) {
    return readout_data.get(carrier);
  }

  // Simulate charge deposition
  std::map<ChargeCarrier, SiElectrodeDataCollection> computeElectrodeData();
  */
  
private:

  std::map<ChargeCarrier,Acts::Vector3> _drift_direction;
  std::map<ChargeCarrier,SiElectrodeDataCollection> _sense_data;
  std::map<ChargeCarrier,SiElectrodeDataCollection> _readout_data;

  // Simple simulation of charge trapping, this is a temporary kludge.
  // Charge collection efficiency with linear drift distance dependence.
  // Input is fraction lost per 100um drift: 0.2 is typical for 1E15 NEQ.
  // FIXME: should be calculated from properties of DopedSilicon (radiation dose)

  double trapping_{0.0};

  bool debug_{false};
  
  
};


  } // digitization
}// tracking
