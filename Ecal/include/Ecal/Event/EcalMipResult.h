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
    EcalMipResult();

    /** Destructor */
    virtual ~EcalMipResult();

    void Print() const;

    void Clear();


    // Large Setter for all mip elements
    void setVariables(int nStraightTracks, int nLinregTracks,
                      int firstNearPhLayer, int nNearPhHits,
                      int photonTerritoryHits, float epAng, float epAngAtTarget,
                      float epSep, float epDot, float epDotAtTarget);
    // Getters
    /// Number of straight tracks found
    int getNStraightTracks() const { return nStraightTracks_; }

    /// Number of linear-regression tracks found
    int getNLinRegTracks() const { return nLinregTracks_; }

    int getFirstNearPhLayer() const { return firstNearPhLayer_; }
    int getNNearPhHits() const { return nNearPhHits_; }
    int getPhotonTerritoryHits() const { return photonTerritoryHits_; }
    float getEPAng() const { return epAng_; }
    float getEPAngAtTarget() const { return epAngAtTarget_; }
    float getEPSep() const { return epSep_; }
    float getEPDot() const { return epDot_; }
    float getEPDotAtTarget() const { return epDotAtTarget_; }


    private:
        /// Number of "straight" tracks found in the event
        int nStraightTracks_{0};
        /// Number of "linreg" tracks found in the event
        int nLinregTracks_{0};
        /// Earliest ECal layer in which a hit is found near the projected photon
        /// trajectory
        int firstNearPhLayer_{0};
        /// Number of hits near the photon trajectory
        int nNearPhHits_{0};
        /// Number of hits in the photon territory
        int photonTerritoryHits_{0};
        /// Angular separation between the projected photon and electron trajectories
        /// as projected at the ECAL
        float epAng_{0};
        /// Angular separation between the projected photon and electron trajectories
        /// as projected at the target
        float epAngAtTarget_{0};

        /// Distance between the projected photon and electron trajectories at the
        /// ECal face
        float epSep_{0};
        /// Dot product of the photon and electron momenta unit vectors as at ECAL
        float epDot_{0};
        /// Dot product of the photon and electron momenta unit vectors as at Target
        float epDotAtTarget_{0};

    ClassDef(EcalMipResult, 1);
};

}  // namespace ldmx

#endif