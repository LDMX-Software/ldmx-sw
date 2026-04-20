/**
 * @file HepMCEvent.h
 * @brief Class defining a HepMC event with a list of particles
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_HEPMCEVENT_H_
#define SIMCORE_HEPMCEVENT_H_

// LDMX
#include "Framework/Exception/Exception.h"
#include "SimCore/HepMC/HepMCParticle.h"

// HepMC3
#include "HepMC3/GenEvent.h"
#include "HepMC3/GenVertex.h"

// Geant4
#include "globals.hh"

// STL
#include <iostream>
#include <memory>
#include <vector>

namespace simcore::hepmc {

/**
 * @class HepMCEvent
 * @brief Wrapper for HepMC3::GenEvent with convenience methods
 *
 * @note
 * Detailed information on the HepMC3 format is provided here:
 * <a href="https://doi.org/10.1016/j.cpc.2020.107310">HepMC3: A modernized
 * Monte Carlo event structure library</a>.
 */
class HepMCEvent {
 public:
  /**
   * Class constructor.
   * @param event Shared pointer to HepMC3 GenEvent
   */
  HepMCEvent(std::shared_ptr<HepMC3::GenEvent> event);

  /**
   * Class destructor.
   */
  virtual ~HepMCEvent() = default;

  /**
   * Get the number of particles in the event.
   * @return The number of particles in event.
   */
  int getNumParticles() const;

  /**
   * Get the event weight.
   * @return The event weight.
   */
  double getEventWeight() const;

  /**
   * Get the vertex location (in mm, as expected by Geant4).
   * @return Array double[3] with x, y, z ordering
   */
  const double* getVertex() const;

  /**
   * Get the vertex time (in ns, as expected by Geant4).
   * @return time of primary vertex.
   */
  double getVertexTime() const;

  /**
   * Get the list of final state particles in the event.
   * These are particles that should be tracked by Geant4.
   * @return The list of final state particles in the event.
   */
  const std::vector<std::unique_ptr<HepMCParticle>>& getParticles() const;

  /**
   * Get the underlying HepMC3 GenEvent
   * @return Shared pointer to the HepMC3 GenEvent
   */
  std::shared_ptr<HepMC3::GenEvent> getGenEvent() const;

 private:
  /**
   * The underlying HepMC3 GenEvent
   */
  std::shared_ptr<HepMC3::GenEvent> event_;

  /**
   * Vertex location (in mm)
   */
  mutable double vtx_[3];

  /**
   * Vertex time (in ns)
   */
  mutable double vtxt_{0.};

  /**
   * The list of final state particles to be tracked
   */
  mutable std::vector<std::unique_ptr<HepMCParticle>> particles_;

  /**
   * Flag to indicate if particles have been extracted
   */
  mutable bool particles_extracted_{false};

  /**
   * Extract final state particles from the HepMC event
   */
  void extractParticles() const;
};

}  // namespace simcore::hepmc

#endif
