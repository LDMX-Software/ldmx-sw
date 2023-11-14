
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
   * List of events in the input files that should be resimulated if
   * `resimulate_all_events` is false.
   *
   * Each event is identified uniquely by its run number and event number.
   *
   * @note: If an event in `events_to_resimulate_` is not part of the
   * input file, it will be ignored.
   */
  std::vector<std::pair<int,int>> events_to_resimulate_;

  /**
   * Whether to resimulate all events in the input files
   */
  bool resimulate_all_events_;

  /**
   * Whether or not we should check the run number when seeing
   * if a specific event should be resimulated
   */
  bool care_about_run_;

  /*
   * How many events have already been resimulated. This determines the event
   * number in the output file, since more than one input file can be used.
   *
   */
  int events_resimulated_ = 0;
};
}  // namespace simcore

#endif /* SIMCORE_RESIMULATOR_H_ */
