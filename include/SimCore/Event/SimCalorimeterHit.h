/**
 * @file SimCalorimeterHit.h
 * @brief Class which stores simulated calorimeter hit information
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_EVENT_SIMCALORIMETERHIT_H_
#define SIMCORE_EVENT_SIMCALORIMETERHIT_H_

// ROOT
#include "TObject.h"  //For ClassDef

// LDMX
#include "SimCore/Event/SimParticle.h"

namespace ldmx {

/**
 * @class SimCalorimeterHit
 * @brief Stores simulated calorimeter hit information
 *
 * @note
 * This class represents simulated hit information from a calorimeter detector.
 * It provides access to the cell ID, energy deposition, cell position and time.
 * Additionally, individual depositions or steps from MC particles are tabulated
 * as contributions stored in vectors.  Contribution information includes a
 * reference to the relevant SimParticle, the PDG code of the actual particle
 * which deposited energy (may be different from the actual SimParticle), the
 * time of the contribution and the energy deposition.
 */
class SimCalorimeterHit {
 public:
  /// name of the ecal sim collection, should match gdml
  static const std::string ECAL_COLLECTION;

  /// name of the hcal sim collection, should match gdml
  static const std::string HCAL_COLLECTION;

  /**
   * @class Contrib
   * @brief Information about a contribution to the hit in the associated cell
   */
  struct Contrib {
    /**
     * trackID of incident particle that is an ancestor of the contributor
     *
     * The incident ancestor is found in TrackMap::findIncident where the
     * ancestry is looped upwards until a particle is found that matches
     * the criteria.
     *      (1) particle will be saved to output file AND
     *      (2) particle originates in a region outside the CalorimeterRegion
     * If no particle is found matching these criteria, the primary particle
     * that is this trackID's ancestor is chosen.
     */
    int incidentID{-1};

    /// track ID of this contributor
    int trackID{-1};

    /// PDG ID of this contributor
    int pdgCode{0};

    /// Energy depostied by this contributor
    float edep{0};

    /// Time this contributor made the hit (global Geant4 time)
    float time{0};
  };

  /**
   * Class constructor.
   */
  SimCalorimeterHit();

  /**
   * Class destructor.
   */
  virtual ~SimCalorimeterHit();

  /**
   * Clear the data in the object.
   */
  void Clear();

  /**
   * Print out the object.
   */
  void Print() const;

  /**
   * Get the detector ID.
   * @return The detector ID.
   */
  int getID() const { return id_; }

  /**
   * Set the detector ID.
   * @id The detector ID.
   */
  void setID(const int id) { this->id_ = id; }

  /**
   * Get the energy deposition of the hit [MeV].
   * @return The energy deposition of the hit.
   */
  float getEdep() const { return edep_; }

  /**
   * Set the energy deposition of the hit [MeV].
   * @param edep The energy deposition of the hit.
   */
  void setEdep(const float edep) { this->edep_ = edep; }

  /**
   * Get the XYZ position of the hit [mm].
   * @return The XYZ position of the hit.
   */
  std::vector<float> getPosition() const { return {x_, y_, z_}; }

  /**
   * Get the XYZ pre-step position of the hit in the coordinate frame of the
   * sensitive volume [mm].
   * @return The local XYZ position of the hit.
   */
  std::vector<float> getPreStepPosition() const { return {preStepX_, preStepY_, preStepZ_}; }
  /**
   * Get the XYZ post-step position of the hit in the coordinate frame of the
   * sensitive volume [mm].
   * @return The XYZ position of the hit.
   */
  std::vector<float> getPostStepPosition() const { return {postStepX_, postStepY_, postStepZ_}; }
  /**
   * Set the XYZ position of the hit [mm].
   * @param x The X position.
   * @param y The Y position.
   * @param z The Z position.
   */
  void setPosition(const float x, const float y, const float z) {
    this->x_ = x;
    this->y_ = y;
    this->z_ = z;
  }
  /**
   * Set the XYZ pre-step position of the hit in the coordinate frame of the
   * sensitive volume [mm].
   * @param x The X position.
   * @param y The Y position.
   * @param z The Z position.
   */
  void setPreStepPosition(const float x, const float y, const float z) {
    preStepX_ = x;
    preStepY_ = y;
    preStepZ_ = z;
  }
  /**
   * Set the XYZ post-step position of the hit in the coordinate frame of the
   * sensitive volume  [mm].
   * @param x The X position.
   * @param y The Y position.
   * @param z The Z position.
   */
  void setPostStepPosition(const float x, const float y, const float z) {
    postStepX_ = x;
    postStepY_ = y;
    postStepZ_ = z;
  }

