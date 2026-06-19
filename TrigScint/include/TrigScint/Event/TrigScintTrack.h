
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
  TrigScintTrack() {};

  /**
   * Class destructor.
   */
  virtual ~TrigScintTrack() {};

  /**
   * Print a description of this object.
   */
  friend std::ostream& operator<<(std::ostream& o, const TrigScintTrack& d);

  /**
   * Reset the TrigScintTrack object.
   */
  void clear();  // Option_t *option);

  /**
   * Get the detector ID centroid of the track.
   * @return The detector ID centroid of the track.
   `             */
  float getCentroid() const { return centroid_; };

  /**
   * Get the x centroid of the track.
   * @return The x centroid of the track in units of bars.
   */
  float getCentroidX() const { return centroid_x_; };

  /**
   * Get the y centroid of the track.
   * @return The y centroid of the track in units of bars.
   */
  float getCentroidY() const { return centroid_y_; };

  /**
   * Get the z centroid of the track.
   * @return The z centroid of the track.
   */
  float getCentroidZ() const { return centroid_z_; };

  /**
   * Get the detector ID residual of the track.
   * @return The detector ID residual of the track.
   */
  float getResidual() const { return residual_; };

  /**
   * Get the x residual of the track.
   * @return The x residual of the track.
   */
  float getResidualX() const { return residual_x_; };

  /**
   * Get the y residual of the track.
   * @return The y residual of the track.
   */
  float getResidualY() const { return residual_y_; };

  /**
   * Get the z residual of the track.
   * @return The z residual of the track.
   */
  float getResidualZ() const { return residual_z_; };

  /**
   * Get the (average) pe of the track.
   * @return The cluster pe count averaged over the track.
   */
  float getPE() const { return pe_; };

  /**
   * Get the number of clusters forming the track.
   * @return The number of clusters in the track.
   */
  int getNclusters() const { return n_clusters_; };

  /**
   * Get the x coordinate of the track.
   * @return The x coordinate of the track in mm [mm].
   */
  float getX() const { return x_; };

  /**
   * Get the y coordinate of the track.
   * @return The y coordinate of the track [mm].
   */
  float getY() const { return y_; };

  /**
   * Get the uncertainty on the x coordinate of the track.
   * @return The uncertainty on the x coordinate of the track in mm [mm].
   */
  float getSigmaX() const { return sx_; };

  /**
   * Get the uncertainty on the y coordinate of the track.
   * @return The uncertainty on the y coordinate of the track in mm [mm].
   */
  float getSigmaY() const { return sy_; };

  /**
   * Get the cluster constituents of the track.
   * @return The list of track constituents.
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
  float getBeamEfrac() const { return beam_efrac_; }

  // setters

  /** Set beam energy fraction of hit. */
  void setBeamEfrac(float e) { beam_efrac_ = e; }

  /**
   * Set the detector ID centroid of the track.
   */
  void setCentroid(float centroid) { centroid_ = centroid; };

  /**
   * Set the x centroid of the track.
   */
  void setCentroidX(float centroid) { centroid_x_ = centroid; };

  /**
   * Set the y centroid of the track.
   */
  void setCentroidY(float centroid) { centroid_y_ = centroid; };

  /**
   * Set the z centroid of the track.
   */
  void setCentroidZ(float centroid) { centroid_z_ = centroid; };

  /**
   * Set the detector ID residual of the track.
   */
  void setResidual(float resid) { residual_ = resid; };

  /**
   * Set the x residual of the track.
   */
  void setResidualX(float resid) { residual_x_ = resid; };

  /**
   * Set the y residual of the track.
   */
  void setResidualY(float resid) { residual_y_ = resid; };

  /**
   * Set the z residual of the track.
   */
  void setResidualZ(float resid) { residual_z_ = resid; };

  /**
   * Set the average cluster pe of  the track.
   */
  void setPE(float pe) { pe_ = pe; };

  /**
   * Set the number of clusters forming the track.
   */
  void setNclusters(uint nCl) { n_clusters_ = nCl; };

  /**
   * Add a cluster to the list of track constituents.
   */
  void addConstituent(TrigScintCluster cl) { constituents_.push_back(cl); };

  /**
   * Set the position of the track [mm].
   * @param x The X position.
   * @param y The Y position.
   */
  void setPosition(const float x, const float y) {
    x_ = x;
    y_ = y;
  };

  /**
   * Set the uncertainty on the position of the track [mm].
   * @param x The X position uncertainty.
   * @param y The Y position uncertainty.
   */
  void setSigmaXY(const float sx, const float sy) {
    sx_ = sx;
    sy_ = sy;
  }

  /**
   * Set the uncertainty on the position of the track [mm].
   * @param x The X position uncertainty.
   */
  void setSigmaX(const float sx) { sx_ = sx; }

  /**
   * Set the uncertainty on the position of the track [mm].
   * @param y The Y position uncertainty.
   */
  void setSigmaY(const float sy) { sy_ = sy; }

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
  bool operator<(const TrigScintTrack& rhs) const {
    return this->getResidual() < rhs.getResidual();
  }

 private:
  /**
   * The detector centroid.
   */
  float centroid_{-1};

  /**
   * The detector x coordinate centroid.
   */
  float centroid_x_{-1};

  /**
   * The detector y coordinate centroid.
   */
  float centroid_y_{-1};

  /**
   * The detector z coordinate centroid.
   */
  float centroid_z_{-99999};

  /**
   * The detector residual.
   */
  float residual_{0};

  /**
   * The x coordinate residual.
   */
  float residual_x_{0};

  /**
   * The y coordinate residual.
   */
  float residual_y_{0};

  /**
   * The z coordinate residual.
   */
  float residual_z_{0};

  /**
   * The number of clusters forming the track.
   */
  int n_clusters_{0};

  /**
   * The list of clusters constituting the track.
   */
  std::vector<ldmx::TrigScintCluster> constituents_{0};

  /**
   * The fraction of the energy deposited in the track constituents
   * that was deposited by a beam electron.
   * WARNING: this is a "truth" variable and will never be
   * measureable in actual data.
   */
  float beam_efrac_{0.};

  /**
   * The average pe count of the clusters making up the track.
   */
  float pe_{0.};

  /**
   * The X position.
   */
  float x_{-99999.};

  /**
   * The Y position.
   */
  float y_{-99999.};

  /**
   * The uncertainty on the X position.
   */
  float sx_{-9999.};

  /**
   * The uncertainty on the Y position.
   */
  float sy_{-9999.};

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
   * The Z position.
   */
  float z_{0};

  /**
   * The ROOT class definition.
   */
  ClassDef(TrigScintTrack, 4);

};  // TrigScintTrack

}  // namespace ldmx

#endif  // TRIGSCINT_EVENT_TRIGSCINTTRACK_H_
