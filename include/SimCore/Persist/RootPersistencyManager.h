#ifndef SIMCORE_PERSIST_ROOTPERSISTENCYMANAGER_H_
#define SIMCORE_PERSIST_ROOTPERSISTENCYMANAGER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>
#include <vector>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventFile.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/EcalHitIO.h"
#include "SimCore/G4CalorimeterHit.h"
#include "SimCore/G4TrackerHit.h"
#include "SimCore/Persist/SimParticleBuilder.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4PersistencyCenter.hh"
#include "G4PersistencyManager.hh"

// Forward declarations
class G4Run;

// Forward declarations within the ldmx namespace
namespace framework {
class Event;
class RunHeader;
}  // namespace framework

namespace simcore {
namespace persist {

/**
 * @class RootPersistencyManager
 *
 * @note
 * Output is written at the end of each event.  An EventFile is used to
 * write from an Event buffer object into an output branch within a tree.
 * The event buffer is cleared after the event is written.  A
 * SimParticleBuilder is used to build a set of SimParticle objects from
 * the Trajectory objects which were created during event processing. An
 * EcalHitIO instance provides translation of G4CalorimeterHit objects in
 * the ECal to an output SimCalorimeterHit collection, transforming the
 * individual steps into cell energy depositions.  The tracker hit
 * collections of G4TrackerHit objects are translated directly into
 * output SimTrackerHit collections.
 */
class RootPersistencyManager : public G4PersistencyManager {
 public:
  /**
   * Class constructor.
   *
   * @param eventFile file to put output events into
   * @param parameters configuration parameters from Simulator
   * @param runNumber current run identifer from Process
   */
  RootPersistencyManager(framework::EventFile &file,
                         framework::config::Parameters &parameters,
                         const int &runNumber, ConditionsInterface &ci);

  /// Destructor
  virtual ~RootPersistencyManager() {}

  /**
   * Get the current ROOT persistency manager or <i>nullptr</i> if not
   * registered.
   *
   * @return The ROOT persistency manager.
   */
  static RootPersistencyManager *getInstance() {
    return static_cast<RootPersistencyManager *>(
        G4PersistencyCenter::GetPersistencyCenter()
            ->CurrentPersistencyManager());
  }

  /**
   * Builds the output ROOT event.
   *
   * @param anEvent The Geant4 event data.
   */
  G4bool Store(const G4Event *anEvent);

  /**
   * This gets called automatically at the end of the run and is used to write
   * out the run header and close the writer.
   *
   * @param aRun The Geant4 run data (not used right now)
   *
   * @return True if event is stored (function is hard-coded to return true).
   */
  G4bool Store(const G4Run *aRun);

  /**
   * Implementing this makes an "overloaded-virtual" compiler warning go away.
   */
  G4bool Store(const G4VPhysicalVolume *) { return false; }

  /**
   * This is called "manually" in UserRunAction to open the ROOT writer for the
   * run.
   */
  void Initialize();

  /**
   * Set the current ldmx-sw event.  This is used by the persistency
   * manager to retrieve and fill the containers that will be
   * persisted.
   *
   * @param event Event buffer for the current event.
   */
  void setCurrentEvent(framework::Event *event) { event_ = event; }

  /**
   * Set the number of events began and completed.
   *
   * These two numbers may or may not be equal
   * depending on if the simulation ran with any filters
   * that would abort events early.
   *
   * These numbers are helpful for evaluating filtering
   * performance, so we put them both in the RunHeader.
   *
   * @param[in] began number of events that were started
   * @param[in] completed number of events that were completed without an abort
   * signal
   */
  void setNumEvents(int began, int completed) {
    eventsBegan_ = began;
    eventsCompleted_ = completed;
  }

 public:
  /**
   * Build an output event from the current Geant4 event.
   *
   * @param anEvent The Geant4 event.
   * @param outputEvent The output event.
   */
  void buildEvent(const G4Event *anEvent);

  /**
   * Write header info into the output event from Geant4.
   *
   * @param anEvent The Geant4 event.
   */
  void writeHeader(const G4Event *anEvent);

  /**
   * Write hits collections from Geant4 into a ROOT event.
   *
   * @param anEvent The Geant4 event.
   * @param outputEvent The output event.
   */
  void writeHitsCollections(const G4Event *anEvent,
                            framework::Event *outputEvent);

  /**
   * Write a collection of tracker hits to an output collection.
   *
   * @param hc The collection of G4TrackerHits.
   * @param outputColl The output collection of SimTrackerHits.
   */
  void writeTrackerHitsCollection(G4TrackerHitsCollection *hc,
                                  std::vector<ldmx::SimTrackerHit> &outputColl);

  /**
   * Write a collection of tracker hits to an output collection.
   *
   * @param hc The collection of G4CalorimeterHits.
   * @param outputColl The output collection of SimCalorimeterHits.
   */
  void writeCalorimeterHitsCollection(
      G4CalorimeterHitsCollection *hc,
      std::vector<ldmx::SimCalorimeterHit> &outputColl);

 private:
  /// Configuration parameters passed to Simulator
  framework::config::Parameters parameters_;

  /// Run Number, given to us by Simulator from Process
  int run_;

  /// Number of events started on this production run
  int eventsBegan_{-1};

  /// Number of events completed without being aborted (due to filters)
  int eventsCompleted_{-1};

  /// The output file.
  framework::EventFile &file_;

  /// The event container used to manage the tree/branches/collections.
  framework::Event *event_{nullptr};

  /// Handles ECal hit readout and IO.
  EcalHitIO ecalHitIO_;

  /// Helper for building output SimParticle collection.
  SimParticleBuilder simParticleBuilder_;
};

}  // namespace persist
}  // namespace simcore

#endif
