/**
 * @file Simulator.h
 * @brief Run the G4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _SIMCORE_SIMULATOR_H_
#define _SIMCORE_SIMULATOR_H_

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <map>
#include <memory>
#include <string>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/ConditionsInterface.h"
#include "SimCore/DetectorConstruction.h"
#include "SimCore/RunManager.h"
#include "SimCore/SimulatorBase.h"

class G4UImanager;
class G4UIsession;
class G4RunManager;
class G4GDMLParser;
class G4GDMLMessenger;
class G4CascadeParameters;

namespace simcore {

/**
 * @class Simulator
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 *
 * Most (if not all) of the heavy lifting is done in the classes in the
 * Sim* modules.  This producer is mainly focused on calling appropriate
 * functions at the right time in the processing chain.
 */
class Simulator : public SimulatorBase {
 public:
  /**
   * Constructor.
   *
   * Blank Producer constructor
   * Constructs object that are non-configurable.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class assocaited with EventProcessor,
   *                provided by the Framework.
   */
  Simulator(const std::string& name, framework::Process& process);

  /**
   * Destructor.
   *
   */
  virtual ~Simulator() = default;

  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   *
   * @param parameters ParameterSet for configuration.
   */
  void configure(framework::config::Parameters& parameters) final override;

  /**
   * Given a non-const reference to the new RunHeader,
   * we can add parameters from the simulation here
   * before the run starts.
   *
   * @param header of new run
   */
  void beforeNewRun(ldmx::RunHeader& header) final override;

  /**
   * Before the run starts (but after the conditions are configured)
   * set up the random seeds for this run.
   *
   * @param[in] header RunHeader for this run, unused
   */
  void onNewRun(const ldmx::RunHeader& header) final override;

  /**
   * Run simulation and export results to output event.
   *
   * @param event The event to process.
   */
  virtual void produce(framework::Event& event) final override;

  /**
   * Callback for the EventProcessor to take any necessary action
   * when a file is closed.
   *
   * @param eventFile The intput/output file.
   */
  void onFileClose(framework::EventFile& eventFile) final override;

  /// Callback called once processing is complete.
  void onProcessEnd() final override;

 private:
  /**
   * Set the seeds to be used by the Geant4 random engine.
   *
   * @param[in] seeds A vector of seeds to pass to the G4 random
   *      engine.  The vector must contain at least 2 seeds otherwise
   *      an exception is thrown.
   */
  void setSeeds(std::vector<int> seeds);

 private:
  /// Number of events started
  int numEventsBegan_{0};

  /// Number of events completed
  int numEventsCompleted_{0};

  /// the run number (for accessing the run header in onFileClose
  int run_{-1};
};
}  // namespace simcore

#endif /* SIMCORE_SIMULATOR_H */