  /**
   * Set the physical path length for the interaction [mm].
   * @param length The physical path lenght
   */
  void setPathLength(const float length) {
    pathLength_ = length;
  }
  /**
   * Get the physical path length for the interaction [mm].
   * @return physical path length
   */
  float getPathLength() const { return pathLength_; }
  /**
   * Set global pre-step time of the hit [ns].
   * @param time The time before the step
   */
  void setPreStepTime(const float time) {
    preStepTime_ = time;
  }
  /**
   * Set global post-step time of the hit [ns].
   * @param time The time before the step
   */
  void setPostStepTime(const float time) {
    postStepTime_ = time;
  }

  /**
   * Set the velocity of the track [mm/ns].
   * @param velocity The track velocity
   */
  void setVelocity(float velocity) {
    velocity_ = velocity;
  }


  /**
   * Get the global time of the hit [ns].
   * @return The global time of the hit.
   */
  float getTime() const { return time_; }
  /**
   * Get the pre-step time of the hit [ns].
   * @return The global time of the hit before the interaction.
   */
  float getPreStepTime() const { return preStepTime_; }
  /**
   * Get the post-step time of the hit [ns].
   * @return The global time of the hit after the interaction.
   */
  float getPostStepTime() const { return postStepTime_; }

  /**
   * Get the track velocity of the hit [mm/ns].
   * @return Thetrack velocity of the hit.
   */
  float getVelocity() const {return velocity_;}


  /**
   * Set the time of the hit [ns].
   * @param time The time of the hit.
   */
  void setTime(const float time) { this->time_ = time; }

  /**
   * Get the number of hit contributions.
   * @return The number of hit contributions.
   */
  unsigned getNumberOfContribs() const { return nContribs_; }

  /**
   * Add a hit contribution from a SimParticle.
   * @param incidentID the Geant4 track ID for the particle's parent incident on
   * the Calorimeter region
   * @param trackID the Geant4 track ID for the particle
   * @param pdgCode The PDG code of the actual track.
   * @param edep The energy deposition of the hit [MeV].
   * @param time The time of the hit [ns].
   */
  void addContrib(int incidentID, int trackID, int pdgCode, float edep,
                  float time);

  /**
   * Get a hit contribution by index.
   * @param i The index of the hit contribution.
   * @return The hit contribution at the index.
   */
  Contrib getContrib(int i) const;

  /**
   * Find the index of a hit contribution from a SimParticle and PDG code.
   * @param trackID the track ID of the particle causing the hit
   * @param pdgCode The PDG code of the contribution.
   * @return The index of the contribution or -1 if none exists.
   */
  int findContribIndex(int trackID, int pdgCode) const;

  /**
   * Update an existing hit contribution by incrementing its edep and setting
   * the time if the new time is less than the old one.
   * @param i The index of the contribution.
   * @param edep The additional energy contribution [MeV].
   * @param time The time of the contribution [ns].
   */
  void updateContrib(int i, float edep, float time);

  /**
   * Sort by time of hit
   */
  bool operator<(const SimCalorimeterHit &rhs) const {
    return this->getTime() < rhs.getTime();
  }

 private:

  /**
   * Member variables used in all calorimeter types
   */
  /**
   * The detector ID.
   */
  int id_{0};

  /**
   * The energy deposition.
   */
  float edep_{0};

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
   * The global time of the hit.
   */
  float time_{0};

  /**
   * The list of track IDs contributing to the hit.
   */
  std::vector<int> trackIDContribs_;

  /**
   * The list of incident IDs contributing to the hit
   */
  std::vector<int> incidentIDContribs_;

  /**
   * The list of PDG codes contributing to the hit.
   */
  std::vector<int> pdgCodeContribs_;

  /**
   * The list of energy depositions contributing to the hit.
   */
  std::vector<float> edepContribs_;

  /**
   * The list of times contributing to the hit.
   */
  std::vector<float> timeContribs_;

  /**
   * The number of hit contributions.
   */
  unsigned nContribs_{0};

  /*
   * Parameters used only for hits corresponding to a single interactions
   * (currently Hcal and TS).
   */

  /**
   * The true path length [mm]. Can in general differ from the distance between
   * the pre and post step position.
   */
  float pathLength_{-1};


  /**
   * The X, Y, and Z positions [mm] before the interaction in the coordinate
   * frame of the sensitive volume.
   */
  float preStepX_{0};
  float preStepY_{0};
  float preStepZ_{0};
  /**
   * The global time before the interaction [ns]
   */
  float preStepTime_{0};
  /**
   * The X, Y, and Z positions [mm] after the interaction in the coordinate
   * frame of the sensitive volume.
   */
  float postStepX_{0};
  float postStepY_{0};
  float postStepZ_{0};
  /**
   * The global time after the interaction [ns]
   */
  float postStepTime_{0};

  /**
   * The track velocity [mm/ns].
   */
  float velocity_{-1};



  /**
   * ROOT class definition.
   */
  ClassDef(SimCalorimeterHit, 4)
};
} // namespace ldmx

#endif
