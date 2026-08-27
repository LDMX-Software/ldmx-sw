#include "SimCore/ReSimulator.h"

namespace simcore {

void ReSimulator::configure(framework::config::Parameters& parameters) {
  SimulatorBase::configure(parameters);
  resimulate_all_events_ = parameters.get<bool>("resimulate_all_events");
  if (!resimulate_all_events_) {
    care_about_run_ = parameters.get<bool>("care_about_run");
    auto configured_events{
        parameters.get<std::vector<framework::config::Parameters>>(
            "events_to_resimulate", {})};
    if (configured_events.size() == 0) {
      EXCEPTION_RAISE(
          "ReSimNoEvents",
          "ReSim was configured with resimulate_all_events marked false but "
          "no event numbers were requested.\n\nDid you forget to configure "
          "the events_to_resimulate parameter?\n");
    }
    for (const auto& run_event : configured_events) {
      events_to_resimulate_.emplace_back(run_event.get<int>("run"),
                                         run_event.get<int>("event"));
    }
  }
}

void ReSimulator::produce(framework::Event& event) {
  /* num_events_began_++; */
  auto& event_header{event.getEventHeader()};
  const auto event_number{event_header.getEventNumber()};
  if (skip(event)) {
    ldmx_log(trace) << "Skipping event: " << event_number
                    << " since it wasn't part of the requested events...";

    this->abortEvent();  // get out of processors loop
    return;
  }

  ldmx_log(trace) << "Resimulating " << event_number;

  std::istringstream iss(event_header.getStringParameter("eventSeed"));
  G4Random::restoreFullState(iss);

  SimulatorBase::prepEvent(event);
  run_manager_->ProcessOneEvent(event_number);

  ldmx_log(trace) << "Finished with event number " << event_number;

  if (run_manager_->GetCurrentEvent()->IsAborted()) {
    run_manager_->TerminateOneEvent();
    SensitiveDetector::Factory::get().apply(
        [](auto sd) { sd->onFinishedEvent(); });
    EXCEPTION_RAISE(
        "ReSimAbortedEvent",
        "Resimulation resulted in an aborted event, something is wrong with "
        "the seed from event " +
            std::to_string(event_number));
  }

  event_header.setEventNumber(++events_resimulated_);
  updateEventHeader(event_header);
  saveTracks(event);

  saveSDHits(event);

  run_manager_->TerminateOneEvent();
}

bool ReSimulator::skip(framework::Event& event) const {
  /**
   * If we are configured to simply resimulate all events, this
   * function always returns false.
   */
  if (resimulate_all_events_) return false;
  /**
   * Otherwise, we check the event number
   * (and also its run number if we care_about_run_)
   * against the list of run/event pairs that we are
   * interested in re-simulating.
   */
  auto found_event_to_resim = std::find_if(
      std::begin(events_to_resimulate_), std::end(events_to_resimulate_),
      [&](const std::pair<int, int>& run_event) -> bool {
        bool runs_match = true;
        if (care_about_run_)
          runs_match = (event.getEventHeader().getRun() == run_event.first);
        return event.getEventNumber() == run_event.second and runs_match;
      });
  return (found_event_to_resim == std::end(events_to_resimulate_));
}

}  // namespace simcore
DECLARE_PRODUCER(simcore::ReSimulator)
