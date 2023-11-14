#include "SimCore/ReSimulator.h"

namespace simcore {

void ReSimulator::configure(framework::config::Parameters& parameters) {
  SimulatorBase::configure(parameters);
  resimulate_all_events_ =
      parameters.getParameter<bool>("resimulate_all_events");
  if (!resimulate_all_events_) {
    care_about_run_ = parameters.getParameter<bool>("care_about_run");
    auto configured_events{parameters.getParameter<std::vector<framework::config::Parameters>>("events_to_resimulate", {})};
    if (configured_events.size() == 0) {
      EXCEPTION_RAISE(
          "ReSimNoEvents",
          "ReSim was configured with resimulate_all_events marked false but "
          "no event numbers were requested.\n\nDid you forget to configure "
          "the events_to_resimulate parameter?\n");
    }
    for (const auto& run_event : configured_events) {
      events_to_resimulate_.emplace_back(
            run_event.getParameter<int>("run"),
            run_event.getParameter<int>("event")
      );
    }
  }
}

void ReSimulator::produce(framework::Event& event) {
  /* numEventsBegan_++; */
  auto& eventHeader{event.getEventHeader()};
  const auto eventNumber{eventHeader.getEventNumber()};
  if (!resimulate_all_events_) {
    auto found_event_to_resim = std::find_if(
        std::begin(events_to_resimulate_), std::end(events_to_resimulate_),
        [&](const std::pair<int,int>& run_event) -> bool {
          bool runs_match = true;
          if (care_about_run_) runs_match = (event.getEventHeader().getRun() == run_event.first);
          return eventNumber == run_event.second and runs_match;
        }
    );
    if (found_event_to_resim == std::end(events_to_resimulate_)) {
      if (verbosity_ > 1) {
        std::cout << "Skipping event: " << eventNumber
                  << " since it wasn't part of the requested events..."
                  << std::endl;
      }
      this->abortEvent();  // get out of processors loop
      return;
    }
  }
  if (verbosity_ > 0) {
    std::cout << "Resimulating " << eventNumber << std::endl;
  }

  std::istringstream iss(eventHeader.getStringParameter("eventSeed"));
  G4Random::restoreFullState(iss);
  runManager_->ProcessOneEvent(eventNumber);
  if (verbosity_ > 1) {
    std::cout << "Finished with event number " << eventNumber << std::endl;
  }
  if (runManager_->GetCurrentEvent()->IsAborted()) {
    runManager_->TerminateOneEvent();
    SensitiveDetector::Factory::get().apply(
        [](auto sd) { sd->OnFinishedEvent(); });
    EXCEPTION_RAISE(
        "ReSimAbortedEvent",
        "Resimulation resulted in an aborted event, something is wrong with "
        "the seed from event " +
            std::to_string(eventNumber));
  }

  eventHeader.setEventNumber(++events_resimulated_);
  updateEventHeader(eventHeader);
  saveTracks(event);

  saveSDHits(event);

  runManager_->TerminateOneEvent();
}

}  // namespace simcore
DECLARE_PRODUCER_NS(simcore, ReSimulator)
