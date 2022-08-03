/**
 * @file LHEEvent.h
 * @brief Class defining an LHE event with a list of particles and information
 * from the header block
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_LHEEVENT_H_
#define SIMCORE_LHEEVENT_H_

// LDMX
#include "SimCore/LHE/LHEParticle.h"

// STL
#include <vector>

namespace simcore::lhe {

/**
 * @class LHEEvent
 * @brief LHE event with a list of particles and information from the header
 * block
 *
 * @note
 * Detailed information on the Les Houches Event (LHE) format is provided here:
 * <a href="https://arxiv.org/abs/hep-ph/0609017">A standard format for Les
 * Houches Event Files</a>.
 */
class LHEEvent {
 public:
  /**
   * Class constructor.
   * @param data The string data of the event header.
   */
  LHEEvent(std::string& data);

  /**
   * Class destructor.
   */
  virtual ~LHEEvent();

  /**
   * Get the number of particles (NUP) in the event.
   * @return The number of particles in event.
   */
  int getNUP() const;

  /**
   * Get the ID of the physics process (IDRUP).
   * @return The ID of the physics process.
   */
  int getIDPRUP() const;

  /**
   * Get the event weight (XWGTUP).
   * @return The event weight.
   */
  double getXWGTUP() const;

  /**
   * Get the scale Q of parton distributions (SCALUP).
   * @return The scale Q of parton distributions.
   */
  double getSCALUP() const;

  /**
   * Get the value of the QED coupling (AQEDUP).
   * @return The value of the QED coupling.
   */
  double getAQEDUP() const;

  /**
   * Get the value of the QED coupling (AQCDUP).
   * @return The value of the QED coupling.
   */
  double getAQCDUP() const;

  /**
   * Set the vertex location (careful to match units as expected!)
   */
  void setVertex(double x, double y, double z);

  /**
   * Parse the vertex from a line of the form "#vertex [x] [y] [z]"
   */
  void setVertex(const std::string& line);

  /**
   * Get the vertex location (careful to match units as expected!)
   * @return Array double[3] with x,y,z ordering
   */
  const double* getVertex() const;

  /**
   * Get the vertex time
   * @return time of primary particle creation.
   */
  const double getVertexTime() const;

  /**
   * Add a particle to the event.
   * @particle The particle to add.
   */
  void addParticle(LHEParticle* particle);

  /**
   * Get the list of particles in the event.
   * @return The list of particles in the event.
   */
  const std::vector<LHEParticle*>& getParticles();

 private:
  /**
   * Number of particles.
   */
  int nup_;

  /**
   * The physics process ID.
   */
  int idprup_;

  /**
   * The event weight.
   */
  double xwgtup_;

  /**
   * Scale Q of parton distributions.
   */
  double scalup_;

  /**
   * QCD coupling value.
   */
  double aqedup_;

  /**
   * QCD coupling value.
   */
  double aqcdup_;

  /**
   * Vertex location
   */
  double vtx_[3];

  /**
   * Vertex time
   */
  double vtxt_{0.};

  /**
   * The list of particles.
   */
  std::vector<LHEParticle*> particles_;
};

}  // namespace simcore

#endif
