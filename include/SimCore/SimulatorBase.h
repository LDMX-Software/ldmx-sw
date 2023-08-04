#ifndef SIMCORE_SIMULATOR_BASE_H_
#define SIMCORE_SIMULATOR_BASE_H_

#include <G4CascadeParameters.hh>
#include <G4GeometryManager.hh>
#include <G4UImanager.hh>
#include <G4UIsession.hh>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventDef.h"
#include "Framework/EventFile.h"
#include "Framework/EventHeader.h"
#include "Framework/EventProcessor.h"
#include "SimCore/ConditionsInterface.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/G4Session.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/Geo/ParserFactory.h"
#include "SimCore/RunManager.h"
#include "SimCore/SensitiveDetector.h"
#include "SimCore/UserEventInformation.h"
namespace simcore {
class SimulatorBase : public framework::Producer {
 public:
  SimulatorBase(const std::string& name, framework::Process& process);
  virtual ~SimulatorBase() = default;
  void configure(framework::config::Parameters& parameters) override;

 protected:
  /// Callback called once processing is complete.
  void onProcessEnd() override;
  /**
   * Initialization of simulation
   *
   * This uses the parameters set in the configure method to
   * construct and initialize the simulation objects.
   *
   * This function runs the post init setup commands.
   */
  void onProcessStart() override;
  ///  Conditions interface
  ConditionsInterface conditionsIntf_;

  /// User interface handle
  G4UImanager* uiManager_{nullptr};

  /// Manager controlling G4 simulation run
  std::unique_ptr<RunManager> runManager_;

  /// Handle to the G4Session -> how to deal with G4cout and G4cerr
  std::unique_ptr<G4UIsession> sessionHandle_;

  /// Commands not allowed to be passed from python config file
  ///     This is because Simulator already runs them.
  static const std::vector<std::string> invalidCommands_;

  /*********************************************************
   * Python Configuration Parameters
   *********************************************************/

  /// Vebosity for the simulation
  int verbosity_{1};
  /// The parameters used to configure the simulation
  framework::config::Parameters parameters_;

  std::vector<std::string> preInitCommands_;

  std::vector<std::string> postInitCommands_;

 private:
  /*
   * Set up logging for Geant4
   *
   *  If the verbosity level is set to 0, create a batch session
   *  If the verbosity level is > 1, log everything to a file.
   *  Otherwise, dump the output. If a prefix has been specified, append it ot
   *  the log message.
   **/
  void createLogging();

 protected:
  /*
   *
   * On succesful event, update event header properties like total PN/EN energy
   * and event weight.
   *
   */
  virtual void updateEventHeader(ldmx::EventHeader& eventHeader) const;

  /*
   * Save all tracks from the event that are marked for saving
   */
  virtual void saveTracks(framework::Event& event);

  /*
   * Save hits from sensitive detectors.
   */
  virtual void saveSDHits(framework::Event& event);

  virtual void produce(framework::Event& event) = 0;

 private:
  /*
   * Create the GDML parser and load the detector geometry
   */
  void buildGeometry();

  /*
   * Check that no invalid commands have been requested or that the old style
   * of setting the run number on the simulator rather than the process object
   * wasn't used.
   *
   * @see invalidCommands_
   */
  void verifyParameters() const;
};
}  // namespace simcore

#endif /* SIMCORE_SIMULATOR_BASE_H_ */
