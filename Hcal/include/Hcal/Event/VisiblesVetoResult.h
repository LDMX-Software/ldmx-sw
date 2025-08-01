/*
 *@file VisiblesVetoResult.h
 *@brief Class used to encapsulate the results obstained
         from VisiblesVetoProcessor
 *@author Tyler Horoho, University of Virginia
 */

#ifndef EVENT_VISIBLESVETORESULT_H_
#define EVENT_VISIBLESVETORESULT_H_

#include <iostream>

//   ROOT   //
#include <TObject.h>

namespace ldmx {

class VisiblesVetoResult {
 public:
  /** Constructor */
  VisiblesVetoResult();

  /** Destructor */
  virtual ~VisiblesVetoResult();

  void setVariables(int n_layers_hit, double x_std, double y_std, double z_std,
                    double x_mean, double y_mean, double r_mean, int iso_hits,
                    double iso_energy, int n_readout_hits, double summed_det,
                    double r_mean_from_photon_track);

  void Clear();

  void Print() const;

  bool passesVeto() const { return passes_veto_; }

  double getDisc() const { return disc_value_; }

  int getNLayersHit() const { return n_layers_hit_; }

  double getXStd() const { return x_std_; }

  double getYStd() const { return y_std_; }

  double getZStd() const { return z_std_; }

  double getXMean() const { return x_mean_; }

  double getYMean() const { return y_mean_; }

  double getRMean() const { return r_mean_; }

  int getIsoHits() const { return iso_hits_; }

  double getIsoEnergy() const { return iso_energy_; }

  int getNReadoutHits() const { return n_readout_hits_; }

  double getSummedDet() const { return summed_det_; }

  double getDistFromPhotonTrack() const { return r_mean_from_photon_track_; }

 private:
  bool passes_veto_{false};

  int n_layers_hit_{0};
  double x_std_{0};
  double y_std_{0};
  double z_std_{0};
  double x_mean_{0};
  double y_mean_{0};
  double r_mean_{0};
  int iso_hits_{0};
  double iso_energy_{0};
  int n_readout_hits_{0};
  double summed_det_{0};
  double r_mean_from_photon_track_{0};

  double disc_value_{0};

  ClassDef(VisiblesVetoResult, 1);
};
}  // namespace ldmx

#endif
