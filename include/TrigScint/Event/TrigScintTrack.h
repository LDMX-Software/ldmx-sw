
#ifndef TRIGSCINT_EVENT_TRIGSCINTTRACK_H_
#define TRIGSCINT_EVENT_TRIGSCINTTRACK_H_

// ROOT
#include "TObject.h"  //For ClassDef

// STL
#include <iostream>

// ldmx
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TrigScintCluster.h"

namespace ldmx {

/**
 * @class TrigScintTrack
 * @brief Represents a track of trigger scintillator clusters
 */
class TrigScintTrack {
 public:
  /**
   * Class constructor.
   */
  TrigScintTrack(){};

  /**
   * Class destructor.
   */
  virtual ~TrigScintTrack(){};

  /**
   * Print a description of this object.
   */
  void Print() const;  // Option_t *option) const;

  /**
   * Reset the TrigScintTrack object.
   */
  void Clear();  // Option_t *option);

  /**
   * Get the detector ID centroid of the track.
   * @return The detector ID centroid of the track.
   `             */
  float getCentroid() const { return centroid_; };

  /**
   * Get the x centroid of the track.
   * @return The x centroid of the track.
   */
  float getCentroidX() const { return centroidX_; };

  /**
   * Get the y centroid of the track.
   * @return The y centroid of the track.
   */
  float getCentroidY() const { return centroidY_; };

  /**
   * Get the z centroid of the track.
   * @return The z centroid of the track.
   */
  float getCentroidZ() const { return centroidZ_; };

  /**
   * Get the detector ID residual of the track.
   * @return The detector ID residual of the track.
   */
  float getResidual() const { return residual_; };

  /**
   * Get the x residual of the track.
   * @return The x residual of the track.
   */
  float getResidualX() const { return residualX_; };

  /**
   * Get the y residual of the track.
   * @return The y residual of the track.
   */
  float getResidualY() const { return residualY_; };

  /**
   * Get the z residual of the track.
   * @return The z residual of the track.
   */
  float getResidualZ() const { return residualZ_; };

  /**
   * Get the number of clusters forming the track.
   * @return The number of clusters in the track.
   */
  int getNclusters() const { return nClusters_; };

  /**
   * Get the XYZ momentum of the particle at the position at which
   * the track took place [MeV].
   * @return The momentum of the particle.
   */
  std::vector<ldmx::TrigScintCluster> getConstituents() const {
    return constituents_;
  };

  /**
   * Get the XYZ momentum of the particle at the position at which
   * the track took place [MeV].
   * @return The momentum of the particle.
   */
  std::vector<double> getMomentum() const { return {px_, py_, pz_}; };

  /** Get beam energy fraction of hit. */
  float getBeamEfrac() const { return beamEfrac_; }

  // setters

  /** Set beam energy fraction of hit. */
  void setBeamEfrac(float e) { beamEfrac_ = e; }

  /**
   * Set the detector ID centroid of the track.
   */
  void setCentroid(float centroid) { centroid_ = centroid; };

  /**
   * Set the x centroid of the track.
   */
  void setCentroidX(float centroid) { centroidX_ = centroid; };

  /**
   * Set the y centroid of the track.
   */
  void setCentroidY(float centroid) { centroidY_ = centroid; };

  /**
   * Set the z centroid of the track.
   */
  void setCentroidZ(float centroid) { centroidZ_ = centroid; };

  /**
   * Set the detector ID residual of the track.
   */
  void setResidual(float resid) { residual_ = resid; };

  /**
   * Set the x residual of the track.
   */
  void setResidualX(float resid) { residualX_ = resid; };

  /**
   * Set the y residual of the track.
   */
  void setResidualY(float resid) { residualY_ = resid; };

  /**
   * Set the z residual of the track.
   */
  void setResidualZ(float resid) { residualZ_ = resid; };

  /**
   * Set the number of clusters forming the track.
   */
  void setNclusters(uint nCl) { nClusters_ = nCl; };

  /**
   * Add a cluster to the list of track constituents.
   */
  void addConstituent(TrigScintCluster cl) { constituents_.push_back(cl); };

  /**
   * Set the position of the track [mm].
   * @param x The X position.
   * @param y The Y position.
   * @param z The Z position.
   */
  void setPosition(const float x, const float y, const float z);

  /**
   * Set the momentum of the particle at the position at which
   * the track took place [GeV].
   * @param px The X momentum.
   * @param py The Y momentum.
   * @param pz The Z momentum.
   */
  void setMomentum(const float px, const float py, const float pz);

  /**
   * Sort by track residual
   */
  bool operator<(const TrigScintTrack &rhs) const {
    return this->getResidual() < rhs.getResidual();
  }

 private:
  /**
   * The detector centroid.
   */
  float centroid_{0};

  /**
   * The x coordinate centroid.
   */
  float centroidX_{0};

  /**
   * The y coordinate centroid.
   */
  float centroidY_{0};

  /**
   * The z coordinate centroid.
   */
  float centroidZ_{0};

  /**
   * The detector residual.
   */
  float residual_{0};

  /**
   * The x coordinate residual.
   */
  float residualX_{0};

  /**
   * The y coordinate residual.
   */
  float residualY_{0};

  /**
   * The z coordinate residual.
   */
  float residualZ_{0};

  /**
   * The number of clusters forming the track.
   */
  int nClusters_{0};

  /**
   * The list of clusters constituting the track.
   */
  std::vector<ldmx::TrigScintCluster> constituents_{0};

  float beamEfrac_{0.};

  // these below here i don't think i'll use.

  /**
   * The X momentum.
   */
  float px_{0};

  /**
   * The Y momentum.
   */
  float py_{0};

  /**
   * The Z momentum.
   */
  float pz_{0};

  /**
   * The X position.
   */
  float x_{0};

  /**
   * The Y position.
   */
  float y_{0};

  /**
   * The Z position.
   */
  float z_{0};

  /**
   * The ROOT class definition.
   */
  ClassDef(TrigScintTrack, 1);

};  // TrigScintTrack

} // namespace ldmx

#endif  // TRIGSCINT_EVENT_TRIGSCINTTRACK_H_
