/**
 * check that a standalone processor can use headers from a library that
 * depends on Geant4 (Issue #2106)
 */

#include "Framework/EventProcessor.h"
#include "SimCore/G4User/UserAction.h"

class StandaloneG4 : public framework::Analyzer {
 public:
  StandaloneG4(const std::string& name, framework::Process& p)
      : framework::Analyzer(name, p) {}
  ~StandaloneG4() override = default;
  void onProcessStart() override;
  void analyze(const framework::Event& event) override;
};

void StandaloneG4::onProcessStart() {
  histograms_.create("action_type", "User Action Type",
                     static_cast<int>(simcore::TYPE::NONE), 0.5,
                     static_cast<int>(simcore::TYPE::NONE) + 0.5);
}

void StandaloneG4::analyze(const framework::Event& event) {
  histograms_.fill("action_type", simcore::TYPE::EVENT);
}

DECLARE_ANALYZER(StandaloneG4);
