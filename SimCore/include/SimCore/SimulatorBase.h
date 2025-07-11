#ifndef SIMCORE_SIMULATOR_BASE_H_
#define SIMCORE_SIMULATOR_BASE_H_

#include <G4CascadeParameters.hh>
#include <G4GeometryManager.hh>
#include <G4UImanager.hh>
#include <G4UIsession.hh>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventFile.h"
#include "Framework/EventHeader.h"
#include "Framework/EventProcessor.h"
#include "Framework/Logger.h"
#include "SimCore/ConditionsInterface.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/G4Session.h"
#include "SimCore/G4User/TrackingAction.h"
#include "SimCore/G4User/UserEventInformation.h"
#include "SimCore/Geo/Parser.h"
#include "SimCore/RunManager.h"
#include "SimCore/SDs/SensitiveDetector.h"

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

  /// Commands not allowed to be passed from python config file
  ///     This is because Simulator already runs them.
  static const std::vector<std::string> invalidCommands_;

  /*********************************************************
   * Python Configuration Parameters
   *********************************************************/
  /// The parameters used to configure the simulation
  framework::config::Parameters parameters_;

  std::vector<std::string> preInitCommands_;

  std::vector<std::string> postInitCommands_;

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

  virtual void produce(framework::Event& event) override = 0;

 private:
  /*
   * Create the GDML parser and load the detector geometry during
   * initialization.
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

 protected:
  /*
   * Enable logging
   */
  enableLogging("SimulatorBase")
};
}  // namespace simcore

#endif /* SIMCORE_SIMULATOR_BASE_H_ */
