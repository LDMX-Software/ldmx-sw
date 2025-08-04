/**
 * @file EcalMipResult.h
 * @brief Class used to encapsulate the results obtained from
 *        EcalMipTrackingProcessor.
 * @author Jihoon Yoo, Tamas Almos Vami (UCSB)
 */

#ifndef EVENT_ECALMIPRESULT_H_
#define EVENT_ECALMIPRESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <array>
#include <iostream>
#include <map>

//----------//
//   ROOT   //
//----------//

#include <TObject.h>  // For ClassDef
namespace ldmx {

class EcalMipResult {
 public:
  /** Constructor */
  EcalMipResult() = default;

  /** Destructor */
  virtual ~EcalMipResult();

  friend std::ostream &operator<<(std::ostream &o, const EcalMipResult &d);

  void Clear();

  // Large Setter for all mip elements
  void setVariables(int n_straight_tracks, int n_linreg_tracks,
                    int first_near_ph_layer, int n_near_ph_hits,
                    int photon_territory_hits);
  // Getters
  /// Number of straight tracks found
  int getNStraightTracks() const { return n_straight_tracks_; }

  /// Number of linear-regression tracks found
  int getNLinRegTracks() const { return n_linreg_tracks_; }

  int getFirstNearPhLayer() const { return first_near_ph_layer_; }
  int getNNearPhHits() const { return n_near_ph_hits_; }
  int getPhotonTerritoryHits() const { return photon_territory_hits_; }

 private:
  /// Number of "straight" tracks found in the event
  int n_straight_tracks_{0};
  /// Number of "linreg" tracks found in the event
  int n_linreg_tracks_{0};
  /// Earliest ECal layer in which a hit is found near the projected photon
  /// trajectory
  int first_near_ph_layer_{0};
  /// Number of hits near the photon trajectory
  int n_near_ph_hits_{0};
  /// Number of hits in the photon territory
  int photon_territory_hits_{0};
  /// Angular separation between the projected photon and electron trajectories
  /// as projected at the ECAL

  ClassDef(EcalMipResult, 1);
};

}  // namespace ldmx

#endif