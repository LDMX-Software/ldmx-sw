
#ifndef SIMCORE_RESIMULATOR_H_
#define SIMCORE_RESIMULATOR_H_
#include "Framework/EventFile.h"
#include "Framework/Process.h"
#include "SimCore/SensitiveDetector.h"
#include "SimCore/SimulatorBase.h"
namespace simcore {
class ReSimulator : public SimulatorBase {
 public:
  ReSimulator(const std::string& name, framework::Process& process)
      : SimulatorBase{name, process} {}
  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   *
   * @param parameters ParameterSet for configuration.
   */
  void configure(framework::config::Parameters& parameters) final override;
  /**
   * Run resimulation if the event is part of the requested sets of events to
   * resimulate
   *
   * @param event The event to process.
   */
  void produce(framework::Event& event) override;

 private:
  /**
   * List of event numbers in the input files that should be resimulated if
   * `resimulate_all_events` is false.
   *
   * @note: If an event number in `events_to_resimulate_` is not part of the
   * input file, it will be ignored.
   */
  std::vector<int> events_to_resimulate_;

  /**
   * List of run numbers in the input files that should be resimulated if
   * `resimulate_all_events` is false.
   *
   * @note: If a run number in `runs_to_resimulate_` is not part of the
   * input file, it will be ignored.
   *
   * The list of runs to resimulate comes in one of three forms:
   * 1. Empty: If no runs are given, we only match based off the event number.
   * 2. Single Entry: If only one run is given, then we require the run number
   *    to be that entry for all events requested to resimulate.
   * 3. More than one entry: In this case, we require the length of this list
   *    to be the same as the length of the events to resimulate so that we 
   *    only resimulate events where the event/run *pair* matches a corresponding
   *    *pair* in the events and runs to resimulate lists.
   */
  std::vector<int> runs_to_resimulate_;

  /**
   * Whether to resimulate all events in the input files
   */
  bool resimulate_all_events;

  /*
   * How many events have already been resimulated. This determines the event
   * number in the output file, since more than one input file can be used.
   *
   */
  int events_resimulated = 0;
};
}  // namespace simcore

#endif /* SIMCORE_RESIMULATOR_H_ */
