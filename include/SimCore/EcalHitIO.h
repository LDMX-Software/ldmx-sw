
#ifndef SIMCORE_ECALHITIO_H_
#define SIMCORE_ECALHITIO_H_

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "SimCore/ConditionsInterface.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/G4CalorimeterHit.h"
#include "SimCore/Persist/SimParticleBuilder.h"

// STL
#include <utility>
#include <vector>

namespace simcore {

/**
 * @class EcalHitIO
 * @brief Provides hit readout for simulated ECal detector
 *
 * @note
 * This class uses the EcalHexReadout to transform the G4CalorimeterHit hits
 * collection into a collection of SimCalorimeterHit objects assigned to
 * hexagonal cells by their ID and positions.  Energy depositions in the same
 * cells are combined together into single hits.
 *
 * @par
 * It can be run in three modes:
 * <ul>
 * <li>Compressed hit contributions combined by matching SimParticle and PDG
 * code (default mode) <li>Full hit contributions with one record per step (when
 * compressHitContribs_ is false) <li>No hit contribution information where
 * energy is combined but vectors are not filled (when enableHitContribs_ is
 * false)
 * </ul>
 */
class EcalHitIO {
 public:
  /**
   * Layer-cell pair.
   */
  typedef std::pair<int, int> LayerCellPair;

 public:
  /**
   * Class constructor.
   */
  EcalHitIO(ConditionsInterface& ci) : conditionsIntf_(ci) {}

  /**
   * Class destructor.
   */
  ~EcalHitIO() {}

  /**
   * Configure the EcalHitIO using the passed parameters
   */
  void configure(const framework::config::Parameters& ps);

  /**
   * Write out a Geant4 hits collection to the provided ROOT array.
   * @param hc The input hits collection.
   * @param outputColl The output collection in ROOT.
   */
  void writeHitsCollection(G4CalorimeterHitsCollection* hc,
                           std::vector<ldmx::SimCalorimeterHit>& outputColl);

  /**
   * Set whether hit contributions should be enabled for the output hits.
   * @param enableHitContribs True to enable hit contributions.
   */
  void setEnableHitContribs(bool enableHitContribs) {
    enableHitContribs_ = enableHitContribs;
  }

  /**
   * Set whether hit contributions should be compressed by particle and PDG
   * code.
   * @param compressHitContribs True to compress hit contribution information.
   */
  void setCompressHitContribs(bool compressHitContribs) {
    compressHitContribs_ = compressHitContribs;
  }

 private:
  /// ConditionsInterface
  ConditionsInterface& conditionsIntf_;

  /**
   * Enable hit contribution output.
   */
  bool enableHitContribs_{true};

  /**
   * Enable compression of hit contributions by SimParticle and PDG code.
   */
  bool compressHitContribs_{true};
};

}  // namespace simcore

#endif
