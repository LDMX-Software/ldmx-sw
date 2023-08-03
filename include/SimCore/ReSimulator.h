
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
  void produce(framework::Event& event) override {
    /* numEventsBegan_++; */
    auto& eventHeader{event.getEventHeader()};
    std::istringstream iss(eventHeader.getStringParameter("eventSeed"));
    G4Random::restoreFullState(iss);
    const auto eventNumber{eventHeader.getEventNumber()};
    runManager_->ProcessOneEvent(eventNumber);
    std::cout << "Finished with event number " << eventNumber << std::endl;
    if (runManager_->GetCurrentEvent()->IsAborted()) {
      runManager_->TerminateOneEvent();
      SensitiveDetector::Factory::get().apply(
          [](auto sd) { sd->EndOfEvent(); });
      EXCEPTION_RAISE(
          "ReSimAbortedEvent",
          "Resimulation resulted in an aborted event, something is wrong with "
          "the seed from event " +
              std::to_string(eventNumber));
    }

    updateEventHeader(eventHeader);
    saveTracks(event);

    saveSDHits(event);
    /* TrackMap& tracks{g4user::TrackingAction::get()->getTrackMap()}; */
    /* tracks.traceAncestry(); */
    /* event.add("SimParticles", tracks.getParticleMap()); */

    runManager_->TerminateOneEvent();
  }
};
}  // namespace simcore

#endif /* SIMCORE_RESIMULATOR_H_ */
