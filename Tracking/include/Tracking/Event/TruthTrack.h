#pragma once

#include <TObject.h>

#include <vector>

namespace ldmx {

class TruthTrack {
 public:
  /** default constructor
   */

  TruthTrack() = default;

  ~TruthTrack() = default;
  /**
   *
   * Use the vertex position of the SimParticle to extract
   * (x, y, z, px, py, pz, q) and create a track seed.
   *
   * @param particle The SimParticle to make a seed from.
   */
  // TruthTrack(const ldmx::SimParticle &particle);

  void Print() const {};

  /**
   * Use the scoring plane hit at the target to extract
   * (x, y, z, px, py, pz) and create a track seed. In this case, the
   * SimParticle is used to extract the charge of the particle.
   *
   * @param particle The SimParticle to extract the charge from.
   * @param hit The SimTrackerHit used to create the seed.
   */
  // TruthTrack(const ldmx::SimParticle &particle,
  //           const ldmx::SimTrackerHit &hit) {};

  /**
   * Create a truth track from the given position, momentum and charge.
   *
   * @param pos The position at which the particle was created.
   * @param p The momentum of the particle at the point of creation.
   * @param charge The charge of the particle.
   */

  // TruthTrack(const std::vector<double> &pos_vec,
  //           const std::vector<double> &p_vec, int charge){};

  /*
    Acts::Vector3 pos{pos_vec.data()};
    Acts::Vector3 mom{p_vec.data()};
    double time{0.};

    // Rotate the position and momentum into the ACTS frame.
    pos = tracking::sim::utils::Ldmx2Acts(pos);
    mom = tracking::sim::utils::Ldmx2Acts(mom);

    // Get the charge of the particle.
    // TODO: Add function that uses the PDG ID to calculate this.
    double q{charge * Acts::UnitConstants::e};

    // Transform the position, momentum and charge to free parameters.
    auto free_params{tracking::sim::utils::toFreeParameters(pos, mom, q)};

    // Create a line surface at the perigee.  The perigee position is extracted
    // from a particle's vertex or the particle's position at a specific
    // scoring plane.
    auto gen_surface{Acts::Surface::makeShared<Acts::PerigeeSurface>(
        Acts::Vector3(free_params[Acts::eFreePos0],
  free_params[Acts::eFreePos1], free_params[Acts::eFreePos2]))};

    // Transform the parameters to local positions on the perigee surface.
    auto bound_params{Acts::detail::transformFreeToBoundParameters(
        free_params, *gen_surface, gctx_)
          .value()};

    // Create the seed track object.
    auto seed_track = ldmx::Track();
    seed_track.setPerigeeLocation(free_params[Acts::eFreePos0],
                                  free_params[Acts::eFreePos1],
                                  free_params[Acts::eFreePos2]);


    seed_track.setPerigeeParameters(
        tracking::sim::utils::convertActsToLdmxPars(bound_params));


  };
  */

  void setTrackID(int trackid) { trackID_ = trackid; };
  int getTrackID() const { return trackID_; };

  void setPdgID(int pdgID) { pdgID_ = pdgID; };
  int getPdgID() const { return pdgID_; };

  void setNhits(int nHits) { nHits_ = nHits; };
  int getNhits() const { return nHits_; }

  // in units of e
  int q() const { return perigee_pars_[4] > 0 ? 1 : -1; }

  // Vector representation
  void setPerigeeParameters(const std::vector<double>& par) {
    perigee_pars_ = par;
  }
  std::vector<double> getPerigeeParameters() const { return perigee_pars_; }

  void setPerigeeLocation(const std::vector<double>& perigee) {
    perigee_ = perigee;
  }

  void setPerigeeLocation(const double& x, const double& y, const double& z) {
    perigee_[0] = x;
    perigee_[1] = y;
    perigee_[2] = z;
  }

  void setMomentum(const double& px, const double& py, const double& pz) {
    momentum_[0] = px;
    momentum_[1] = py;
    momentum_[2] = pz;
  }

  void setPosition(const double& x, const double& y, const double& z) {
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
  }

  std::vector<double> getPerigeeLocation() const { return perigee_; };
  double getPerigeeX() const { return perigee_[0]; };
  double getPerigeeY() const { return perigee_[1]; };
  double getPerigeeZ() const { return perigee_[2]; };

  std::vector<double> getMomentum() const { return momentum_; };
  std::vector<double> getPosition() const { return position_; };

  // getters -- TODO use an enum instead

  double getD0() const { return perigee_pars_[0]; };
  double getZ0() const { return perigee_pars_[1]; };
  double getPhi() const { return perigee_pars_[2]; };
  double getTheta() const { return perigee_pars_[3]; };
  double getQoP() const { return perigee_pars_[4]; };
  double getT() const { return perigee_pars_[5]; };

  friend std::ostream& operator<<(std::ostream& output, const TruthTrack& trk);

 private:
  // 6 elements
  // d0 / z0 / phi / theta / qop / t
  std::vector<double> perigee_pars_{0., 0., 0., 0., 0., 0.};

  // The perigee location
  std::vector<double> perigee_{0., 0., 0.};

  // The 3-momentum at the perigee
  std::vector<double> momentum_{0., 0., 0.};

  // The 3-position at the perigee
  std::vector<double> position_{0., 0., 0.};

  // N hits
  int nHits_{0};

  // ID of the matched particle in the SimParticles map
  int trackID_{-1};

  // pdgID
  int pdgID_{0};

  /// Class declaration needed by the ROOT dictionary.
  ClassDef(TruthTrack, 2);

};  // TruthTrack

}  // namespace ldmx
